// Boot ROM

#include "rvcrt0.h"
#include "rombase.h"
#include "xadc.h"
#include "apu.h"
#include "vpu.h"
#include "dma.h"
#include "uart.h"
#include "usbhost.h"
#include "usbhidhandler.h"
#include "max3421e.h"
#include "mini-printf.h"
#include "keyringbuffer.h"
#include "serialinringbuffer.h"
#include "serialoutringbuffer.h"
#include "serialinput.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// On-device version
#define VERSIONSTRING "v1.09"

static char s_execName[33] = "";
static char s_execParam0[33] = "";
static uint32_t s_execParamCount = 1;
// ID of task executing on hart#1
static uint32_t s_userTaskID = 0;

static char s_cmdString[128] = "";
static char s_pathtmp[PATH_MAX];
static int32_t s_cmdLen = 0;
static uint32_t s_startAddress = 0;
static int s_refreshConsoleOut = 1;
static int s_runOnCPU = 0;

static const char *s_taskstates[]={ "????", "HALT", "EXEC", "TERM", "DEAD"};
static const char *s_regnames[]={"PC", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s1", "s1", "t3", "t4", "t5", "t6"};

static struct SUSBHostContext s_usbhostctx;

void UnrecoverableError()
{
	asm volatile(
		"csrw mie, 0;"			// Disable all interrupt types
		"csrw mstatus, 0;"		// Disable machine interrupts
	);

	// We have experienced a situation which requires a hard-boot
	while (1)
	{
		LEDSetState(0xF);
		E32Sleep(ONE_SECOND_IN_TICKS);
		LEDSetState(0x0);
		E32Sleep(ONE_SECOND_IN_TICKS);
	}
}

void _stubTask()
{
	// NOTE: This task won't actually run
	// It is a stub routine which will be stomped over by main()
	// on first entry to the timer ISR
	while(1)
	{
		asm volatile("nop;");
	}
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
		: "=m" (s_startAddress)
		: "r" (s_execName), "r" (s_execParam0), "r" (s_execParamCount)
		// Clobber list
		: "s0"
	);

	// NOTE: Execution should never reach here since the ELF will invoke ECALL(0x5D) to quit
	// and will be removed from the task list, thus removing this function from the
	// execution pool.
}

void DeviceDefaultState(int _bootTime)
{
	// Stop output
	APUSetSampleRate(ASR_Halt);

	// Turn off LEDs
	LEDSetState(0x0);

	// TODO: Wait for any pending raster ops to complete

	// Wait for any pending DMA to complete
	DMAWait(CPUIncoherent);

	// Set up console view
	struct EVideoContext *kernelgfx = GetKernelGfxContext();
	VPUSetWriteAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	VPUSetScanoutAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	VPUSetDefaultPalette(kernelgfx);
	kernelgfx->m_vmode = EVM_640_Wide;
	kernelgfx->m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(kernelgfx, EVS_Enable);

	// Preserve contents of screen for non-boot time
	if (_bootTime)
	{
		VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
		VPUConsoleClear(kernelgfx);
	}

	// Clear screen to overlay loader color
	if (_bootTime == 2)
		VPUClear(kernelgfx, 0x09090909);
}

