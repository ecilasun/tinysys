// Boot ROM

#include "rvcrt0.h"
#include "rombase.h"
#include "xadc.h"
#include "apu.h"
#include "gpu.h"
#include "opl2.h"
#include "dma.h"
#include "usbserial.h"
#include "usbhost.h"
#include "usbhidhandler.h"
#include "max3420e.h"
#include "max3421e.h"
#include "mini-printf.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define VERSIONSTRING "v000B"

const uint32_t s_consolebgcolor = 0x36363636;
static char s_execName[64] = "ROM";
static char s_execParam0[64] = "auto";
static uint32_t s_execParamCount = 1;

static char s_cmdString[64] = "";
static char s_workdir[64] = "sd:/";
static int32_t s_cmdLen = 0;
static uint32_t s_startAddress = 0;
static int s_refreshConsoleOut = 1;
static int s_relocOffset = 0;

static char *s_taskstates[]={
	"UNKNOWN    ",
	"PAUSED     ",
	"RUNNING    ",
	"TERMINATING",
	"TERMINATED " };

static char *s_usbdevstates[]={
	"UNKNOWN",
	"DETACHED",
	"ATTACHED",
	"ADDRESSING",
	"RUNNING",
	"ERROR",
	"HALT" };

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
	RPUFlushCache();		// Complete writes
	RPUInvalidateCache();	// Invalidate cache
	RPUBarrier();
	RPUWait();				// Complete all RPU ops

	// Wait for any pending DMA to complete
	DMAWait();

	// Set up console view
	struct EVideoContext *kernelgfx = GetKernelGfxContext();
	GPUSetWriteAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	GPUSetScanoutAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	GPUSetDefaultPalette(kernelgfx);
	kernelgfx->m_vmode = EVM_640_Wide;
	kernelgfx->m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(kernelgfx, EVS_Enable);

	// Preserve contents of screen for non-boot time
	if (_bootTime == 1)
	{
		GPUConsoleSetColors(kernelgfx, 0x0F, 0x36);
		GPUConsoleClear(kernelgfx);
	}
}

