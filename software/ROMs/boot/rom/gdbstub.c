#include <string.h>

#include "basesystem.h"
#include "uart.h"
#include "serialinput.h"
#include "rombase.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyringbuffer.h"
#include "mini-printf.h"
#include <stdlib.h>

static char *packetData = (char *)GDB_PACKET_BUFFER;
static char *responseData = (char *)GDB_RESPONSE_BUFFER;
static uint32_t debuggerAttached = 0;
static uint32_t packetCursor = 0;
static uint32_t haveResponse = 0;
static uint32_t isackresponse = 0;
static uint32_t currentGProcess = 0;
//static uint32_t currentCProcess = 0;

uint8_t GDBChecksum(const char *data)
{
    uint8_t sum = 0;
    while (*data)
	{
        sum += (uint8_t)*data++;
    }
    return sum;
}

void GDBQSupported()
{
	packetData += 10; // Skip 'qSupported'

	// We have multiple queries in the packet, separated by ';'
	// First query starts with ':', so skip that
	if (packetData[0] == ':')
		packetData++;
	
	strcat(responseData, "PacketSize=1024;");

	// Parse queries
	char *command = strtok(packetData, ";");
	while (command != NULL)
	{
		if (strstr(command, "swbreak") == command)
		{
			if (strstr(command, "+"))
			{
				strcat(responseData, "swbreak+;");
			}
		}
		else if (strstr(command, "hwbreak") == command)
		{
			if (strstr(command, "+"))
			{
				// Can't do hardware breakpoints
				strcat(responseData, "hwbreak-;");
			}
		}
		else if (strstr(command, "vContSupported") == command)
		{
			if (strstr(command, "+"))
			{
				strcat(responseData, "vContSupported+;");
			}
		}
		else if (strstr(command, "multiprocess") == command)
		{
			if (strstr(command, "+"))
			{
				strcat(responseData, "multiprocess+;");
			}
		}
		else
		{
			kprintf("? %s\n", command);
			//strcat(responseData, "-");
		}
		command = strtok(NULL, ";");
	}

	haveResponse = 1;
}

void GDBThreadOp()
{
	// Set thread for subsequent operations

	packetData++; // Skip 'H'

	if (packetData[0] == 'g') // Hg
	{
		packetData++; // Skip 'g'

		// Parse process/thread ID
		char *threadID = strtok(packetData, ";");
		if (threadID != NULL)
		{
			kprintf("Hg: %s\n", threadID);
			strcpy(responseData, "OK");
			//currentGProcess = ??;
			haveResponse = 1;
		}
	}
	else if (packetData[0] == 'c') // Hc
	{
		packetData++; // Skip 'c'

		char *threadID = strtok(packetData, ";");
		if (threadID != NULL)
		{
			kprintf("Hc: %s\n", threadID);
			strcpy(responseData, "OK");
			//currentCProcess = ??;
			haveResponse = 1;
		}
	}
}

void GDBQTStatus()
{
	// Empty response
	strcpy(responseData, "");
	haveResponse = 1;
}

void GDBThreadInfo()
{
	struct STaskContext *ctx = _task_get_context(0);

	// GDB expects the first thread in the response to be able to stop.
	// We'll have to deny that for the OS threads.
	strcpy(responseData, "m");
	for (int i=0;i<ctx->numTasks;++i)
	{
		//struct STask *task = &ctx->tasks[i];
		// Process ID, Thread ID (we consider our tasks to be processes)
		mini_snprintf(responseData, 1023, "%sp%d.%d", responseData, i, 0);
		if (i != ctx->numTasks-1)
			strcat(responseData, ",");
	}

	haveResponse = 1;
}

void GDBQAttached()
{
	packetData += 9; // Skip 'qAttached'
	packetData++; // Skip ':'

	int processid = atoi(packetData);

	kprintf("?attached(%d)\n", processid);

	// 1: Attached to existing process, 0: Created new process
	strcpy(responseData, "1");
	haveResponse = 1;
}

void GDBMemRead()
{
	packetData++; // Skip 'm'

	// Parse address
	char *address = strtok(packetData, ",");
	if (address != NULL)
	{
		packetData += strlen(address) + 1; // Skip address

		// Parse length
		char *length = strtok(packetData, ":");
		if (length != NULL)
		{
			packetData += strlen(length) + 1; // Skip length

			uint32_t addr = strtol(address, NULL, 16);
			uint32_t len = strtol(length, NULL, 16);

			// Read memory
			uint8_t *mem = (uint8_t *)addr;
			for (int i=0; i<len; ++i)
				mini_snprintf(responseData, 1023, "%s%02X", responseData, mem[i]);

			haveResponse = 1;
		}
	}
}

void GDBReadRegisters()
{
	// Registers for 'currentGProcess'

	struct STaskContext *ctx = _task_get_context(0); // CPU0
	struct STask *task = &ctx->tasks[currentGProcess];

	mini_snprintf(responseData, 1023, "%s00000000", responseData); // Since we're stashing PC here, we'll just return 0 for zero register instead
	for (int i=1; i<32; ++i)
		mini_snprintf(responseData, 1023, "%s%08X", responseData, task->regs[i]);
	mini_snprintf(responseData, 1023, "%s%08X", responseData, task->regs[0]); // Now we have to add the PC at the end of the register list

	haveResponse = 1;
}

