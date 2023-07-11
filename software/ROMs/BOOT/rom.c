// Boot ROM

#include "rvcrt0.h"
#include "rombase.h"
#include "xadc.h"
#include "apu.h"
#include "gpu.h"
#include "opl2.h"
#include "usbserial.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define VERSIONSTRING "v1.021"

static struct EVideoContext s_gpuContext;

static char s_execName[64] = "ROM";
static char s_execParam[64] = "auto";
static char s_cmdString[128] = "";
static char s_workdir[64] = "sd:/";
static uint32_t s_execParamCount = 1;
static int32_t s_cmdLen = 0;
static uint32_t s_startAddress = 0;
static int s_refreshConsoleOut = 1;
static int s_stop_ring_buffer = 0;
static int s_stop_output = 0;
static int s_usbserialenabled = 0;

static struct SUSBContext s_usbserialctx;

void _stubTask() {
	// NOTE: This task won't actually run
	// It is a stub routine which will be stomped over by main()
	// on first entry to the timer ISR
	asm volatile("nop;");
}

// This task is a trampoline to the loaded executable
void RunExecTask()
{
	// Start the loaded executable
	asm volatile(
		"addi sp, sp, -16;"
		"sw %3, 0(sp);"		// Store argc
		"sw %1, 4(sp);"		// Store argv[1] (path to exec)
		"sw %2, 8(sp);"		// Store argv[2] (exec params)
		"sw zero, 12(sp);"	// Store argv[3] (nullptr)
		"lw s0, %0;"        // Target branch address
		"jalr s0;"          // Branch to the entry point
		"addi sp, sp, 16;"	// We most likely won't return here
		: "=m" (s_startAddress)
		: "r" (s_execName), "r" (s_execParam), "r" (s_execParamCount)
		// Clobber list
		: "s0"
	);

	// NOTE: Execution should never reach here since the ELF will invoke ECALL(0x5D) to quit
	// and will be removed from the task list, thus removing this function from the
	// execution pool.
}

void DeviceDefaultState()
{
	// Clear both audio output buffers to stop raw sound output
	APUStop();
	APUSwapBuffers();
	APUStop();
	APUSwapBuffers();

	// Stop any pending OPL2 audio output by sending a series commands
	OPL2Stop();

	// Shut down display
	s_gpuContext.m_vmode = EVM_320_Wide;
	s_gpuContext.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&s_gpuContext, EVS_Disable);
}

void receive_file(const char *savename)
{
	// TODO: Load file from remote over UART
	if (!savename)
		USBSerialWrite("usage: rcv targetfilename\n");
	else
	{
		USBSerialWrite("Waiting for file '");
		USBSerialWrite(savename);
		USBSerialWrite("'...\n");

		s_stop_ring_buffer = 1;

		uint32_t HH=0, HL=0, LH=0, LL=0;
		int state = 0;
		int done = 0;
		do
		{
			switch (state)
			{
				case 0: if (RingBufferRead(&HH, sizeof(uint32_t))) state = 1; break;
				case 1: if (RingBufferRead(&HL, sizeof(uint32_t))) state = 2; break;
				case 2: if (RingBufferRead(&LH, sizeof(uint32_t))) state = 3; break;
				case 3: if (RingBufferRead(&LL, sizeof(uint32_t))) state = 4; break;
				case 4: {
					uint32_t bytecount = ((HH&0xFF)<<24) | ((HL&0xFF)<<16) | ((LH&0xFF)<<8) | (LL&0xFF);
					USBSerialWrite("Receiving 0x");
					USBSerialWriteHex(bytecount);
					USBSerialWrite(" bytes...");

					if (bytecount > 65536)
					{
						USBSerialWrite("Unsupported file size\n");
						s_stop_ring_buffer = 0;
						return;
					}

					FIL fp;
					FRESULT res = f_open(&fp, savename, FA_CREATE_ALWAYS | FA_WRITE);
					if (res == FR_OK)
					{
						// 4- Write all future bytes to file
						uint32_t cnt = 0;
						uint32_t D;
						while (cnt != bytecount)
						{
							if (RingBufferRead(&D, sizeof(uint32_t)))
							{
								D = D&0xFF;
								UINT wb = 0;
								f_write(&fp, &D, 1, &wb);
								/*if ((cnt%64)==0)
									*IO_UARTRX = 0xFF;*/ // TODO: We need to send ack after each packet, also add CRC check.
								++cnt;
							}
						}
						f_close(&fp);
						USBSerialWrite("done\n");
						state = 5;
					}
					else
						USBSerialWrite("Target file could not be created\n");

					break;
				}
				default:
					done = 1;
				break;
			}
		} while (!done);
		
		s_stop_ring_buffer = 0;
	}
}

