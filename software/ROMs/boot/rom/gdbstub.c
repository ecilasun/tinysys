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
static uint32_t currentGCPU = 0;
static uint32_t currentGProcess = 2;
static uint32_t currentCCPU = 0;
static uint32_t currentCProcess = 2;
static uint32_t currentCPU = 0;		// First CPU
static uint32_t currentThread = 2;	// User process

uint32_t GDBHexToUint(char *hex)
{
	uint32_t val = 0;
	while (*hex) {
		uint8_t nibble = *hex++;
		if (nibble >= '0' && nibble <= '9')
			nibble = nibble - '0';
		else if (nibble >= 'a' && nibble <='f')
			nibble = nibble - 'a' + 10;
		else if (nibble >= 'A' && nibble <='F')
			nibble = nibble - 'A' + 10;
		val = (val << 4) | (nibble & 0xF);
	}
	return val;
}

uint32_t GDBDecToUint(char *dec)
{
	uint32_t val = 0;
	while (*dec) {
		uint8_t digit = *dec++;
		if (digit >= '0' && digit <= '9')
			digit = digit - '0';
		val = (val*10) + digit;
	}
	return val;
}

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
			// Can only do sofware breakpoints (i.e. replace instructions with ebreak and save them in a table)
			strcat(responseData, "swbreak+;");
		}
		else if (strstr(command, "hwbreak") == command)
		{
			// Can't do hardware breakpoints
			strcat(responseData, "hwbreak-;");
		}
		else if (strstr(command, "vContSupported") == command)
		{
			// vCont supported
			strcat(responseData, "vContSupported+;");
		}
		else if (strstr(command, "multiprocess") == command)
		{
			// Only one user process with multiple threads at one time
			strcat(responseData, "multiprocess-;");
		}
		else
		{
			// Ignore unknown queries
			//kprintf("? %s\n", command);
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
		int tid;
		c_sscanf(packetData, "p%x", &tid);
		tid = tid <= 0 ? 0 : tid - 1;

		kprintf("g %d\n", tid);
		currentGCPU = 0; // TODO: Find out from thread ID
		currentGProcess = tid;

		strcpy(responseData, "OK");
		haveResponse = 1;
	}
	else if (packetData[0] == 'c') // Hc
	{
		packetData++; // Skip 'c'

		int tid;
		c_sscanf(packetData, "p%x", &tid);
		tid = tid <= 0 ? 0 : tid - 1;

		kprintf("c %d\n", tid);
		currentCCPU = 0; // TODO: Find out from thread ID
		currentCProcess = tid;

		strcpy(responseData, "OK");
		haveResponse = 1;
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
	packetData++; // Skip second ':'

	int offset, length;
	c_sscanf(packetData, "%d,%d", &offset, &length);

	// m: more data, l: last packet
	strcpy(responseData, "l<?xml version=\"1.0\"?>\n<threads>\n");

	// NOTE: The OS thread (CPU#0 thread#0) can't be debugged as it's running the OS and the debugger
	for (int cpu=0; cpu<MAX_HARTS; ++cpu)
	{
		struct STaskContext *ctx = _task_get_context(cpu);

		// Skip the OS threads
		for (int j=cpu==0 ? 2 : 1; j<ctx->numTasks; ++j)
		{
			struct STask *task = &ctx->tasks[j];
			// Apparently GDB expects thread IDs across CPUs to be unique
			mini_snprintf(responseData, 1023, "%s\t<thread id=\"%d\" core=\"%d\" name=\"%s\" handle=\"%x\">%s [CPU%d]</thread>\n", responseData, j+1, cpu, task->name, cpu*TASK_MAX+j, task->name, cpu);
		}
	}

	strcat(responseData, "</threads>\n");

	haveResponse = 1;
}

void GDBThreadInfo()
{
	// GDB expects the first thread in the response to be able to stop.
	// We'll have to deny that for the OS threads.
	strcpy(responseData, "m ");
	for (int cpu=0; cpu<MAX_HARTS; ++cpu)
	{
		struct STaskContext *ctx = _task_get_context(cpu);

		// Skip the OS threads
		for (int j=cpu==0 ? 2 : 1; j<ctx->numTasks; ++j)
		{
			//struct STask *task = &ctx->tasks[i];
			mini_snprintf(responseData, 1023, "%s%d", responseData, j+1);
			strcat(responseData, ",");
		}
	}

	// Delete the last ','
	responseData[strlen(responseData)-1] = 0;

	haveResponse = 1;
}

void GDBQAttached()
{
	packetData += 9; // Skip 'qAttached'
	packetData++; // Skip ':'

	uint32_t processid;
	c_sscanf(packetData, "%x", &processid);
	
	//kprintf("?attached(%d)\n", processid);

	// 1: Attached to existing process, 0: Created new process
	strcpy(responseData, "1");
	haveResponse = 1;
}

void GDBMemRead()
{
	packetData++; // Skip 'm'

	// Parse address
	uint32_t addrs, numbytes;
	c_sscanf(packetData, "%x,%d", &addrs, &numbytes);

	// Read memory and format response
	for (uint32_t i = 0; i < numbytes; ++i)
	{
		uint8_t byte = *((uint8_t *)(addrs + i));
		mini_snprintf(responseData, 1023, "%s%02x", responseData, byte);
	}

	haveResponse = 1;
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
		int tid;
		c_sscanf(threadStr, "p%x", &tid);
		tid = tid <= 0 ? 0 : tid - 1;

		kprintf("T %d\n", tid);
		currentCPU = 0; // TODO: Find out from thread ID
		currentThread = tid;

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
		// TODO: halt the user task

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

		// TODO: Detach (i.e. resume process and restore all breakpoints)

		strcpy(responseData, "OK");
		haveResponse = 1;
	}
	else if (strstr(packetData, "g") == packetData)
	{
		GDBReadRegisters();
	}
	else if (strstr(packetData, "qC") == packetData)
	{
		mini_snprintf(responseData, 1023, "QC %d", currentThread);
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

		// Kill user process and its threads
		struct STaskContext *ctx0 = _task_get_context(0);
		for (int i=2; i<ctx0->numTasks; ++i)
			_task_exit_task_with_id(ctx0, i, 0);
		struct STaskContext *ctx1 = _task_get_context(1);
		for (int i=1; i<ctx0->numTasks; ++i)
			_task_exit_task_with_id(ctx1, i, 0);

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
		// No more threads
		strcpy(responseData, "l");
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
		//kprintf("unknown command: %s\n", packetData);
		strcpy(responseData, "-");
		isackresponse = 1;
		haveResponse = 1;
	}

	if (haveResponse)
	{
		if (isackresponse)
		{
			UARTPrintf("%s", responseData);
			//kprintf("(N)ACK: %s\n", responseData);
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
