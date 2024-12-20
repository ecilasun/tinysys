#include <string.h>

#include "basesystem.h"
#include "uart.h"
#include "serialinput.h"
#include "rombase.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyringbuffer.h"
#include "mini-printf.h"
#include "mini-scanf.h"
#include <stdlib.h>

static char *packetData = (char *)GDB_PACKET_BUFFER;
static char *responseData = (char *)GDB_RESPONSE_BUFFER;
static uint32_t debuggerAttached = 0;
static uint32_t packetCursor = 0;
static uint32_t haveResponse = 0;
static uint32_t isackresponse = 0;
static uint32_t currentGProcess = 0;
static uint32_t currentCProcess = 0;
static uint32_t currentThread = 0;

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
	
	strcat(responseData, "PacketSize=1024;qXfer:threads:read+;");

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
		char *threadStr = strtok(packetData, ";");
		if (threadStr != NULL)
		{
			int procid, tid;
			c_sscanf(threadStr, "p%d.%d", &procid, &tid);

			kprintf("proc %d thread %d\n", procid, tid);
			currentGProcess = tid - 1;

			strcpy(responseData, "OK");
			haveResponse = 1;
		}
	}
	else if (packetData[0] == 'c') // Hc
	{
		packetData++; // Skip 'c'

		char *threadStr = strtok(packetData, ";");
		if (threadStr != NULL)
		{
			int procid, tid;
			c_sscanf(threadStr, "p%d.%d", &procid, &tid);

			kprintf("proc %d thread %d\n", procid, tid);
			currentCProcess = tid - 1;

			strcpy(responseData, "OK");
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

void GDBThreadsRead()
{
	packetData += 19; // Skip 'qXfer:threads:read:'
	packetData++; // Skip ':'

	int offset, length;
	c_sscanf(packetData, "%d,%d", &offset, &length);

	// m: more data l: last packet
	strcpy(responseData, "l<?xml version=\"1.0\"?>\n<threads>\n");

	for (int i=0; i<MAX_HARTS; ++i)
	{
		struct STaskContext *ctx = _task_get_context(i);

		for (int j=0; j<ctx->numTasks; ++j)
		{
			struct STask *task = &ctx->tasks[j];
			mini_snprintf(responseData, 1023, "%s\t<thread id=\"%x\" core=\"%d\" name=\"%s\" handle=\"%04X\"></thread>\n", responseData, j+1, i, task->name, 0xA000+i);
		}
	}

	strcat(responseData, "</threads>\n");

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
		mini_snprintf(responseData, 1023, "%sp%d.%d", responseData, i+1, 1); // process and thread IDs are one-based (and hex unless there's a - sign?)
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
	char *addressStr = strtok(packetData, ",");
	if (addressStr != NULL)
	{
		// Reverse the hex bytes of the address
		size_t len = strlen(addressStr);
		char reversedAddressStr[9] = {0}; // 8 hex digits + null terminator
		for (size_t i = 0; i < len; i += 2)
		{
			reversedAddressStr[len - 2 - i] = addressStr[i];
			reversedAddressStr[len - 1 - i] = addressStr[i + 1];
		}

		uint32_t address = (uint32_t)strtol(reversedAddressStr, NULL, 16);

		packetData += strlen(addressStr) + 1; // Skip address

		// Parse length
		char *lengthStr = strtok(packetData, ",");
		if (lengthStr != NULL)
		{
			uint32_t length = (uint32_t)strtol(lengthStr, NULL, 16);

			// Read memory and format response
			responseData[0] = '\0';
			for (uint32_t i = 0; i < length; ++i)
			{
				uint8_t byte = *((uint8_t *)(address + i));
				mini_snprintf(responseData + strlen(responseData), 1023 - strlen(responseData), "%02x", byte);
			}

			haveResponse = 1;
		}
	}
}

void GDBReadRegisters()
{
	// Registers for 'currentGProcess'

	struct STaskContext *ctx = _task_get_context(0); // CPU0
	struct STask *task = &ctx->tasks[currentGProcess];

	// Since we're stashing PC here, we'll just return 0 for zero register instead
	mini_snprintf(responseData, 1023, "%s00000000", responseData);

	// General purpose registers - float registers are aliased to these in our arhitecture
	for (int i=1; i<32; ++i)
	{
		uint32_t reg = task->regs[i];
		mini_snprintf(responseData + strlen(responseData), 1023 - strlen(responseData), "%02x%02x%02x%02x",
					(reg >> 0) & 0xFF,
					(reg >> 8) & 0xFF,
					(reg >> 16) & 0xFF,
					(reg >> 24) & 0xFF);
	}

	// PC comes last
	uint32_t pc = task->regs[0];
	mini_snprintf(responseData + strlen(responseData), 1023 - strlen(responseData), "%02x%02x%02x%02x",
				(pc >> 0) & 0xFF,
				(pc >> 8) & 0xFF,
				(pc >> 16) & 0xFF,
				(pc >> 24) & 0xFF);

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

void GDBSetThread()
{
	packetData++; // Skip 'T'

	// Parse thread ID
	char *threadStr = strtok(packetData, ";");
	if (threadStr != NULL)
	{
		int procid, tid;
		c_sscanf(threadStr, "p%d.%d", &procid, &tid);

		kprintf("proc %d thread %d\n", procid, tid);
		currentThread = tid - 1;

		strcpy(responseData, "OK");
		haveResponse = 1;
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
	else if (strstr(packetData, "T") == packetData)
	{
		GDBSetThread();
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
		strcpy(responseData, "p1.0");
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
	else if (strstr(packetData, "qXfer:threads:read") == packetData)
	{
		GDBThreadsRead();
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
