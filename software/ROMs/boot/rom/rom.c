// Boot ROM

#include "rvcrt0.h"
#include "rombase.h"
#include "xadc.h"
#include "apu.h"
#include "vpu.h"
#include "opl2.h"
#include "dma.h"
#include "usbserial.h"
#include "usbhost.h"
#include "usbhidhandler.h"
#include "usbserialhandler.h"
#include "max3420e.h"
#include "max3421e.h"
#include "mini-printf.h"
#include "debugstub.h"
#include "keyringbuffer.h"
#include "serialinringbuffer.h"
#include "serialoutringbuffer.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define VERSIONSTRING "R004"
#define DEVVERSIONSTRING "D006"

// For ROM image residing on the device:
const uint8_t s_consolefgcolor = 0x2A; // Ember
const uint8_t s_consolebgcolor = 0x11; // Dark gray

// For ROM image loaded from storage:
const uint8_t s_devfgcolor = 0x02; // Green
const uint8_t s_devbgcolor = 0x11; // Dark gray

static char s_execName[32] = "ROM";
static char s_execParam0[32] = "auto";
static uint32_t s_execParamCount = 1;

static char s_cmdString[64] = "";
static char s_workdir[48];
static char s_pathtmp[48];
static int32_t s_cmdLen = 0;
static uint32_t s_startAddress = 0;
static int s_refreshConsoleOut = 1;
static int s_relocOffset = 0;

static const char *s_taskstates[]={
	"UNKNOWN    ",
	"PAUSED     ",
	"RUNNING    ",
	"TERMINATING",
	"TERMINATED " };

static struct SUSBSerialContext s_usbserialctx;
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
	// Clear both audio output buffers to stop raw sound output
	APUStop();
	APUSwapBuffers();
	APUStop();
	APUSwapBuffers();

	// Stop any pending OPL2 audio output by sending a series commands
	OPL2Stop();

	// Turn off LEDs
	LEDSetState(0x0);

	// Wait for any pending raster ops to complete
	//RPUFlushCache();		// Complete writes
	//RPUInvalidateCache();	// Invalidate cache
	//RPUBarrier();
	//RPUWait();				// Complete all RPU ops

	// Wait for any pending DMA to complete
	DMAWait();

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
		uint32_t waterMark = read_csr(0xFF0);
		if (waterMark == 0) // On-device ROM
			VPUConsoleSetColors(kernelgfx, s_consolefgcolor, s_consolebgcolor);
		else // Overlay ROM (white on blue)
			VPUConsoleSetColors(kernelgfx, s_devfgcolor, s_devbgcolor);
		VPUConsoleClear(kernelgfx);
	}

	// Clear screen to overlay loader color
	if (_bootTime == 2)
		VPUClear(kernelgfx, 0x09090909);
}