void ShowVersion(int waterMark)
{
	struct EVideoContext *kernelgfx = GetKernelGfxContext();
	VPUConsoleSetColors(kernelgfx, CONSOLEWHITE, CONSOLEGRAY);
	kprintf("\n                                                   \n");

	kprintf(" OS version          : " VERSIONSTRING "                       \n");
	if (waterMark != 0)
		kprintf(" ROM overlay loaded from sdcard                    \n");
	else
		kprintf(" Using ROM image from firmware                     \n");

	// TODO: These two values need to come from a CSR,
	// pointing at a memory location with device config data (machineconfig?)
	// That memory location will in turn point at an onboard EEPROM we can
	// read device versions/presence from.
	kprintf(" Board               : issue 2E:2024 Engin Cilasun \n");
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

uint32_t ExecuteCmd(char *_cmd)
{
	const char *command = strtok(_cmd, " ");
	if (!command)
		return 1;

	uint32_t loadELF = 0;
	struct EVideoContext *kernelgfx = GetKernelGfxContext();

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
		// TODO: Instead, talk to ESP32-C6 to get it to hard-reboot us
		VPUConsoleClear(kernelgfx);
		// Clear to reboot color
		VPUClear(kernelgfx, 0x0C0C0C0C);
		VPUSetVMode(kernelgfx, EVS_Disable);
		// Remove watermark since we might have deleted / changed the rom image before reboot
		write_csr(0xFF0, 0x0);

		// Reset to default ROM entry points without override (thus ending up in main())
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
				kprintf("No tasks running on core #%d\n", hartid);
			}
			else
			{
				kprintf("%d tasks running on core #%d\n", ctx->numTasks, hartid);
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
			if (krealpath(path, s_pathtmp))
			{
				// Append missing trailing slash
				int L = (int)strlen(s_pathtmp);
				if (L != 0 && s_pathtmp[L-1] != '/')
					strcat(s_pathtmp, "/");

				// Finally, change to this new path
				FRESULT cwdres = f_chdir(s_pathtmp);
				if (cwdres == FR_OK)
					SetWorkDir(s_pathtmp);
				else
				{
					kprintf("invalid path(0) '%s'\n", s_pathtmp);
				}
			}
			else
				kprintf("invalid path(1) '%s'\n", s_pathtmp);
		}
	}
	else if (!strcmp(command, "ver"))
	{
		uint32_t waterMark = read_csr(0xFF0);
		ShowVersion(waterMark);
	}
	else if(!strcmp(command, "runon"))
	{
		const char *runcpu = strtok(NULL, " ");
		if (!runcpu)
			kprintf("usage: runon cpu\n");
		else
			s_runOnCPU = atoi(runcpu);
		kprintf("Will run next task on CPU#%d\n", s_runOnCPU);
	}
	else if (!strcmp(command, "help"))
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
	else // Anything else defers to being a command on storage
		loadELF = 1;

	if (loadELF)
	{
		// TODO: load user ELF files on HART#1.
		struct STaskContext* tctx[2] = {GetTaskContext(0), GetTaskContext(1)};
		int32_t taskcounts[2] = {tctx[0]->numTasks, tctx[1]->numTasks};
		int32_t maxcounts[2] = {2, 1};

		// Temporary measure to avoid loading another executable while the first one is running
		// until we get a virtual memory device
		if (taskcounts[s_runOnCPU] > maxcounts[s_runOnCPU])
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
			s_startAddress = LoadExecutable(filename, 0, false);
			// TODO: Scan and push all argv and the correct argc onto stack

			// If we could not find the executable where we are, look into the 'sys/bin' directory
			if (s_startAddress == 0x0)
			{
				strcpy(filename, "sd:/sys/bin/");
				strcat(filename, command);
				strcat(filename, ".elf");
				s_startAddress = LoadExecutable(filename, 0, false);
			}

			// If we succeeded in loading the executable, the trampoline task can branch into it.
			// NOTE: Without code relocation or virtual memory, two executables will ovelap when loaded
			// even though each gets a new task slot assigned.
			// This will cause corruption of the runtime environment.
			if (s_startAddress != 0x0)
			{
				// Make sure everything is flushed to RAM and the instruction cache is invalidated
				CFLUSH_D_L1;
				FENCE_I;

				strncpy(s_execName, filename, 32);

				const char *param = strtok(NULL, " ");
				// Change working directory
				if (!param)
					s_execParamCount = 1;
				else
				{
					strncpy(s_execParam0, param, 32);
					s_execParamCount = 2;
				}

				s_userTaskID = TaskAdd(tctx[s_runOnCPU], command, _runExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

				return 0;
			}
			else
				kprintf("Executable '%s' not found\n", command);
		}
	}

	return 1;
}

