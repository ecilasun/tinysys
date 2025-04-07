#include "basesystem.h"
#include "rombase.h"
#include "task.h"
#include "device.h"
#include "commandline.h"
#include "rombase.h"
#include "keyringbuffer.h"
//#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Console input context
extern struct SCommandLineContext* s_cliCtx;

// Names of task states for process dump
static const char *s_taskstates[]={ "NONE", "HALT", "EXEC", "TERM", "DEAD", "IDLE" };

// Device version
#define ROMVERSIONSTRING "00152"
#define GATEWAREVERSIONSTRING "00203"
#define CORECLOCKSTRING "175MHz"

// File transfer timeout
#define FILE_TRANSFER_TIMEOUT 1000

struct SCommandLineContext* CLIGetContext()
{
	return s_cliCtx;
}

void CLIClearStatics()
{
	s_cliCtx->cmdLen = 0;
	s_cliCtx->refreshConsoleOut = 1;
	s_cliCtx->execParamCount = 1;
	s_cliCtx->userTaskID = 0;
	s_cliCtx->cmdString[0] = 0;
	s_cliCtx->prevCmdString[0] = 0;
	s_cliCtx->pathtmp[0] = 0;
	s_cliCtx->startAddress = 0;
	s_cliCtx->execName[0] = 0;
	s_cliCtx->execParam0[0] = 0;
}

// This task is a trampoline to the loaded executable
void __attribute__((aligned(64))) _runExecTask()
{
	// Start the loaded executable
	asm volatile(
		"addi sp, sp, -16;"
		"sw %3, 0(sp);"		// Store argc
		"sw %1, 4(sp);"		// Store argv[1] (path to exec)
		"sw %2, 8(sp);"		// Store argv[2] (exec param0)
		"sw zero, 12(sp);"	// Store argv[3] (have to end list with a nullptr)
		".insn 0xFC000073;"	// Invalidate & Write Back D$ (CFLUSH.D.L1) - ensure that we have written all loaded code to memory
		"fence.i;"			// Invalidate I$ - ensure loaded binary can be fetched as fresh instructions by the CPU
		"lw t0, %0;"		// Set target branch address
		"jalr t0;"			// Branch to the entry point
		"addi sp, sp, 16;"	// We really won't come back here, but do the proper stack cleanup anyway
		"infinite_loop:"	// However, if ra register was correct and we do somehow return here, we'll just loop forever until killed
		"wfi;"
		"j infinite_loop;"
		: "=m" (s_cliCtx->startAddress)
		: "r" (s_cliCtx->execName), "r" (s_cliCtx->execParam0), "r" (s_cliCtx->execParamCount)
		// Clobber list
		: "t0"
	);
	// NOTE: Execution should never reach here since the ELF will invoke ECALL(0x5D) to quit
	// and will be removed from the task list, thus removing this function from the
	// execution pool.
}

void ShowVersion(struct EVideoContext *kernelgfx)
{
	uint32_t waterMark = read_csr(0xFF0);
	uint32_t isEmulator = read_csr(0xF12) & 0x80000000 ? 0 : 1; // CSR_MARCHID is 0x80000000 for read hardware, 0x00000000 for emulator

	VPUConsoleSetColors(kernelgfx, CONSOLEWHITE, CONSOLEGRAY);
	kprintf("\n");
	kfillline(' ');
	kprintf(" OS               : " ROMVERSIONSTRING " Loaded from %s", waterMark == 0 ? "ROM" : "SDCARD");
	kfillline(' ');
	kprintf(" Board            : issue 2H:DEC24");
	kfillline(' ');
	kprintf(" Gateware         : (%s)", isEmulator ? "EMULATED" : GATEWAREVERSIONSTRING);
	kfillline(' ');
	kprintf(" CPU architecture : rv32im_zicsr_zifencei_zfinx (dual core)");
	kfillline(' ');
	kprintf(" Bus clock        : " CORECLOCKSTRING);
	kfillline(' ');
	kfillline(' ');
	kprintf("\n");
	VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
}

