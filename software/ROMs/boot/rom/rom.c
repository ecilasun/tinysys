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
#include "gpioringbuffer.h"
#include "serialinput.h"
#include "mailbox.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// On-device version
#define VERSIONSTRING "r1.09"
// On-storage version
#define DEVVERSIONSTRING "r1.09"

static char s_execName[32] = "                               ";
static char s_execParam0[32] = "                               ";
static uint32_t s_execParamCount = 1;
// ID of task executing on hart#1
static uint32_t s_userTaskID = 0;

static char s_cmdString[64] = "";
static char s_workdir[48];
static char s_pathtmp[48];
static int32_t s_cmdLen = 0;
static uint32_t s_startAddress = 0;
static int s_refreshConsoleOut = 1;
static int s_runOnCPU = 0;

static const char *s_taskstates[]={
	"UNKNOWN    ",
	"PAUSED     ",
	"RUNNING    ",
	"TERMINATING",
	"TERMINATED " };

static struct SUSBHostContext s_usbhostctx;

void _stubTask()
{
	// NOTE: This task won't actually run
	// It is a stub routine which will be stomped over by main()
	// on first entry to the timer ISR
	asm volatile("nop;");
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
		".word 0xFC000073;"	// Invalidate & Write Back D$ (CFLUSH.D.L1)
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
	kprintf("\n                                               \n");
	kprintf(" tinysys                                       \n");

	if (waterMark == 0)
		kprintf(" ROM             : " VERSIONSTRING "                       \n");
	else
		kprintf(" ROM             : " DEVVERSIONSTRING "                       \n");

	// TODO: These two values need to come from a CSR,
	// pointing at a memory location with device config data (machineconfig?)
	// That memory location will in turn point at an onboard EEPROM we can
	// read device versions/presence from.
	kprintf(" Board           : issue 2E:2024               \n");
	kprintf(" CPU & bus clock : 150MHz                      \n");
	kprintf(" ARCH            : rv32im_zicsr_zifencei_zfinx \n");
	for (uint32_t i=0; i<16; ++i)
	{
		uint32_t isalive = MailboxRead(i, MAILSLOT_HART_AWAKE);
		if (isalive==0xCAFEB000)
			kprintf(" HART%d%s          : alive                       \n", i, i>9 ? "" : " ");
	}
	kprintf(" ESP32           : ESP32-C6-WROOM-1-N8         \n");

	// Report USB host chip version
	uint8_t m3421rev = MAX3421ReadByte(rREVISION);
	if (m3421rev != 0xFF)
		kprintf(" MAX3421(host)   : 0x%X                        \n", m3421rev);
	else
		kprintf(" MAX3421(host)   : n/a                         \n");

	// Video circuit on 2B has no info we can read so this is hardcoded
	kprintf(" SII164(video)   : 12bpp DVI                   \n");
	kprintf(" CS4344(audio)   : 11/22/44KHz stereo          \n");
	kprintf("                                               \n");
	kprintf(" Run HELP for a list of available commands     \n");
	kprintf("                                               \n\n");
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
			ListFiles(s_workdir);
		else
			ListFiles(path);
	}
	else if (!strcmp(command, "mount"))
	{
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
		// Jump to start of ROM for CPU#0, which then reboots CPU#1 accordingly
		asm volatile(
			"li s0, 0x0FFE0000;"
			"jalr s0;"
		);
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
					kprintf("#%d:%s PC:0x%x name:'%s'\n", i, s_taskstates[task->state], task->regs[0], task->name);
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
		kprintf("%s\n", s_workdir);
	}
	else if (!strcmp(command, "cd"))
	{
		const char *path = strtok(NULL, " ");
		// Change working directory
		if (!path)
			kprintf("usage: cd path\n");
		else
		{
			// Path has to follow the format: sd:/path/

			if (strstr(path, "/") == path)		// Attempt to go to root directory?
				strncpy(s_pathtmp, "sd:/", 47);	// Prepend if not

			if (strstr(path, "sd:") != path)	// Does it begin with device name?
			{
				strncpy(s_pathtmp, "sd:/", 47);	// Prepend if not
				strcat(s_pathtmp, path);		// Rest of the path
			}
			else
				strncpy(s_pathtmp, path, 47);

			int L = strlen(s_pathtmp)-1;
			if (s_pathtmp[L] != '/')		// Does it end with a slash?
				strcat(s_pathtmp, "/");		// Append one if not

			// Finally, change to this new path
			FRESULT cwdres = f_chdir(s_pathtmp);
			if (cwdres == FR_OK)
			{
				// Save for later
				strncpy(s_workdir, s_pathtmp, 48);
			}
			else
			{
				kprintf("invalid path '%s'\n", s_pathtmp);
			}
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
		kprintf("Run on cpu: #%d", s_runOnCPU);
	}
	else if (!strcmp(command, "help"))
	{
		VPUConsoleSetColors(kernelgfx, CONSOLEWHITE, CONSOLEGRAY);

		kprintf("\n                                                  \n");
		kprintf(" COMMAND      | USAGE                             \n");
		kprintf(" cls          | Clear terminal                    \n");
		kprintf(" cd path      | Change working directory          \n");
		kprintf(" del fname    | Delete file                       \n");
		kprintf(" dir [path]   | Show list of files in cwd or path \n");
		kprintf(" mem          | Show available memory             \n");
		kprintf(" pwd          | Show current work directory       \n");
		kprintf(" reboot       | Reboot the device                 \n");
		kprintf(" ren old new  | Rename file from old to new name  \n");
		kprintf(" ver          | Show version info                 \n");

		// Hidden commands for dev mode reveal when running from overlay
		uint32_t waterMark = read_csr(0xFF0);
		if (waterMark != 0)
		{
			kprintf("--------------------------------------------------\n");
			kprintf(" DEV MODE SPECIFIC - USE AT YOUR OWN RISK         \n");
			kprintf(" COMMAND      | USAGE                             \n");
			kprintf(" kill pid cpu | Kill process with id pid on CPU   \n");
			kprintf(" mount        | Mount drive sd:                   \n");
			kprintf(" proc cpu     | Show process info for given CPU   \n");
			kprintf(" runon cpu    | Run ELF files on given cpu        \n");
			kprintf(" unmount      | Unmount drive sd:                 \n");
		}

		kprintf("                                                  \n\n");
		VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
	}
	else // Anything else defers to being a command on storage
		loadELF = 1;

	if (loadELF)
	{
		// TODO: load user ELF files on HART#1.
		struct STaskContext* tctx = GetTaskContext(0);

		// Temporary measure to avoid loading another executable while the first one is running
		// until we get a virtual memory device
		if (tctx->numTasks > 2)
		{
			kprintf("Virtual memory / code relocator not implemented.\n");
		}
		else
		{
			char filename[64];
			strcpy(filename, s_workdir);	// Current path already contains a trailing slash
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
					strncpy(s_execParam0, param, 31);
					s_execParamCount = 2;
				}

				if (s_runOnCPU == 0)
					s_userTaskID = TaskAdd(tctx, command, _runExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
				else
					MailboxWrite(1, MAILSLOT_HART_EXEC, s_startAddress);

				return 0;
			}
			else
				kprintf("Executable not found\n");
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
					else
						MailboxWrite(1, MAILSLOT_HART_STOP, 1); // taskid + 1
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
						if (s_cmdLen > 62)
							s_cmdLen = 62;
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
				kprintf("%s>%s ", s_workdir, s_cmdString);
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
		".word 0xFC000073;"		// Invalidate D$ and I$
		"fence.i;"
		"lui s0, 0x0;"			// Branch to copy of payload at 0x00000000 which will then load the ROM overlay from storage
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

// This is the payload to copy (34 bytes but we can copy 64 to cover both compressed and uncompressed variants)
void __attribute__((aligned(64), noinline)) CopyOverlayToROM()
{
	asm volatile(
		"lui s0, 0x00010;"		// Source: 0x00010000
		"lui s1, 0x0FFE0;"		// Target: 0x0FFE0000
		"lui s2, 0x4;"			// Count:  65536/4 (0x4000)
		"copyoverlayloop:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, copyoverlayloop;"
		".word 0xFC000073;"		// Invalidate & Write Back D$ (CFLUSH.D.L1)
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
		// Reset to overlay time defaults and unmount SDCard
		DeviceDefaultState(2);

		FSIZE_t fileSize = f_size(&fp);
		UINT readsize = 0;
		uint32_t *overlay = (uint32_t *)0x00010000; // Overlay
		f_read(&fp, overlay, fileSize, &readsize);
		f_close(&fp);

		// Watermark register will retain this value on soft boot
		// so that when we load the overlay (which is the same code as this)
		// we won't attempt to re-load the overlay over and over.
		// NOTE: Only hart#0 updates its watermark register
		write_csr(0xFF0, 0xFFFFFFFF);

		return 1;
	}

	return 0;
}

// Core task for worker CPUs
void __attribute__((aligned(64), noinline)) workerMain()
{
	// Boot sequence for CPU#1
	asm volatile(
		"csrw mstatus,0;"		// Disable all interrupts (mstatus:mie=0)
		"fence.i;"				// Invalidate I$
		"li sp, 0x0FFDFEF0;"	// Stack is at near end of BRAM
		"mv s0, sp;"			// Set frame pointer to current stack pointer
	);

	// Play dead
	uint32_t self = read_csr(mhartid);
	MailboxWrite(self, MAILSLOT_HART_AWAKE, 0x00000000);

	// Worker cores do not handle hardware interrupts by default,
	// only task switching (timer) and software (illegal instruction)
	InitializeTaskContext(self);
	InstallISR(self, false, true);

	struct STaskContext *taskctx = GetTaskContext(self);
	TaskAdd(taskctx, "hart1idle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	// Mark us alive
	MailboxWrite(self, MAILSLOT_HART_AWAKE, 0xCAFEB000);

	while(1)
	{
		uint32_t startAddress = MailboxRead(self, MAILSLOT_HART_EXEC);
		uint32_t stopTask = MailboxRead(self, MAILSLOT_HART_STOP);

		if (stopTask != 0)
		{
			MailboxWrite(self, MAILSLOT_HART_STOP, 0x0);
			TaskExitTaskWithID(taskctx, s_userTaskID, 0); // Sig:0, terminate process if no debugger is attached
		}

		if (startAddress != 0)
		{
			MailboxWrite(self, MAILSLOT_HART_EXEC, 0x0);
			s_startAddress = startAddress;
			s_userTaskID = TaskAdd(taskctx, "ELF", _runExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
		}

		// Yield time back to any tasks running on this core after handling an interrupt
		/*uint64_t currentTime =*/ TaskYield();
	}
}

int main()
{
	// Reset all helper CPUs
	E32ResetCPU(1, workerMain);

	LEDSetState(0xF);

	// Play dead
	uint32_t self = read_csr(mhartid);
	MailboxWrite(self, MAILSLOT_HART_AWAKE, 0x00000000);

	LEDSetState(0xE);
	// Set default path before we mount any storage devices
	f_chdir("sd:/");
	strncpy(s_workdir, "sd:/", 48);

	// Attempt to mount the FAT volume on micro sd card
	// NOTE: Loaded executables do not have to worry about this part
	LEDSetState(0xD);
	MountDrive();

	// Attempt to load ROM overlay, if it exists
	LEDSetState(0xC);
	// Watermark register is zero on hard boot
	uint32_t waterMark = read_csr(0xFF0);
	if ((waterMark == 0) && LoadOverlay("sd:/boot/rom.bin"))
	{
		// Point of no return. Literally.
		CopyPayloadAndChainOverlay(CopyOverlayToROM);

		// We should never come back here
		while(1) {}
	}

	// NOTE: Since we'll loop around here again if we receive a soft reset,
	// we need to make sure all things are stopped and reset to default states
	LEDSetState(0xB);
	DeviceDefaultState(1);

	// Set up ring buffers
	LEDSetState(0xA);
	KeyRingBufferReset();
	SerialInRingBufferReset();
	SerialOutRingBufferReset();
	GPIORingBufferReset();

	// Create task context
	LEDSetState(0x9);
	InitializeTaskContext(self);
	struct STaskContext *taskctx = GetTaskContext(self);

	// With current layout, OS takes up a very small slices out of whatever is left from other tasks
	LEDSetState(0x8);
	TaskAdd(taskctx, "hart0idle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
	// Command line interpreter task
	TaskAdd(taskctx, "cmd", _cliTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	// Start the timer and hardware interrupt handlers.
	// This is where all task switching and other interrupt handling occurs
	InstallISR(self, true, true);

	LEDSetState(0x7);

	// Start USB host
	InitializeUSBHIDData();
	USBHostSetContext(&s_usbhostctx);
	USBHostInit(1);

	// Mark us alive
	MailboxWrite(self, MAILSLOT_HART_AWAKE, 0xCAFEB000);

	// Ready to start, silence LED activity since other systems need it
	LEDSetState(0x0);

	ShowVersion(waterMark);

	// Main CLI loop
	struct EVideoContext *kernelgfx = GetKernelGfxContext();
	while (1)
	{
		// ----------------------------------------------------------------
		// Tasks which can be interrupted go here
		// ----------------------------------------------------------------

		// Refresh console output
		if (kernelgfx->m_consoleUpdated)
			VPUConsoleResolve(kernelgfx);

		// Kernel error, stop everything except sys
		if (taskctx->kernelError)
		{
			for (int i=0;i<taskctx->numTasks;++i)
			{
				struct STask *task = &taskctx->tasks[i];
				task->state = TS_TERMINATING;
				task->exitCode = -1;
			}
			while(1) { asm volatile("wfi;"); }
		}

		// Yield time as soon as we're done here (disables/enables interrupts)
		uint64_t currentTime = TaskYield();

		// ----------------------------------------------------------------
		// High level maintenance tasks which should not be interrupted
		// ----------------------------------------------------------------

		// Disable machine interrupts on this core
		clear_csr(mstatus, MSTATUS_MIE);

		// Deal with USB peripheral setup and data traffic
		ProcessUSBDevice(currentTime);

		// Emit outgoing serial data
		UARTEmitBufferedOutput();

		// Enable machine interrupts on this core
		set_csr(mstatus, MSTATUS_MIE);

		// ----------------------------------------------------------------
		// GDB stub / serial keyboard input / file transfer handler
		// ----------------------------------------------------------------

		HandleSerialInput();
	}

	return 0;
}
