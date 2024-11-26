#include "basesystem.h"
#include "rombase.h"
#include "task.h"
#include "device.h"
#include "max3421e.h"
#include "commandline.h"
#include "keyringbuffer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Console input context
static struct SCommandLineContext s_cliCtx;

// Names of task states for process dump
static const char *s_taskstates[]={ "NONE", "HALT", "EXEC", "TERM", "DEAD"};

// Device version
#define VERSIONSTRING "v1.0C"

// File transfer timeout
#define FILE_TRANSFER_TIMEOUT 1000

struct SCommandLineContext* CLIGetContext()
{
	return &s_cliCtx;
}

void CLIClearStatics()
{
	s_cliCtx.cmdLen = 0;
	s_cliCtx.refreshConsoleOut = 1;
	s_cliCtx.execParamCount = 1;
	s_cliCtx.userTaskID = 0;
	s_cliCtx.cmdString[0] = 0;
	s_cliCtx.pathtmp[0] = 0;
	s_cliCtx.startAddress = 0;
	s_cliCtx.execName[0] = 0;
	s_cliCtx.execParam0[0] = 0;
}

// This task is a trampoline to the loaded executable
void _runExecTask()
{
	// Start the loaded executable
	asm volatile(
		"addi sp, sp, -16;"
		"sw %3, 0(sp);"		// Store argc
		"sw %1, 4(sp);"		// Store argv[1] (path to exec)
		"sw %2, 8(sp);"		// Store argv[2] (exec params0)
		"sw zero, 12(sp);"	// Store argv[3] (nullptr)
		".insn 0xFC000073;"	// Invalidate & Write Back D$ (CFLUSH.D.L1)
		"fence.i;"			// Invalidate I$
		"lw s0, %0;"		// Target branch address
		"jalr s0;"			// Branch to the entry point
		"addi sp, sp, 16;"	// We most likely won't return here
		: "=m" (s_cliCtx.startAddress)
		: "r" (s_cliCtx.execName), "r" (s_cliCtx.execParam0), "r" (s_cliCtx.execParamCount)
		// Clobber list
		: "s0"
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
	kprintf("\n                                                   \n");

	kprintf(" OS version          : " VERSIONSTRING " (%s)              \n", waterMark == 0 ? "IN-ROM" : "SDCARD");

	// TODO: These two values need to come from a CSR,
	// pointing at a memory location with device config data (machineconfig?)
	// That memory location will in turn point at an onboard EEPROM we can
	// read device versions/presence from.
	kprintf(" Board               : issue 2E:2024 (%s)    \n", isEmulator ? "EMULATED" : "HARDWARE");
	kprintf(" CPU & bus clock     : 150MHz                      \n");
	kprintf(" ARCH                : rv32im_zicsr_zifencei_zfinx \n");
	kprintf(" ESP32               : ESP32-C6-WROOM-1-N8         \n");

	// Report USB host chip version if found
	uint8_t m3421rev = MAX3421ReadByte(rREVISION);
	if (m3421rev != 0xFF)
		kprintf(" MAX3421(USB Host)   : 0x%X                        \n", m3421rev);

	// Video circuit on 2B has no info we can read so this is hardcoded
	kprintf(" SII164(video)       : 12bpp DVI                   \n");
	kprintf(" CS4344(audio)       : 11/22/44KHz stereo          \n");
	kprintf("                                                   \n\n");

	VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
}

void ShowHelp(struct EVideoContext *kernelgfx)
{
	VPUConsoleSetColors(kernelgfx, CONSOLEWHITE, CONSOLEGRAY);

	kprintf("\n                                                     \n");
	kprintf(" COMMAND      | USAGE                                \n");
	kprintf(" cls          | Clear terminal                       \n");
	kprintf(" cd path      | Change working directory             \n");
	kprintf(" del fname    | Delete file                          \n");
	kprintf(" dir [path]   | Show list of files in cwd or path    \n");
	kprintf(" mem          | Show available memory                \n");
	kprintf(" proc cpu     | Show process info for given CPU(0/1) \n");
	kprintf(" pwd          | Show current work directory          \n");
	kprintf(" reboot       | Reboot the device                    \n");
	kprintf(" ren old new  | Rename file from old to new name     \n");
	kprintf(" ver          | Show version info                    \n");
	kprintf("                                                     \n");

	// Hidden commands for dev mode reveal when running from overlay
	uint32_t waterMark = read_csr(0xFF0);
	if (waterMark != 0)
	{
		kprintf(" DEV MODE SPECIFIC - USE AT YOUR OWN RISK            \n");
		kprintf(" COMMAND      | USAGE                                \n");
		kprintf(" kill pid cpu | Kill process with id pid on CPU      \n");
		kprintf(" mount        | Mount drive sd:                      \n");
		kprintf(" runon cpu    | Run ELF files on given cpu           \n");
		kprintf(" unmount      | Unmount drive sd:                    \n");
		kprintf("                                                     \n");
	}
	kprintf("\n");

	VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
}

uint32_t ExecuteCmd(char *_cmd, struct EVideoContext *kernelgfx)
{
	const char *command = strtok(_cmd, " ");
	if (!command)
		return 1;

	uint32_t loadELF = 0;

	if (!strcmp(command, "dir"))
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
		VPUSetVMode(kernelgfx, EVS_Disable);

		// Remove watermark since we might have deleted / changed the rom image for next boot.
		write_csr(0xFF0, 0x0);

		// Reset to default ROM entry points and reset each CPU. Make sure main CPU ist last to go.
		E32SetupCPU(1, (void*)0x0);
		E32ResetCPU(1);
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
			kprintf("usage: proc cpu\n");
		}
		else
		{
			uint32_t hartid = atoi(cpuindex);
			// TODO: We need atomics here to accurately view memory contents written by another core
			struct STaskContext *ctx = GetTaskContext(hartid);
			if (ctx->numTasks == 0)
			{
				kprintf("No tasks on core #%d\n", hartid);
			}
			else
			{
				kprintf("%d tasks on core #%d\n", ctx->numTasks, hartid);
				for (int i=0;i<ctx->numTasks;++i)
				{
					struct STask *task = &ctx->tasks[i];
					kprintf("#%d:%s PC=0x%08X Name:'%s'\n", i, s_taskstates[task->state], task->regs[0], task->name);
				}
			}
		}
	}
	else if (!strcmp(command, "del"))
	{
		const char *path = strtok(NULL, " ");
		if (!path)
			kprintf("usage: rm fname\n");
		else
		{
			int res = remove(path);
			if (res < 0)
				kprintf("file '%s' not found\n", path);
			else
				kprintf("file '%s' removed\n", path);
		}
	}
	else if (!strcmp(command, "kill"))
	{
		const char *processid = strtok(NULL, " ");
		if (!processid)
			kprintf("usage: kill processid cpu\n");
		else
		{
			const char *hartindex = strtok(NULL, " ");
			if (!hartindex)
			{
				kprintf("usage: kill processid cpu\n");
			}
			else
			{
				uint32_t hartid = atoi(hartindex);
				// Warning! This can also kill PID(1) which is the CLI
				struct STaskContext *ctx = GetTaskContext(hartid);
				int taskid = atoi(processid);
				TaskExitTaskWithID(ctx, taskid, 0);
			}
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
			if (krealpath(path, s_cliCtx.pathtmp))
			{
				// Append missing trailing slash
				int L = (int)strlen(s_cliCtx.pathtmp);
				if (L != 0 && s_cliCtx.pathtmp[L-1] != '/')
					strcat(s_cliCtx.pathtmp, "/");

				// Finally, change to this new path
				FRESULT cwdres = f_chdir(s_cliCtx.pathtmp);
				if (cwdres == FR_OK)
					SetWorkDir(s_cliCtx.pathtmp);
				else
				{
					kprintf("invalid path(0) '%s'\n", s_cliCtx.pathtmp);
				}
			}
			else
				kprintf("invalid path(1) '%s'\n", s_cliCtx.pathtmp);
		}
	}
	else if (!strcmp(command, "ver"))
	{
		ShowVersion(kernelgfx);
	}
	else if (!strcmp(command, "help"))
	{
		ShowHelp(kernelgfx);
	}
	else // Anything else defers to being a command on storage
		loadELF = 1;

	if (loadELF)
	{
		// TODO: Add support to load user ELF files on HART#1/2
		struct STaskContext* tctx[MAX_HARTS] = {GetTaskContext(0), GetTaskContext(1), GetTaskContext(2)};
		int32_t taskcounts[MAX_HARTS] = {tctx[0]->numTasks, tctx[1]->numTasks, tctx[2]->numTasks};
		int32_t maxcounts[MAX_HARTS] = {2, 1, 1};

		// Temporary measure to avoid loading another executable while the first one is running
		// until we get a virtual memory device
		if (taskcounts[0] > maxcounts[0]) // Tasks always boot on main CPU
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
			s_cliCtx.startAddress = LoadExecutable(filename, 0, false);
			// TODO: Scan and push all argv and the correct argc onto stack

			// If we could not find the executable where we are, look into the 'sys/bin' directory
			if (s_cliCtx.startAddress == 0x0)
			{
				strcpy(filename, "sd:/sys/bin/");
				strcat(filename, command);
				strcat(filename, ".elf");
				s_cliCtx.startAddress = LoadExecutable(filename, 0, false);
			}

			// If we succeeded in loading the executable, the trampoline task can branch into it.
			// NOTE: Without code relocation or virtual memory, two executables will ovelap when loaded
			// even though each gets a new task slot assigned.
			// This will cause corruption of the runtime environment.
			if (s_cliCtx.startAddress != 0x0)
			{
				// Make sure everything is flushed to RAM and the instruction cache is invalidated
				CFLUSH_D_L1;
				FENCE_I;

				strncpy(s_cliCtx.execName, filename, 32);

				const char *param = strtok(NULL, " ");
				// Change working directory
				if (!param)
					s_cliCtx.execParamCount = 1;
				else
				{
					strncpy(s_cliCtx.execParam0, param, 32);
					s_cliCtx.execParamCount = 2;
				}

				// User tasks always boot on main CPU, and can then add their own tasks to the other CPUs
				s_cliCtx.userTaskID = _task_add(tctx[0], command, _runExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

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

	// Keyboard input or other data
	while (KeyRingBufferRead(&asciicode, 1))
	{
		switch (asciicode)
		{
			case 10:
			case 13:	// Return / Enter
			{
				// Properly terminate the command string
				s_cliCtx.cmdString[s_cliCtx.cmdLen] = 0;
				execcmd++;
			}
			break;
		
			case 3:		// EXT (Ctrl+C) - BREAK
			{
				// Return with empty command string
				s_cliCtx.cmdLen = 0;
				s_cliCtx.cmdString[0] = 0;
				++s_cliCtx.refreshConsoleOut;
				// Stop task on main CPU
				TaskExitTaskWithID(taskctx, s_cliCtx.userTaskID, 0); // Sig:0, terminate process if no debugger is attached

				// Stop all other tasks on helper CPUs
				{
					struct STaskContext* tctx1 = GetTaskContext(1);
					for (uint32_t i=1; i<tctx1->numTasks; ++i)
						TaskExitTaskWithID(tctx1, i, 0);
					struct STaskContext* tctx2 = GetTaskContext(2);
					for (uint32_t i=1; i<tctx2->numTasks; ++i)
						TaskExitTaskWithID(tctx2, i, 0);
				}
			}
			break;

			case 8:		// Backspace
			{
				s_cliCtx.cmdLen--;
				++s_cliCtx.refreshConsoleOut;
				if (s_cliCtx.cmdLen<0)
					s_cliCtx.cmdLen = 0;
			}
			break;

			case 27:	// ESC
			{
				s_cliCtx.cmdLen = 0;
				++s_cliCtx.refreshConsoleOut;
				// TODO: Erase current line
			}
			break;

			default:
			{
				// Do not enqueue characters into command string if we're running some app
				if(taskctx->numTasks <= 2)
				{
					++s_cliCtx.refreshConsoleOut;
					s_cliCtx.cmdString[s_cliCtx.cmdLen++] = (char)asciicode;
					if (s_cliCtx.cmdLen > 126)
						s_cliCtx.cmdLen = 126;
				}
			}
			break;
		}
	}

	return execcmd;
}

void _CLITask()
{
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();

	ShowVersion(kernelgfx);

	struct STaskContext *taskctx = GetTaskContext(0);
	while(1)
	{
		int execcmd = 0;
		execcmd = HandleCommandLine(taskctx);

		// Report task termination and reset device to default state
		struct STask *task = &taskctx->tasks[s_cliCtx.userTaskID];
		if (task->state == TS_TERMINATED)
		{
			task->state = TS_UNKNOWN;
			++s_cliCtx.refreshConsoleOut;
			taskctx->interceptUART = 0;
			DeviceDefaultState(0);
		}

		// Process or echo input only when we have no ELF running on hart#1
		if(taskctx->numTasks <= 2)
		{
			if (execcmd)
			{
				kprintf("\n");
				s_cliCtx.refreshConsoleOut += ExecuteCmd(s_cliCtx.cmdString, kernelgfx);
				// Reset command string
				s_cliCtx.cmdLen = 0;
				s_cliCtx.cmdString[0] = 0;
			}

			if (s_cliCtx.refreshConsoleOut)
			{
				s_cliCtx.refreshConsoleOut = 0;
				// properly terminate the command string
				s_cliCtx.cmdString[s_cliCtx.cmdLen] = 0;
				// Rewind cursor and print the command string
				int cx,cy;
				kgetcursor(&cx, &cy);
				ksetcursor(0, cy);
				kprintf("%s>%s ", GetWorkDir(), s_cliCtx.cmdString);
			}
		}

		TaskYield();
	}
}