void GDBVCont()
{
	packetData += 6; // Skip 'vCont'

	// Parse commands
	char *command = strtok(packetData, ";");
	while (command != NULL)
	{
		if (strstr(command, "?") == command)
		{
			strcpy(responseData, "vCont;c;C;s;S;");
			haveResponse = 1;
		}
		else if (strstr(command, "c") == command)
		{
			// Continue
			strcpy(responseData, "S05");
			haveResponse = 1;
		}
		else if (strstr(command, "s") == command)
		{
			// Step
			strcpy(responseData, "S05");
			haveResponse = 1;
		}
		else
		{
			kprintf("Unknown vCont: %s\n", command);
		}
		command = strtok(NULL, ";");
	}
}

void GDBBreak()
{
	if (debuggerAttached == 0)
	{
		// Pass this through to the CLI for now
		// Later on we'll let the GDB stub handle this
		uint8_t drain = 0x03;
		KeyRingBufferWrite(&drain, 1);
	}
	else
	{
		// Stopped due to CTRL+C
		strcpy(responseData, "T02");
		haveResponse = 1;
	}
}

void GDBParseCommands()
{
	if (packetData[0] == 0x03) // CTRL+C
	{
		GDBBreak();
	}
	else if (strstr(packetData, "m") == packetData)
	{
		GDBMemRead();
	}
	else if (strstr(packetData, "H") == packetData)
	{
		GDBThreadOp();
	}
	else if (strstr(packetData, "c") == packetData)
	{
		// Continue
		strcpy(responseData, "OK"); // or S05 or other codes if process stops
		haveResponse = 1;
	}
	else if (strstr(packetData, "D") == packetData)
	{
		debuggerAttached = 0;
		// Detach
		strcpy(responseData, "OK");
		haveResponse = 1;
	}
	else if (strstr(packetData, "g") == packetData)
	{
		GDBReadRegisters();
	}
	else if (strstr(packetData, "qC") == packetData)
	{
		strcpy(responseData, "p0.0");
		haveResponse = 1;
	}
	else if (strstr(packetData, "Z0") == packetData)
	{
		// TODO: Insert breakpoint
		strcpy(responseData, "OK");
		haveResponse = 1;
	}
	else if (strstr(packetData, "z0") == packetData)
	{
		// TODO: Remove breakpoint
		strcpy(responseData, "OK");
		haveResponse = 1;
	}
	else if (strstr(packetData, "qSupported") == packetData)
	{
		debuggerAttached = 1;
		GDBQSupported();
	}
	else if (strstr(packetData, "vKill") == packetData)
	{
		debuggerAttached = 0;
		// TODO: Kill attached process
		strcpy(responseData, "OK");
		haveResponse = 1;
	}
	else if (strstr(packetData, "qTStatus") == packetData)
	{
		GDBQTStatus();
	}
	else if (strstr(packetData, "qfThreadInfo") == packetData)
	{
		GDBThreadInfo();
	}
	else if (strstr(packetData, "qAttached") == packetData)
	{
		GDBQAttached();
	}
	else if (strstr(packetData, "qSymbol") == packetData)
	{
		// Empty response
		haveResponse = 1;
	}
	else if (strstr(packetData, "qsThreadInfo") == packetData)
	{
		// Empty response
		haveResponse = 1;
	}
	else if (strstr(packetData, "vMustReplyEmpty"))
	{
		// Empty response
		haveResponse = 1;
	}
	else if (strstr(packetData, "qOffsets") == packetData)
	{
		// Empty response
		haveResponse = 1;
	}
	else if (strstr(packetData, "?") == packetData)
	{
		// S00 if the process is not halted
		strcpy(responseData, "S00");
		haveResponse = 1;
	}
	else if (strstr(packetData, "vCont") == packetData)
	{
		GDBVCont();
		haveResponse = 1;
	}
	else
	{
		kprintf("unknown command: %s\n", packetData);
		strcpy(responseData, "-");
		isackresponse = 1;
		haveResponse = 1;
	}

	if (haveResponse)
	{
		if (isackresponse)
		{
			UARTPrintf("%s", responseData);
			kprintf("(N)ACK: %s\n", responseData);
		}
		else
		{
			uint8_t checksum = GDBChecksum(responseData);
			UARTPrintf("+$%s#%02X", responseData, checksum);
			//kprintf("Response: +$%s#%02X\n", responseData, checksum);
		}
	}
}

void GDBStubBeginPacket()
{
	responseData[0] = 0;
	packetCursor = 0;
	haveResponse = 0;
	isackresponse = 0;
}

void GDBStubEndPacket()
{
	// Null-terminate the packet
	packetData[packetCursor++] = 0;

	// TODO: Parse and respond to packet contents
	GDBParseCommands();
}

void GDBStubAddByte(uint8_t byte)
{
	packetData[packetCursor++] = byte;
}