void DumpTasks(const uint32_t _hartid)
{
	struct STaskContext *ctx = _task_get_context(_hartid);
	if (ctx->numTasks == 0)
	{
		kprintf("CPU%d : no tasks\n", _hartid);
	}
	else
	{
		for (int i=0;i<ctx->numTasks;++i)
		{
			struct STask *task = &ctx->tasks[i];
			kprintf("CPU%d : #%d : %s : PC=0x%08X : '%s'\n", _hartid, i, s_taskstates[task->state], task->regs[0], task->name);
		}
	}
}

uint32_t ExecuteCmd(char *_cmd, struct EVideoContext *kernelgfx)
{
	const char *command = strtok(_cmd, " ");
	if (!command)
		return 1;

	uint32_t loadELF = 0;

	if (!strcmp(command, "help"))
	{
		kprintf("Commands:\ndir, mount, unmount, cls, reboot, mem, proc, del, ren, pwd, cd,\nor name of ELF without extension\n");
	}
	else if (!strcmp(command, "dir"))
	{
		const char *path = strtok(NULL, " ");
		if (!path)
			ListFiles(GetWorkDir());
		else
			ListFiles(path);
	}
	else if (!strcmp(command, "mount"))
	{
		SetWorkDir("sd:/");
		MountDrive();
	}
	else if (!strcmp(command, "unmount"))
	{
		UnmountDrive();
	}
	else if (!strcmp(command, "cls"))
	{
		VPUConsoleClear(kernelgfx);
	}
	else if (!strcmp(command, "reboot"))
	{
		// Clear to "we're rebooting" color
		VPUClear(kernelgfx, 0x0C0C0C0C);
		// Turn video output off
		VPUSetVMode(kernelgfx, EVS_Disable);

		// Remove watermark since we might have deleted / changed the rom image oin the sdcard for next boot.
		write_csr(0xFF0, 0x0);

		// Reset to default ROM entry points and reset each CPU. Make sure main CPU ist last to go.
		E32SetupCPU(0, (void*)0x0);
		E32ResetCPU(0);
	}
	else if (!strcmp(command, "mem"))
	{
		kprintf("Available memory:");
		uint32_t inkbytes = core_memavail()/1024;
		uint32_t inmbytes = inkbytes/1024;
		if (inmbytes!=0)
			kprintf("%d Mbytes\n", inmbytes);
		else
			kprintf("%d Kbytes\n", inkbytes);
	}
	else if (!strcmp(command, "proc"))
	{
		const char *cpuindex = strtok(NULL, " ");
		if (!cpuindex)
		{
			for (uint32_t i=0; i<MAX_HARTS; ++i)
				DumpTasks(i);
		}
		else
		{
			uint32_t hartid = atoi(cpuindex);
			DumpTasks(hartid);
		}
	}
	else if (!strcmp(command, "del"))
	{
		const char *path = strtok(NULL, " ");
		if (!path)
			kprintf("usage: del fname\n");
		else
		{
			int res = remove(path);
			if (res < 0)
				kprintf("file '%s' not found\n", path);
			else
				kprintf("file '%s' removed\n", path);
		}
	}
	else if (!strcmp(command, "ren"))
	{
		const char *path = strtok(NULL, " ");
		const char *newpath = strtok(NULL, " ");
		if (!path || !newpath)
			kprintf("usage: ren oldname newname\n");
		else
			rename(path, newpath);
	}
	else if (!strcmp(command, "pwd"))
	{
		kprintf("%s\n", GetWorkDir());
	}
	else if (!strcmp(command, "cd"))
	{
		const char *path = strtok(NULL, " ");
		// Change working directory
		if (!path)
			kprintf("usage: cd path\n");
		else
		{
			if (krealpath(path, s_cliCtx->pathtmp))
			{
				// Append missing trailing slash
				int L = strlen(s_cliCtx->pathtmp);

				if (L != 0 && s_cliCtx->pathtmp[L-1] != '/')
					strcat(s_cliCtx->pathtmp, "/");
				
				// Finally, change to this new path
				FRESULT cwdres = f_chdir(s_cliCtx->pathtmp);
				if (cwdres == FR_OK)
					SetWorkDir(s_cliCtx->pathtmp);
				else
				{
					kprintf("Invalid: '%s'\n", s_cliCtx->pathtmp);
				}
			}
			else
				kprintf("Invalid: '%s'\n", s_cliCtx->pathtmp);
		}
	}
	else // Anything else defers to being a command on storage
		loadELF = 1;

	if (loadELF)
	{
		// TODO: Add support to load user ELF files on HART#1/2
		struct STaskContext* tctx[MAX_HARTS] = {_task_get_context(0), _task_get_context(1)};
		int32_t taskcounts[MAX_HARTS] = {tctx[0]->numTasks, tctx[1]->numTasks};
		int32_t maxcounts[MAX_HARTS] = {2, 1};

		// Temporary measure to avoid loading another executable while the first one is running
		// until we get a virtual memory device
		if (taskcounts[0] > maxcounts[0]) // User programs always boot on main CPU
		{
			kprintf("Virtual memory / code relocator not implemented.\n");
		}
		else
		{
			char filename[64];
			strcpy(filename, GetWorkDir());	// Current path already contains a trailing slash
			strcat(filename, command);		// User supplied string
			strcat(filename, ".elf");		// We don't expect command to contain the .elf extension

			// First parameter is excutable name
			s_cliCtx->startAddress = LoadExecutable(filename, 0, false);
			// TODO: Scan and push all argv and the correct argc onto stack

			// If we could not find the executable where we are, look into the 'sys/bin' directory
			if (s_cliCtx->startAddress == 0x0)
			{
				strcpy(filename, "sd:/sys/bin/");
				strcat(filename, command);
				strcat(filename, ".elf");
				s_cliCtx->startAddress = LoadExecutable(filename, 0, false);
			}

			// If we succeeded in loading the executable, the trampoline task can branch into it.
			// NOTE: Without code relocation or virtual memory, two executables will ovelap when loaded
			// even though each gets a new task slot assigned.
			// This will cause corruption of the runtime environment.
			if (s_cliCtx->startAddress != 0x0)
			{
				strncpy(s_cliCtx->execName, command, 32);

				const char *param = strtok(NULL, " ");
				// Change working directory
				if (!param)
					s_cliCtx->execParamCount = 1;
				else
				{
					strncpy(s_cliCtx->execParam0, param, 32);
					s_cliCtx->execParamCount = 2;
				}

				// User tasks always boot on main CPU, and can then add their own tasks to the other CPUs
				s_cliCtx->userTaskID = TaskAdd(tctx[0], command, (taskfunc)_runExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS, TASK_STACK_POINTER(0, 2, TASK_STACK_SIZE));

				return 0;
			}
			else
				kprintf("Executable '%s' not found\n", command);
		}
	}

	return 1;
}