void _cliTask()
{
	while(1)
	{
		struct STaskContext *taskctx = GetTaskContext(0);

		// Echo all of the characters we can find back to the sender
		uint32_t uartData = 0;
		int execcmd = 0;

		// Keyboard input
		while (KeyRingBufferRead(&uartData, sizeof(uint32_t)))
		{
			uint8_t asciicode = (uint8_t)(uartData&0xFF);

			switch (asciicode)
			{
				case 10:
				case 13:	// Return / Enter
				{
					execcmd++;
				}
				break;

				case 3:		// EXT (Ctrl+C)
				{
					++s_refreshConsoleOut;
					execcmd++;
					if (s_runOnCPU == 0)
						TaskExitTaskWithID(taskctx, s_userTaskID, 0); // Sig:0, terminate process if no debugger is attached

					// Stop all other tasks on helper CPUs
					{
						struct STaskContext* tctx1 = GetTaskContext(1);
						for (uint32_t i=1; i<tctx1->numTasks; ++i)
							TaskExitTaskWithID(tctx1, i, 0);
					}
				}
				break;

				case 8:		// Backspace
				{
					++s_refreshConsoleOut;
					s_cmdLen--;
					if (s_cmdLen<0)
						s_cmdLen = 0;
				}
				break;

				case 27:	// ESC
				{
					++s_refreshConsoleOut;
					s_cmdLen = 0;
					// TODO: Erase current line
				}
				break;

				default:
				{
					// Do not enqueue characters into command string if we're running some app
					if(taskctx->numTasks <= 2)
					{
						++s_refreshConsoleOut;
						s_cmdString[s_cmdLen++] = (char)asciicode;
						if (s_cmdLen > 126)
							s_cmdLen = 126;
					}
				}
				break;
			}
		}

		// Report task termination
		struct STask *task = &taskctx->tasks[s_userTaskID];
		if (task->state == TS_TERMINATED)
		{
			task->state = TS_UNKNOWN;
			++s_refreshConsoleOut;
			DeviceDefaultState(0);
		}

		// Process or echo input only when we have no ELF running on hart#1
		if(taskctx->numTasks <= 2)
		{
			if (execcmd)
			{
				kprintf("\n");
				s_refreshConsoleOut += ExecuteCmd(s_cmdString);
				// Rewind
				s_cmdLen = 0;
				s_cmdString[0] = 0;
			}

			if (s_refreshConsoleOut)
			{
				s_refreshConsoleOut = 0;
				s_cmdString[s_cmdLen] = 0;
				// Reset current line and emit the command string
				int cx,cy;
				kgetcursor(&cx, &cy);
				ksetcursor(0, cy);
				kprintf("%s>%s ", GetWorkDir(), s_cmdString);
			}
		}

		TaskYield();
	}
}

// Make sure we're aligned to sit at a cache line boundary
void __attribute__((aligned(64), noinline)) CopyPayloadAndChainOverlay(void *source)
{
	// Copy this code outside ROM area
	asm volatile(
		"csrw mie, 0;"			// Disable all interrupt types
		"csrw mstatus, 0;"		// Disable machine interrupts
		"mv s0, %0;"			// Payload address: @payload
		"lui s1, 0x0;"			// Target: 0x00000000
		"addi s2, zero, 0x10;"	// Count:  64/4 (0x10)
		"copypayloadloop:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, copypayloadloop;"
		".insn 0xFC000073;"		// Invalidate D$ and I$ (CFLUSH_D_L1)
		"fence.i;"				// Discard anything previously in I$ which might point at the CopyOverlayToROM function's new location
		"lui s0, 0x0;"			// Branch to 0x00000000 which will then copy the ROM from its temporary position onto ROM shadow area
		"jalr s0;"
		:
		// Return values
		:
		// Input parameters
		"r" (source)
		:
		// Clobber list
	);
}

// This is the payload to copy (56 bytes the copy loop copies over 64 in total, adjust to cover all code)
void __attribute__((aligned(64), noinline)) CopyOverlayToROM()
{
	// NOTE: Chained ROM must invalidate I$ on entry!
	asm volatile(
		"lui s0, 0x00010;"		// Source: 0x00010000 (Overlay buffer)
		"lui s1, 0x0FFE0;"		// Target: 0x0FFE0000 (ROM Shadow)
		"lui s2, 0x4;"			// Count:  65536/4 (0x4000, 64Kbytes)
		"copyoverlayloop:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, copyoverlayloop;"
		".insn 0xFC000073;"		// Invalidate & Write Back D$ (CFLUSH.D.L1)
		"lui s0, 0x0FFE0;"		// Branch to reset vector: 0x0FFE0000
		"jalr s0;"				// NOTE: ROM must invalidate I$ on entry
	);
}