void ExecuteCmd(char *_cmd)
{
	const char *command = strtok(_cmd, " ");
	if (!command)
		return;

	uint32_t loadELF = 0;

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
		USBSerialWrite("\033[H\033[0m\033[2J");
	}
	else if (!strcmp(command, "mem"))
	{
		USBSerialWrite("Available memory:");
		uint32_t inkbytes = core_memavail()/1024;
		uint32_t inmbytes = inkbytes/1024;
		if (inmbytes!=0)
		{
			USBSerialWriteDecimal(inmbytes);
			USBSerialWrite(" Mbytes\n");
		}
		else
		{
			USBSerialWriteDecimal(inkbytes);
			USBSerialWrite(" Kbytes\n");
		}
	}
	else if (!strcmp(command, "ps"))
	{
		struct STaskContext *ctx = GetTaskContext();
		if (ctx->numTasks==1)
			USBSerialWrite("No tasks running\n");
		else
		{
			for (int i=1;i<ctx->numTasks;++i)
			{
				struct STask *task = &ctx->tasks[i];
				USBSerialWrite(" task:");
				USBSerialWrite(task->name);
				USBSerialWrite(" len:");
				USBSerialWriteDecimal(task->runLength);
				USBSerialWrite(" state:");
				USBSerialWriteDecimal(task->state);
				USBSerialWrite(" PC:");
				USBSerialWriteHex(task->regs[0]);
				USBSerialWrite("\n");
			}
		}
	}
	else if (!strcmp(command, "rm"))
	{
		const char *path = strtok(NULL, " ");
		if (!path)
			USBSerialWrite("usage: rm fname\n");
		else
			remove(path);
	}
	else if (!strcmp(command, "cwd"))
	{
		const char *path = strtok(NULL, " ");
		// Change working directory
		if (!path)
			USBSerialWrite("usage: cwd path\n");
		else
		{
			f_chdir(path);
			strncpy(s_workdir, path, 64);
		}
	}
	else if (!strcmp(command, "rcv"))
	{
		const char *savename = strtok(NULL, " ");
		receive_file(savename);
	}
	else if (!strcmp(command, "ver"))
	{
		USBSerialWrite("tinysys " VERSIONSTRING "\n");
	}
	else if (!strcmp(command, "tmp"))
	{
		USBSerialWrite("Temperature:");
		uint32_t ADCcode = *XADCTEMP;
		//float temp_centigrates = (ADCcode*503.975f)/4096.f-273.15f;
		uint32_t temp_centigrates = (ADCcode*503975)/4096000-273;
		USBSerialWriteDecimal(temp_centigrates);
		USBSerialWrite("\n");
	}
	else if (!strcmp(command, "usb"))
	{
		// Start USB port with serial device configuration
		if (s_usbserialenabled == 0)
		{
			USBSerialWrite("Initializing USB serial device\n");
			s_usbserialenabled = USBSerialInit(1);
			USBSerialWrite("MAX3420 die rev# ");
			USBSerialWriteHexByte(MAX3420ReadByte(rREVISION));
			USBSerialWrite("\n");
		}
		else
			USBSerialWrite("USB serial already enabled\n");
	}
	else if (!strcmp(command, "help"))
	{
		// Bright blue
		USBSerialWrite("\033[0m\n\033[94m");
		USBSerialWrite("Available commands\n");
		USBSerialWrite("ver: Show version info\n");
		USBSerialWrite("clear: Clear terminal\n");
		USBSerialWrite("cwd path: Change working directory\n");
		USBSerialWrite("ls [path]: Show list of files in cwd or path\n");
		USBSerialWrite("rm fname: Delete file\n");
		USBSerialWrite("rcv fname: Receive and save a file to storage\n");
		USBSerialWrite("ps: Show process info\n");
		USBSerialWrite("mount: Mount drive sd:\n");
		USBSerialWrite("umount: Unmount drive sd:\n");
		USBSerialWrite("usb: Start USB serial port\n");
		USBSerialWrite("mem: Show available memory\n");
		USBSerialWrite("tmp: Show device temperature\n");
		USBSerialWrite("Any other input will load a file from sd: with matching name\n");
		USBSerialWrite("CTRL+C terminates current program\n");
		USBSerialWrite("\033[0m\n");
	}
	else // Anything else defers to being a command on storage
		loadELF = 1;

	if (loadELF)
	{
		struct STaskContext* tctx = GetTaskContext();
		// Temporary measure to avoid loading another executable while the first one is running
		// until we get a virtual memory device
		if (tctx->numTasks>1)
		{
			USBSerialWrite("Virtual memory support required to run more than one task.\n");
		}
		else
		{
			s_stop_output = 1;
			char filename[128];
			strcpy(filename, s_workdir); // current path already contains trailing slash
			strcat(filename, command);
			strcat(filename, ".elf");

			// First parameter is excutable name
			s_startAddress = LoadExecutable(filename, true);
			strcpy(s_execName, filename);

			const char *param = strtok(NULL, " ");
			// Change working directory
			if (!param)
				s_execParamCount = 1;
			else
			{
				strncpy(s_execParam, param, 64);
				s_execParamCount = 2;
			}

			// TODO: Push argc/argv onto stack

			// If we succeeded in loading the executable, the trampoline task can branch into it.
			// NOTE: Without code relocation or virtual memory, two executables will ovelap when loaded
			// even though each gets a new task slot assigned.
			// This will cause corruption of the runtime environment.
			if (s_startAddress != 0x0)
				TaskAdd(tctx, command, RunExecTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);
		}
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
	s_startAddress = LoadExecutable("sd:/boot.elf", false);
	if (s_startAddress != 0x0)
	{
		// The boot executable is responsible for
		// setting the entire hardware up including
		// copying itself to the ROM shadow address
		// at ROMSHADOW_START (0x0FFE0000 by default)
		// This is easiest done by having main()
		// copy a payload embedded in the binary itself
		// and branch to _start() of the payload.
		RunExecTask();
		// Do not let rest of the code run on return
		while(1) {}
	}

	// NOTE: Since we'll loop around here again if we receive a soft reset,
	// we need to make sure all things are stopped and reset to default states
	LEDSetState(0xC);
	DeviceDefaultState();

	// Set up internals
	LEDSetState(0xB);
	RingBufferReset();

	// Create task context
	LEDSetState(0xA);
	struct STaskContext *taskctx = CreateTaskContext();

	// With current layout, OS takes up a very small slices out of whatever is left from other tasks
	LEDSetState(0x9);
	TaskAdd(taskctx, "kernelStub", _stubTask, TS_RUNNING, QUARTER_MILLISECOND_IN_TICKS);

	// Ready to start, silence LED activity since other systems need it
	LEDSetState(0x0);

	// Start the timer and hardware interrupt handlers.
	// This is where all task switching and other interrupt handling occurs
	InstallISR();

	// Set up kernel side usb context
    USBSerialSetContext(&s_usbserialctx);
	// Start USB serial
	s_usbserialenabled = USBSerialInit(1);
	//USBSerialWrite("MAX3420 die rev# ");
	//USBSerialWriteHexByte(MAX3420ReadByte(rREVISION));
	//USBSerialWrite("\n");

	// // Splash - we drop to embedded OS if there's no boot image (boot.elf)
	// USBSerialWrite("\033[H\033[0m\033[2J\033[96;40mtinysys embedded OS " VERSIONSTRING "\033[0m\n\n");
	// USBSerialWrite("Use 'help' a list of available commands\n");

	// Main CLI loop
	while (1)
	{
		// Echo all of the characters we can find back to the sender
		uint32_t uartData = 0;
		int execcmd = 0;

		// NOTE: In debug mode none of the following UART dependent code will work
		while (!s_stop_ring_buffer && RingBufferRead(&uartData, sizeof(uint32_t)))
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
					TaskExitTaskWithID(taskctx, 1, 0); // Sig:0, terminate process if no debugger is attached
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
					s_cmdString[s_cmdLen++] = (char)asciicode;
					if (s_cmdLen > 127)
						s_cmdLen = 127;
				}
				break;
			}
		}

		// Report task termination
		struct STask *task = &taskctx->tasks[1];
		if (task->state == TS_TERMINATED)
		{
			s_stop_output = 0;
			task->state = TS_UNKNOWN;
			USBSerialWrite("\n");
			USBSerialWrite(task->name);
			USBSerialWrite("' terminated (0x");
			USBSerialWriteHex(task->exitCode);
			USBSerialWrite(")\n");
			++s_refreshConsoleOut;
			DeviceDefaultState();
		}

		if (execcmd)
		{
			++s_refreshConsoleOut;
			if (!s_stop_output)
				USBSerialWrite("\n");
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
			if (!s_stop_output)
			{
				USBSerialWrite("\033[2K\r");
				USBSerialWrite(s_workdir);
				USBSerialWrite(":");
				USBSerialWrite(s_cmdString);
			}
		}
	}

	return 0;
}