int HandleCommandLine(struct STaskContext *taskctx)
{
	// Echo all of the characters we can find back to the sender
	uint8_t asciicode = 0;
	int execcmd = 0;
	int controltrap = 0;

	struct EVideoContext* ctx = VPUGetKernelGfxContext();

	// Keyboard input or other data
	while (!controltrap && KeyRingBufferRead(&asciicode, 1))
	{
		// DEBUG: UARTPrintf("[%d]", asciicode);

		switch (asciicode)
		{
			case 224: // Extended key code
			{
				// Arrow keys and other control codes
				controltrap = 1;
			}
			break;

			case 10:
			case 13:	// Return / Enter
			{
				// Properly terminate the command string
				s_cliCtx->cmdString[s_cliCtx->cmdLen] = 0;
				// Copy command string to previous command buffer
				strcpy(s_cliCtx->prevCmdString, s_cliCtx->cmdString);
				execcmd++;
			}
			break;

			case 3:		// EXT (Ctrl+C) - BREAK
			{
				// Return with empty command string
				s_cliCtx->cmdLen = 0;
				s_cliCtx->cmdString[0] = 0;
				++s_cliCtx->refreshConsoleOut;

				// NOTE: This has to be done in reverse order!
				// NOTE: Sig:0, terminate process if no debugger is attached

				// Stop task on main CPU except IDLE and CLI
				for (int i=taskctx->numTasks-1; i>=2; i--)
					_task_exit_task_with_id(taskctx, i, 0);

				// Stop all other tasks on helper CPUs
				{
					struct STaskContext* tctx1 = _task_get_context(1);
					for (int i=tctx1->numTasks-1; i>=1; i--)
						_task_exit_task_with_id(tctx1, i, 0);
				}
			}
			break;

			case 9: 	// Tab
			{
				// TODO: Implement tab completion
			}
			break;

			case 8:		// Backspace
			{
				s_cliCtx->cmdLen--;
				++s_cliCtx->refreshConsoleOut;
				if (s_cliCtx->cmdLen<0)
					s_cliCtx->cmdLen = 0;
			}
			break;

			case 27:	// ESC
			{
				// Erase current line
				VPUConsoleClearLine(ctx, ctx->m_cursorY);
				s_cliCtx->cmdLen = 0;
				++s_cliCtx->refreshConsoleOut;
				// TODO: Erase current line
			}
			break;

			// CTRL + L (Clear screen i.e. form feed)
			case 12:
			{
				++s_cliCtx->refreshConsoleOut;
				VPUConsoleClear(ctx);
			}
			break;

			default:
			{
				// Do not enqueue characters into command string if we're running some app
				if(taskctx->numTasks <= 2)
				{
					++s_cliCtx->refreshConsoleOut;
					s_cliCtx->cmdString[s_cliCtx->cmdLen++] = (char)asciicode;
					if (s_cliCtx->cmdLen > 126)
						s_cliCtx->cmdLen = 126;
				}
			}
			break;
		}
	}

	if (controltrap)
	{
		// Arrow keys
		uint8_t controlcode = 0;
		// Wait for next byte
		while (controlcode == 0)
		{
			KeyRingBufferRead(&controlcode, 1);
		}
		switch (controlcode)
		{
			case 72:	// Up
			{
				// Copy previous command to current command buffer if we have one
				if (s_cliCtx->prevCmdString[0] != 0)
				{
					strcpy(s_cliCtx->cmdString, s_cliCtx->prevCmdString);
					s_cliCtx->cmdLen = strlen(s_cliCtx->cmdString);
					++s_cliCtx->refreshConsoleOut;
				}
			}
			break;
			case 80:	// Down
			{
				// TODO: Implement command history
			}
			break;
			case 75:	// Left
			{
			}
			break;
			case 77:	// Right
			{
			}
			break;
		}
		controltrap = 0;
	}

	return execcmd;
}