uint32_t LoadOverlay(const char *filename)
{
	FIL fp;
	FRESULT fr = f_open(&fp, filename, FA_READ);

	// If the binary blob exists, load it into memory
	if (fr == FR_OK)
	{
		// Reset device to overlay time defaults
		DeviceDefaultState(2);

		FSIZE_t fileSize = f_size(&fp);
		UINT readsize = 0;
		uint32_t *overlay = (uint32_t *)0x00010000; // Overlay buffer
		__builtin_memset(overlay, 0, 65536);		// Clear overlay buffer to zeros
		f_read(&fp, overlay, fileSize, &readsize);	// Load the overlay binary
		f_close(&fp);								// Done with file access
		f_mount(NULL, "sd:", 1);					// Unmount the SD card
		CFLUSH_D_L1;								// Flush D$ to RAM

		// New ROM image will do its own thing to initialize the CPUs,
		// remove our previous function pointers to branch to on reboot.
		E32SetupCPU(1, (void*)0x0);
		E32SetupCPU(0, (void*)0x0);

		// Watermark register will retain this value on soft reboot
		// so that when we load the overlay (which is the same code as this)
		// we won't attempt to re-load the overlay over and over.
		// NOTE: Only hart#0 updates its watermark register
		write_csr(0xFF0, 0xFFFFFFFF);

		return 1;
	}

	return 0;
}