void ExecuteCmd(char *_cmd)
{
	const char *command = strtok(_cmd, " ");
	if (!command)
		return;

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
	else if (!strcmp(command, "reloc"))
	{
		const char *offset = strtok(NULL, " ");
		if (!offset)
		{
			kprintf("Relocation offset reset\n");
			s_relocOffset = 0;
		}
		else
		{
			s_relocOffset = atoi(offset);
		}
	}
	else if (!strcmp(command, "reboot"))
	{
		// Blank
		VPUConsoleClear(kernelgfx);
		VPUClear(kernelgfx, 0x00000000);
		VPUSetVMode(kernelgfx, EVS_Disable);
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
		struct STaskContext *ctx = GetTaskContext();
		if (ctx->numTasks==1)
			kprintf("No tasks running\n");
		else
		{
			for (int i=0;i<ctx->numTasks;++i)
			{
				struct STask *task = &ctx->tasks[i];
				kprintf("#%d:%s PC:0x%x name:'%s'\n", i, s_taskstates[task->state], task->regs[0], task->name);
			}
		}
	}
	else if (!strcmp(command, "del"))
	{
		const char *path = strtok(NULL, " ");
		if (!path)
			kprintf("usage: rm fname\n");
		else
			remove(path);
	}
	else if (!strcmp(command, "kill"))
	{
		const char *processid = strtok(NULL, " ");
		if (!processid)
			kprintf("usage: kill processid\n");
		else
		{
			// Warning! This can also kill PID(1) which is the CLI
			struct STaskContext *ctx = GetTaskContext();
			int taskid = atoi(processid);
			TaskExitTaskWithID(ctx, taskid, 0);
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
				strncpy(s_pathtmp, "sd:/", 48);	// Prepend if not

			if (strstr(path, "sd:") != path)	// Does it begin with device name?
			{
				strncpy(s_pathtmp, "sd:/", 48);	// Prepend if not
				strcat(s_pathtmp, path);		// Rest of the path
			}
			else
				strncpy(s_pathtmp, path, 48);

			int L = strlen(s_pathtmp)-1;
			if (s_pathtmp[L] != '/')		// Does it end with a slash?
				strcat(s_pathtmp, "/");		// Append one if not

			// Finally, change to this new path
			f_chdir(s_pathtmp);
			// Save for later
			strncpy(s_workdir, s_pathtmp, 48);
		}
	}
	else if (!strcmp(command, "ver"))
	{
		kprintf("tinysys (c)2024 Engin Cilasun\n");

		uint32_t waterMark = read_csr(0xFF0);
		if (waterMark == 0)
			kprintf("ROM             : " VERSIONSTRING "\n");
		else
			kprintf("ROM             : " DEVVERSIONSTRING "\n");

		// TODO: These two values need to come from a CSR,
		// pointing at a memory location with device config data (machineconfig?)
		kprintf("Board:          : revision 2B\n");
		kprintf("CPU:            : 166.67MHz\n");

		// Report USB serial chip version
		uint8_t m3420rev = MAX3420ReadByte(rREVISION);
		if (m3420rev != 0xFF)
			kprintf("MAX3420(serial) : 0x%X\n", m3420rev);
		else
			kprintf("MAX3420(serial) : n/a\n");

		// Report USB host chip version
		uint8_t m3421rev = MAX3421ReadByte(rREVISION);
		if (m3421rev != 0xFF)
			kprintf("MAX3421(host)   : 0x%X\n", m3421rev);
		else
			kprintf("MAX3421(host)   : n/a\n");

		// Video circuit on 2B has no info we can read
		kprintf("SII164(video)   : 12bpp DVI\n");
	}
	else if (!strcmp(command, "help"))
	{
		kprintf(" COMMAND      | USAGE\n");
		kprintf(" cls          | Clear terminal                   \n");
		kprintf(" cd path      | Change working directory         \n");
		kprintf(" del fname    | Delete file                      \n");
		kprintf(" dir [path]   | Show list of files in cwd or path\n");
		kprintf(" kill pid     | Kill process with id pid         \n");
		kprintf(" mem          | Show available memory            \n");
		kprintf(" mount        | Mount drive sd:                  \n");
		kprintf(" proc         | Show process info                \n");
		kprintf(" pwd          | Show current work directory      \n");
		kprintf(" reboot       | Soft reboot                      \n");
		kprintf(" ren old new  | Rename file from old to new name \n");
		kprintf(" unmount      | Unmount drive sd:                \n");
		kprintf(" ver          | Show version info                \n");

		// Hidden commands for dev mode
		uint32_t waterMark = read_csr(0xFF0);
		if (waterMark != 0)
		{
			kprintf("\nDEV MODE SPECIFIC\n");\
			kprintf(" COMMAND      | USAGE\n");
			kprintf(" reloc        | Set ELF relocation offset (test) \n");
		}
	}
	else // Anything else defers to being a command on storage
		loadELF = 1;

	if (loadELF)
	{
		struct STaskContext* tctx = GetTaskContext();
		// Temporary measure to avoid loading another executable while the first one is running
		// until we get a virtual memory device
		if (tctx->numTasks>2)
		{
			kprintf("Virtual memory / code relocator not implemented.\n");
		}
		else
		{
			char filename[32];
			strcpy(filename, s_workdir);	// Current path already contains a trailing slash
			strcat(filename, command);		// User supplied string
			strcat(filename, ".elf");		// We don't expect command to contain the .elf extension

			// First parameter is excutable name
			// TODO: ELF relocation on load to avoid exec+data+stack overlap
			s_startAddress = LoadExecutable(filename, s_relocOffset, true);
			// TODO: Scan and push all argv and the correct argc onto stack

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

				TaskAdd(tctx, command, _runExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
			}
		}
	}
}