void _CLITask()
{
	s_cliCtx->refreshConsoleOut = 1;
	s_cliCtx->cmdLen = 0;
	s_cliCtx->cmdString[0] = 0;

	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	ShowVersion(kernelgfx);

	int cursortoggle = 0;
	struct STaskContext *taskctx = _task_get_context(0);
	uint64_t cursorTime = E32ReadTime();
	while(1)
	{
		int execcmd = 0;
		execcmd = HandleCommandLine(taskctx);

		// Toggle cursor every 500ms
		uint64_t currentTime = E32ReadTime();
		if (ClockToMs(currentTime-cursorTime) > 500)
		{
			cursorTime = currentTime;
			++cursortoggle;
			// Mark console for refresh
			++s_cliCtx->refreshConsoleOut;
		}

		if (execcmd)
			cursortoggle = 0;

		// Process or echo input only when we have no ELF running on hart#1
		if(taskctx->numTasks <= 2)
		{
			if (execcmd)
			{
				kprintf("\n");
				s_cliCtx->refreshConsoleOut += ExecuteCmd(s_cliCtx->cmdString, kernelgfx);
				// Reset command string
				s_cliCtx->cmdLen = 0;
				s_cliCtx->cmdString[0] = 0;
			}

			if (s_cliCtx->refreshConsoleOut)
			{
				s_cliCtx->refreshConsoleOut = 0;
				// properly terminate the command string
				s_cliCtx->cmdString[s_cliCtx->cmdLen] = 0;
				// Make sure the ENTER key is echoed
				// Rewind cursor and print the command string
				int cx,cy;
				kgetcursor(&cx, &cy);
				ksetcursor(0, cy);
				kprintf("%s>%s ", GetWorkDir(), s_cliCtx->cmdString);
				kgetcursor(&cx, &cy);
				ksetcaret(cx-1, cy, cursortoggle%2);
			}
		}

		clear_csr(mie, MIP_MTIP);
		_task_yield();
		set_csr(mie, MIP_MTIP);
	}
}