void ExecuteCmd(char *_cmd)
{
	const char *command = strtok(_cmd, " ");
	if (!command)
		return;

	uint32_t loadELF = 0;
	struct EVideoContext *kernelgfx = GetKernelGfxContext();

	if (!strcmp(command, "ls"))
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
	else if (!strcmp(command, "umount"))
	{
		UnmountDrive();
	}
	else if (!strcmp(command, "clear"))
	{
		GPUConsoleClear(kernelgfx);
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
		GPUSetVMode(kernelgfx, EVS_Disable);
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
	else if (!strcmp(command, "ps"))
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
	else if (!strcmp(command, "rm"))
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
	else if (!strcmp(command, "usb"))
	{
		int state = GetUSBDeviceState();
		kprintf("Device state: %d(%s)\n", state, s_usbdevstates[state]);
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
	else if (!strcmp(command, "cwd"))
	{
		const char *path = strtok(NULL, " ");
		// Change working directory
		if (!path)
			kprintf("usage: cwd path\n");
		else
		{
			f_chdir(path);
			strncpy(s_workdir, path, 64);
		}
	}
	else if (!strcmp(command, "ver"))
	{
		kprintf("tinysys         : " VERSIONSTRING "\n");
		kprintf("Board:          : revision 1K\n");	// TODO: need to read board and CPU data form system config
		kprintf("CPU:            : 166.67MHz\n");

		// Report USB device die versions

		uint8_t m3420rev = MAX3420ReadByte(rREVISION);
		if (m3420rev != 0xFF)
			kprintf("MAX3420(serial) : 0x%X\n", m3420rev);
		else
			kprintf("MAX3420(serial) : n/a\n");

		uint8_t m3421rev = MAX3421ReadByte(rREVISION);
		if (m3421rev != 0xFF)
			kprintf("MAX3421(host)   : 0x%X\n", m3421rev);
		else
			kprintf("MAX3421(host)   : n/a\n");
	}
	else if (!strcmp(command, "help"))
	{
		// Bright blue
		kprintf("Available commands\n");
		kprintf("ver: Show version info\n");
		kprintf("clear: Clear terminal\n");
		kprintf("cwd path: Change working directory\n");
		kprintf("ls [path]: Show list of files in cwd or path\n");
		kprintf("rm fname: Delete file\n");
		kprintf("ren oldname newname: Rename file\n");
		kprintf("ps: Show process info\n");
		kprintf("kill pid: Kill process with id pid\n");
		kprintf("usb: Show current USB peripheral state\n");
		kprintf("mount: Mount drive sd:\n");
		kprintf("umount: Unmount drive sd:\n");
		//kprintf("reloc: Set ELF relocation offset\n"); // Hidden - this is not a safe feature
		kprintf("mem: Show available memory\n");
		kprintf("reboot: Soft reboot\n");
		kprintf("Any other input will load a file from sd: with matching name\n");
		kprintf("CTRL+C terminates current program\n");
		kprintf("\n");
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
			kprintf("Virtual memory support required to run more than one ELF.\n");
		}
		else
		{
			char filename[128];
			strcpy(filename, s_workdir); // current path already contains trailing slash
			strcat(filename, command);
			strcat(filename, ".elf");

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
				strcpy(s_execName, filename);

				const char *param = strtok(NULL, " ");
				// Change working directory
				if (!param)
					s_execParamCount = 1;
				else
				{
					strncpy(s_execParam0, param, 64);
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

		while (RingBufferRead(&uartData, sizeof(uint32_t)))
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

int main()
{
	LEDSetState(0xF);
	// Set default path before we mount any storage devices
	f_chdir("sd:/");
	strncpy(s_workdir, "sd:/", 64);

	// Attempt to mount the FAT volume on micro sd card
	// NOTE: Loaded executables do not have to worry about this part
	LEDSetState(0xE);
	MountDrive();

	// Attempt to load ROM overlay, if it exists
	LEDSetState(0xD);
	s_startAddress = LoadExecutable("sd:/boot.elf", 0, false);
	if (s_startAddress != 0x0)
	{
		// TODO: Possibly we can get away with a compressed binary
		// int packedsize = fastlz_compress_level(1, rawdata, rawdatalength, outdata);
		// int unpackedsize = fastlz_decompress(indata, indatalength, rawdata, rawdatamaxsize);

		// Reset to defaults
		DeviceDefaultState(1);
		// Unmount current drive - the loaded app has to mount on their own
		UnmountDrive();
		// At this point there are no ISR, debug aid or any facilities
		// and the boot app is on its own.
		// The first thing the boot app has to do is
		// to copy itself to the ROM shadow address
		// and branch to it.
		// From thereon it can become the replacement OS or RT application
		_runExecTask();
		// Not expecting it to return
		while(1) {}
	}

	// NOTE: Since we'll loop around here again if we receive a soft reset,
	// we need to make sure all things are stopped and reset to default states
	LEDSetState(0xC);
	DeviceDefaultState(1);
	kprintf("tinysys " VERSIONSTRING "\n");

	// Set up internals
	LEDSetState(0xB);
	RingBufferReset();

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
		// High level maintenance tasks which should not be interrupted

		// Disable machine interrupts
		write_csr(mstatus, 0);

		// Deal with USB peripheral setup and data traffic
		ProcessUSBDevice();

		// Enable machine interrupts
		write_csr(mstatus, MSTATUS_MIE);
		// Yield time as soon as we're done here
		TaskYield();

		// Tasks that can be interrupted

		// Refresh console output
		if (kernelgfx->m_needBGClear)
			GPUClear(kernelgfx, s_consolebgcolor);
		if (kernelgfx->m_consoleUpdated)
			GPUConsoleResolve(kernelgfx);
	}

	return 0;
}