// Core task for CPU#1 onwards
void __attribute__((aligned(64), noinline)) UserMain()
{
	// Boot sequence for CPU#1
	asm volatile(
		"fence.i;"				// Discard I$
		"csrw 0xFEE, 0;"		// Stop reset
		"csrw mstatus,0;"		// Disable all interrupts (mstatus:mie=0)
		"li sp, 0x0FFDFEF0;"	// Stack pointer for CPU#1 (256 bytes above CPU#0)
		"mv s0, sp;"			// Set frame pointer to current stack pointer
	);

	// Worker cores do not handle hardware interrupts by default,
	// only task switching (timer) and software (illegal instruction)
	uint32_t self = read_csr(mhartid);
	InitializeTaskContext(self);

	struct STaskContext *taskctx = GetTaskContext(self);
	TaskAdd(taskctx, "cpu1idle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	InstallISR(self, false, true);

	while(1)
	{
		// Kernel error halts all CPUs other than CPU#0
		// It is up to CPU#0 to detect errors and report/reset others
		if (taskctx->kernelError)
			while(1) { asm volatile("wfi;"); }

		asm volatile("wfi;");

		// Yield time back to any tasks running on this core after handling an interrupt
		/*uint64_t currentTime =*/ TaskYield();
	}
}

void HandleCPUError(struct STaskContext *ctx, const uint32_t cpu)
{
	ksetcolor(CONSOLERED, CONSOLEDIMGRAY);

	switch (ctx->kernelError)
	{
		case 1: kprintf("Unknown hardware device"); break;
		case 2: kprintf("Unknown interrupt type"); break;
		case 3: kprintf("Guru meditation"); break;
		case 4: kprintf("Illegal instruction"); break;
		case 5: kprintf("Breakpoint with no debugger attached"); break;
		default: kprintf("Unknown kernel error"); break;
	}

	kprintf(" on CPU #%d\n", cpu);

	ksetcolor(CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);

	// Dump task registers
	if (ctx->kernelError == 4)
	{
		uint32_t taskid = ctx->kernelErrorData[0];
		struct STask *task = &ctx->tasks[taskid];
		// Skip zero register and emit '0' since we save PC there
		kprintf("Task: '%s'\nIR=0x%08X\n", task->name, ctx->kernelErrorData[1]);
		for (uint32_t i=0; i<32; ++i)
			kprintf("%s=0x%08X%c", s_regnames[i], task->regs[i], (i+1)%4==0 ? '\n':' ');
		kprintf("\n");
	}

	// Clear error once handled and reported
	ctx->kernelError = 0;
}

// Core task for CPU#0
void __attribute__((aligned(64), noinline)) KernelMain()
{
	// Boot sequence for CPU#1
	asm volatile(
		"fence.i;"				// Discard I$
		"csrw 0xFEE, 0;"		// Stop reset
		"csrw mstatus,0;"		// Disable all interrupts (mstatus:mie=0)
		"li sp, 0x0FFDFFF0;"	// Stack poiner for CPU#0
		"mv s0, sp;"			// Set frame pointer to current stack pointer
	);

	// Mount FAT32 file system on SDcard, if one exists
	LEDSetState(0x1);															// xxxO
	SetWorkDir("sd:/");
	uint32_t mountSuccess = MountDrive();

	// Attempt to load ROM overlay, if it exists
	// Watermark register is zero on hard boot
	LEDSetState(0x2);															// xxOx
	uint32_t waterMark = read_csr(0xFF0);
	if (mountSuccess && (waterMark == 0))
	{
		if (LoadOverlay("sd:/boot/rom.bin"))
		{
			// Copy and branch into CopyOverlayToROM which will then chain back into the new ROM
			CopyPayloadAndChainOverlay(CopyOverlayToROM);
			// We should never arrive here
			UnrecoverableError();
		}
	}

	// Reset buffers
	LEDSetState(0x4);															// xOxx
	KeyRingBufferReset();
	SerialInRingBufferReset();
	SerialOutRingBufferReset();

	// Reset peripherals to default states
	LEDSetState(0x8);															// Oxxx
	DeviceDefaultState(1);

	// Start HID driver
	LEDSetState(0xC);															// OOxx
	InitializeUSBHIDData();
	USBHostSetContext(&s_usbhostctx);
	USBHostInit(1);

	// Worker cores do not handle hardware interrupts by default,
	// only task switching (timer) and software (illegal instruction)
	LEDSetState(0x6);															// xOOx
	uint32_t self = read_csr(mhartid);
	InitializeTaskContext(self);

	LEDSetState(0x3);			// xxOO
	struct STaskContext *taskctx[2];
	taskctx[0] = GetTaskContext(self);
	taskctx[1] = GetTaskContext(1);
	TaskAdd(taskctx[0], "cpu0idle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
	TaskAdd(taskctx[0], "cmd", _cliTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	LEDSetState(0x7);															// xOOO
	InstallISR(self, true, true);

	LEDSetState(0xE);															// OOOx
	ShowVersion(waterMark);

	// Reset secondary CPUs
	LEDSetState(0x0);															// xxxx

	//kprintf("CPU1 entry:%x mtvec:%x rst:%x\n", (uint32_t)UserMain, E32ReadMemMappedCSR(1, CSR_MSCRATCH), E32ReadMemMappedCSR(1, CSR_CPURESET));

	// Main CLI loop
	struct EVideoContext *kernelgfx = GetKernelGfxContext();
	while (1)
	{
		// ----------------------------------------------------------------
		// Tasks which can be interrupted go here
		// ----------------------------------------------------------------

		// Kernel error / crash handler for this CPU
		// TODO: Catch kernerl errors on CPU#1 onwards
		for (int cpu=0;cpu<2;++cpu)
		{
			if (taskctx[cpu]->kernelError)
				HandleCPUError(taskctx[cpu], cpu);
		}

		// Refresh console output
		if (kernelgfx->m_consoleUpdated)
			VPUConsoleResolve(kernelgfx);

		// Yield time as soon as we're done here (disables/enables interrupts)
		uint64_t currentTime = TaskYield();

		// ----------------------------------------------------------------
		// H/W related tasks which should't cause IRQs or be interrupted
		// ----------------------------------------------------------------

		clear_csr(mstatus, MSTATUS_MIE);
		ProcessUSBDevice(currentTime);
		set_csr(mstatus, MSTATUS_MIE);

		// ----------------------------------------------------------------
		// Tasks which should not switch context to avoid memory conflicts
		// ----------------------------------------------------------------

		clear_csr(mie, MIP_MTIP);
		UARTEmitBufferedOutput();
		set_csr(mie, MIP_MTIP);

		// ----------------------------------------------------------------
		// Serial input to feed to keyboard buffer
		// ----------------------------------------------------------------

		HandleSerialInput();
	}
}

int main()
{
	// Zero out the task memory (this will survive a soft reboot)
	ClearTaskMemory();

	// Reset and wake up all CPUs again, this time with their correct entry points
	E32SetupCPU(1, UserMain);
	E32ResetCPU(1);
	E32SetupCPU(0, KernelMain);
	E32ResetCPU(0);

	// We should never arrive here
	UnrecoverableError();
	return 0;
} 