void _cliTask()
{
	while(1)
	{
		struct STaskContext *taskctx = GetTaskContext();

		// Echo all of the characters we can find back to the sender
		uint32_t uartData = 0;
		int execcmd = 0;

		// Keyboard input
		while (KeyRingBufferRead(&uartData, sizeof(uint32_t)))
		{
			uint8_t asciicode = (uint8_t)(uartData&0xFF);

			++s_refreshConsoleOut;
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
					execcmd++;
					// TODO: This has to be handled differently and terminate active task not hardcoded one
					TaskExitTaskWithID(taskctx, 2, 0); // Sig:0, terminate process if no debugger is attached
				}
				break;

				case 8:		// Backspace
				{
					s_cmdLen--;
					if (s_cmdLen<0)
						s_cmdLen = 0;
				}
				break;

				case 27:	// ESC
				{
					s_cmdLen = 0;
					// TODO: Erase current line
				}
				break;

				default:
				{
					// Do not enqueue characters into command string if we're running some app
					if(taskctx->numTasks <= 2)
					{
						s_cmdString[s_cmdLen++] = (char)asciicode;
						if (s_cmdLen > 63)
							s_cmdLen = 63;
					}
				}
				break;
			}
		}

		// Report task termination
		struct STask *task = &taskctx->tasks[2];
		if (task->state == TS_TERMINATED)
		{
			task->state = TS_UNKNOWN;
			//kprintf("\n'%s' terminated (0x%x)\n", task->name, task->exitCode);
			++s_refreshConsoleOut;
			DeviceDefaultState(0);
		}

		// Process or echo input only when we have no ELF running
		if(taskctx->numTasks <= 2)
		{
			if (execcmd)
			{
				++s_refreshConsoleOut;
				kprintf("\n");
				ExecuteCmd(s_cmdString);
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
void __attribute__((aligned(64))) CopyPayloadAndChainOverlay(void *source)
{
	// Copy this code outside ROM area
	asm volatile(
		"mv s0, %0;"			// Payload address: @payload
		"lui s1, 0x0;"			// Target: 0x00000000
		"addi s2, zero, 0x10;"	// Count:  64/4 (0x10)
		"movepayload:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, movepayload;"
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
void __attribute__((aligned(64))) Payload()
{
	asm volatile(
		"lui s0, 0x00010;"		// Source: 0x00010000
		"lui s1, 0x0FFE0;"		// Target: 0x0FFE0000
		"lui s2, 0x4;"			// Count:  65536/4 (0x4000)
		"copyloop:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, copyloop;"
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
		write_csr(0xFF0, 0xFFFFFFFF);

		return 1;
	}

	return 0;
}

int main()
{
	LEDSetState(0xF);
	// Set default path before we mount any storage devices
	f_chdir("sd:/");
	strncpy(s_workdir, "sd:/", 48);

	// Attempt to mount the FAT volume on micro sd card
	// NOTE: Loaded executables do not have to worry about this part
	LEDSetState(0xE);
	MountDrive();

	// Attempt to load ROM overlay, if it exists
	LEDSetState(0xD);
	// Watermark register is zero on hard boot
	uint32_t waterMark = read_csr(0xFF0);
	if ((waterMark == 0) && LoadOverlay("sd:/rom.bin"))
	{
		// Point of no return. Literally.
		CopyPayloadAndChainOverlay(Payload);

		// We should never come back here
		while(1) {}
	}

	// NOTE: Since we'll loop around here again if we receive a soft reset,
	// we need to make sure all things are stopped and reset to default states
	LEDSetState(0xC);
	DeviceDefaultState(1);
	if (waterMark == 0)
		kprintf("ROM: " VERSIONSTRING "\n");
	else
		kprintf("ROM: " DEVVERSIONSTRING "\n");

	// Set up ring buffers
	LEDSetState(0xB);
	KeyRingBufferReset();
	SerialInRingBufferReset();
	SerialOutRingBufferReset();

	// Create task context
	LEDSetState(0xA);
	struct STaskContext *taskctx = CreateTaskContext();

	// With current layout, OS takes up a very small slices out of whatever is left from other tasks
	LEDSetState(0x9);
	TaskAdd(taskctx, "idle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
	// Command line interpreter task
	TaskAdd(taskctx, "cmd", _cliTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	// Ready to start, silence LED activity since other systems need it
	LEDSetState(0x0);

	// Start the timer and hardware interrupt handlers.
	// This is where all task switching and other interrupt handling occurs
	InstallISR();

	// Start USB serial peripheral
	USBSerialSetContext(&s_usbserialctx);
	USBSerialInit(1);

	// Start USB host
	InitializeUSBHIDData();
	USBHostSetContext(&s_usbhostctx);
	USBHostInit(1);

	// Main CLI loop
	struct EVideoContext *kernelgfx = GetKernelGfxContext();
	while (1)
	{
		// ----------------------------------------------------------------
		// High level maintenance tasks which should not be interrupted
		// ----------------------------------------------------------------

		// Disable machine interrupts
		write_csr(mstatus, 0);

		// Deal with USB peripheral setup and data traffic
		ProcessUSBDevice();

		// Emit outgoing serial data
		USBEmitBufferedOutput();

		// GDB stub
		ProcessGDBRequest();

		// Enable machine interrupts
		write_csr(mstatus, MSTATUS_MIE);

		// Yield time as soon as we're done here
		TaskYield();

		// ----------------------------------------------------------------
		// Tasks which can be interrupted
		// ----------------------------------------------------------------

		// Refresh console output
		if (kernelgfx->m_consoleUpdated)
			VPUConsoleResolve(kernelgfx);
	}

	return 0;
}
