#include <stdlib.h>
#include <stdio.h>

#include "gdbstub.h"

static int s_currentCPU = 0;
static int s_currentTask = 0;

// These are from task.h in the SDK folder

#define TASK_MAX 4

enum ETaskState
{
	TS_UNKNOWN,
	TS_PAUSED,
	TS_RUNNING,
	TS_TERMINATING,
	TS_TERMINATED
};

struct STask {
	uint32_t HART;			// HART affinity mask (for migration)
	uint32_t runLength;		// Time slice dedicated to this task
	enum ETaskState state;	// State of this task
	uint32_t exitCode;		// Task termination exit code
	uint32_t regs[32];		// Integer registers - NOTE: register zero here is actually the PC, 128 bytes

	// Debug support - this will probably move somewhere else
	char name[16];			// Name of this task
};

// 672 bytes total for one core (1344 for two cores)
struct STaskContext {
	// 160 x 4 bytes (640)
	struct STask tasks[TASK_MAX];	// List of all the tasks
	// 32 bytes total below
	int32_t currentTask;			// Current task index
	int32_t numTasks;				// Number of tasks
	int32_t interceptUART;			// Lets an application intecept UART input
	int32_t kernelError;			// Current kernel error
	int32_t kernelErrorData[3];		// Data relevant to the crash
	int32_t hartID;					// Id of the HART where this task context runs
};

// ------------------------------------------------------------

void StopEmulator(CEmulator* emulator)
{
	emulator->m_debugStop = 1;
	do {} while (!emulator->m_debugAck);
}

void ResumeEmulator(CEmulator* emulator)
{
	emulator->m_debugStop = 0;
}

uint8_t gdbchecksum(const char *data)
{
	uint8_t sum = 0;
	while (*data)
	{
		sum += (uint8_t)*data++;
	}
	return sum;
}

void gdbplainpacket(socket_t gdbsocket, const char* buffer)
{
	char response[1024];
	snprintf(response, 1024, "$%s#%02x", buffer, gdbchecksum(buffer));
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbresponsepacket(socket_t gdbsocket, const char* buffer)
{
	char response[1024];
	snprintf(response, 1024, "+$%s#%02x", buffer, gdbchecksum(buffer));
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbresponseack(socket_t gdbsocket)
{
	char response[4];
	snprintf(response, 4, "+");
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbresponsenack(socket_t gdbsocket)
{
	char response[4];
	snprintf(response, 4, "-");
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbreadthreads(socket_t gdbsocket, CEmulator* emulator, const char* buffer)
{
	char response[1024];
	snprintf(response, 1024, "l<?xml version=\"1.0\"?>\n<threads>\n");

	StopEmulator(emulator);

	for (int cpu = 0; cpu < 2; ++cpu)
	{
		CRV32 *core = emulator->m_cpu[cpu];

		// Access the task context of the CPU
		struct STaskContext* contextpool = (struct STaskContext *)emulator->m_bus->GetHostAddress(DEVICE_MAIL);
		struct STaskContext& ctx = contextpool[cpu];

		for (int j = 0; j < ctx.numTasks; ++j)
		{
			struct STask* task = &ctx.tasks[j];
			// Apparently GDB expects thread IDs across CPUs to be unique
			snprintf(response, 1024, "%s\t<thread id=\"%d\" core=\"%d\" name=\"%s:%d\" handle=\"%x\"> </thread>\n", response, (cpu*TASK_MAX+j)+1, cpu, task->name, cpu, cpu*TASK_MAX+j);
		}
	}

	ResumeEmulator(emulator);

	strcat(response, "</threads>\n");
	gdbresponsepacket(gdbsocket, response);
}

void gdbprocessquery(socket_t gdbsocket, CEmulator* emulator, const char* buffer)
{
	// Check the query type
	if (strstr(buffer, "qAttached") == buffer)
	{
		// Attached query
		gdbresponsepacket(gdbsocket, "1");
	}
	else if (strstr(buffer, "qOffsets") == buffer)
	{
		gdbresponsepacket(gdbsocket, "Text=0;Data=0;Bss=0");
	}
	else if (strstr(buffer, "qXfer:threads:read:") == buffer)
	{
		// Read threads query
		gdbreadthreads(gdbsocket, emulator, buffer);
	}
	else if (strstr(buffer, "qTStatus") == buffer)
	{
		// Status query
		gdbresponsepacket(gdbsocket, "");
	}
	else if (strstr(buffer, "qSupported") == buffer)
	{
		// Supported query (packetsize is hex therefore 0x1000==4096 bytes)
		gdbresponsepacket(gdbsocket, "PacketSize=1000;qXfer:threads:read+;swbreak+;");
	}
	else if (strstr(buffer, "qSymbol") == buffer)
	{
		// No symbol query
		gdbresponsepacket(gdbsocket, "");
	}
	else if (strstr(buffer, "qfThreadInfo") == buffer)
	{
		// Thread info query
		fprintf(stderr, "Thread info\n");
	}
	else if (strstr(buffer, "qC") == buffer)
	{
		// Current thread query
		gdbresponsepacket(gdbsocket, "QC 1");
	}
	else
	{
		// Unknown query
		fprintf(stderr, "Unknown query: %s\n", buffer);
	}
}

void gdbvcont(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'vCont'
	buffer += 5;

	// Parse commands
	char* command = strtok(buffer, ";");
	while (command != NULL)
	{
		if (strstr(command, "?") == command)
		{
			// vCont query
			gdbresponsepacket(gdbsocket, "vCont;c;C;s;S;");
		}
		else if (strstr(command, "c") == command)
		{
			// Continue
			StopEmulator(emulator);
			emulator->Continue(s_currentCPU);
			ResumeEmulator(emulator);
			gdbresponseack(gdbsocket);
		}
		else if (strstr(command, "s") == command)
		{
			// Step
			//emulator->StepFromPC(s_currentCPU);
			//gdbresponsepacket(gdbsocket, "S05");
			gdbresponseack(gdbsocket);
		}
		else
		{
			// Unknown command
			fprintf(stderr, "Unknown vCont: %s\n", command);
		}
		command = strtok(NULL, ";");
	}
}

void gdbkillprocess(socket_t gdbsocket, uint32_t hartid, int proc, CEmulator* emulator, char* buffer)
{
	StopEmulator(emulator);

	emulator->RemoveAllBreakpoints(0);

	// Kill processes by removing them from the thread pool
	for (int cpu = 0; cpu < 2; ++cpu)
	{
		if (cpu == hartid || hartid == -1)
		{
			CRV32 *core = emulator->m_cpu[cpu];

			// Access the task context of the CPU
			struct STaskContext* contextpool = (struct STaskContext *)emulator->m_bus->GetHostAddress(DEVICE_MAIL);
			struct STaskContext& ctx = contextpool[cpu];

			// Set task state to terminating
			for (int t = 0; t < ctx.numTasks; ++t)
			{
				struct STask* task = &ctx.tasks[t];

				if (t == proc || proc == -1)
				{
					task->state = TS_TERMINATING;
					task->exitCode = 0;
					//ctx->kernelError = ?; // TODO: set kernel error to 'process terminated by debugger', need to handle it in ROM code
				}
			}
		}
	}

	ResumeEmulator(emulator);
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbreadregisters(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	char response[1024];

	// x0
	snprintf(response, 1024, "00000000");

	StopEmulator(emulator);

	// x1-x31
	CRV32* core = emulator->m_cpu[0];
	for (int i = 1; i < 32; ++i)
	{
		uint32_t reg = core->m_GPR[i];
		snprintf(response, 1024, "%s%02X%02X%02X%02X",
			response,
			(reg >> 0) & 0xFF,
			(reg >> 8) & 0xFF,
			(reg >> 16) & 0xFF,
			(reg >> 24) & 0xFF);
	}

	// PC
	{
		uint32_t reg = core->m_PC;
		snprintf(response, 1024, "%s%02X%02X%02X%02X",
			response,
			(reg >> 0) & 0xFF,
			(reg >> 8) & 0xFF,
			(reg >> 16) & 0xFF,
			(reg >> 24) & 0xFF);
	}

	ResumeEmulator(emulator);

	//fprintf(stderr, "R:%s\n", response);
	gdbresponsepacket(gdbsocket, response);
}

void gdbbinarypacket(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'X'
	buffer++;

	// Parse the address and length
	uint32_t addrs;
	uint32_t len;
	uint32_t readoffset = sscanf(buffer, "%x,%x", &addrs, &len);

	// This is a support check if len is 0
	if (len == 0)
	{
		// We support binary data
		gdbresponsepacket(gdbsocket, "OK");
	}
	else
	{
		// Skip the address and length
		buffer = strchr(buffer, ':');
		buffer++;

		StopEmulator(emulator);

		// Decode the encoded binary data, paying attention to escape sequences and repeat counts
		uint8_t* data = new uint8_t[len];
		uint32_t i = 0;
		uint8_t lastchar = 0;
		while (i < len)
		{
			if (*buffer == '}') // Escape sequence
			{
				buffer++;
				uint8_t original = *buffer ^ 0x20;
				data[i++] = original;
			}
			else if (*buffer == '*') // Repeat last character as a sequence
			{
				buffer++;
				uint8_t count = *buffer - 29;
				for (uint8_t j = 0; j < count; ++j)
					data[i++] = lastchar;
			}
			else // Normal character
				data[i++] = *buffer;
			lastchar = *buffer;
			buffer++;
		}

		// Invalidate data and instruction caches of both CPUs and write incoming data to memory
		emulator->m_cpu[0]->m_dcache.Flush(emulator->m_bus);
		emulator->m_cpu[0]->m_dcache.Discard();
		emulator->m_cpu[0]->m_icache.Discard();
		emulator->m_cpu[1]->m_dcache.Flush(emulator->m_bus);
		emulator->m_cpu[1]->m_dcache.Discard();
		emulator->m_cpu[1]->m_icache.Discard();

		// Write the binary data to memory as 4 byte words
		for (uint32_t i = 0; i < len; i += 4)
		{
			uint32_t word = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24);
			emulator->m_bus->Write(addrs + i, word, 0xF);
		}

		ResumeEmulator(emulator);

		delete[] data;

		// Respond with an ACK on successful memory write
		gdbresponsepacket(gdbsocket, "OK");
	}
}

void gdbsetreg(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'P'
	buffer++;

	uint32_t reg, val;
	sscanf(buffer, "%x=%x", &reg, &val);

	// Reverse the byte order
	val = (val >> 24) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | (val << 24);

	// TODO: when setting PC, our entire task system breaks down
	// Therefore we need to add a dummy task to the task list and set its PC to the supplied value

	StopEmulator(emulator);

	if (reg == 32) // PC is a special case and is always at lastgpr+1
		emulator->m_cpu[0]->m_PC = val;
	else
		emulator->m_cpu[0]->m_GPR[reg] = val;

	ResumeEmulator(emulator);

	gdbresponsepacket(gdbsocket, "OK");
}

void gdbsetcurrentthread(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Parse the thread id
	int threadid;
	sscanf(buffer, "%d", &threadid);

	threadid = threadid-1;

	s_currentCPU = threadid/TASK_MAX;
	s_currentTask = threadid%TASK_MAX;

	fprintf(stderr, "Set current: CPU=%d Task=%d\n", s_currentCPU, s_currentTask);

	gdbresponsepacket(gdbsocket, "OK");
}

void gdbreadmemory(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'm'
	buffer++;

	// Parse the address and length
	uint32_t addrs;
	uint32_t len;
	uint32_t readoffset = sscanf(buffer, "%x,%x", &addrs, &len);

	StopEmulator(emulator);

	// Read the memory
	uint32_t maxlen = (len+1) * 2;
	char* response = new char[maxlen];
	response[0] = 0;
	for (uint32_t i = 0; i < len; i++)
	{
		uint32_t dataword;
		emulator->m_bus->Read(addrs & 0xFFFFFFF0, dataword);

		uint32_t range1 = SelectBitRange(addrs, 1, 1);
		uint32_t range2 = SelectBitRange(addrs, 1, 0);

		uint32_t b[4];
		b[3] = SelectBitRange(dataword, 31, 24);
		b[2] = SelectBitRange(dataword, 23, 16);
		b[1] = SelectBitRange(dataword, 15, 8);
		b[0] = SelectBitRange(dataword, 7, 0);

		uint8_t byte = b[range2];

		snprintf(response, maxlen, "%s%02X", response, byte);
		addrs++;
	}

	ResumeEmulator(emulator);

	gdbresponsepacket(gdbsocket, response);
	delete[] response;
}

void gdbwritememory(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'M'
	buffer++;

	// Parse the address and length
	uint32_t addrs;
	uint32_t len;
	uint32_t readoffset = sscanf(buffer, "%x,%x:", &addrs, &len);

	// Skip the address and length
	buffer = strchr(buffer, ':');
	buffer++;

	StopEmulator(emulator);

	// Decode the encoded binary data, paying attention to escape sequences and repeat counts
	uint8_t* data = new uint8_t[len];
	uint32_t i = 0;
	uint8_t lastchar = 0;
	while (i < len)
	{
		if (*buffer == '}') // Escape sequence
		{
			buffer++;
			uint8_t original = *buffer ^ 0x20;
			data[i++] = original;
		}
		else if (*buffer == '*') // Repeat last character as a sequence
		{
			buffer++;
			uint8_t count = *buffer - 29;
			for (uint8_t j = 0; j < count; ++j)
				data[i++] = lastchar;
		}
		else // Normal character
			data[i++] = *buffer;
		lastchar = *buffer;
		buffer++;
	}

	for (uint32_t i = 0; i < len; i++)
	{
		uint32_t byte = data[i];
		uint32_t ah = SelectBitRange(addrs, 1, 1);
		uint32_t ab = SelectBitRange(addrs, 0, 0);
		uint32_t himask = (ah << 3) | (ah << 2) | ((1 - ah) << 1) | (1 - ah);
		uint32_t lomask = ((ab << 3) | ((1 - ab) << 2) | (ab << 1) | (1 - ab));
		uint32_t wstrobe = himask & lomask;
		uint32_t word = byte | (byte << 8) | (byte << 16) | (byte << 24);

		emulator->m_bus->Write(addrs & 0xFFFFFFF0, word, wstrobe);

		addrs++;
	}

	ResumeEmulator(emulator);

	delete[] data;

	// Respond with an ACK on successful memory write
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbstep(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	StopEmulator(emulator);

	// Step to next instruction
	//emulator->m_cpu[0]->SingleStep();

	ResumeEmulator(emulator);

	// Respond with an ACK on successful step
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbaddbreakpoint(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'Z'
	buffer++;

	// Parse the type, address and length
	uint32_t type, addrs, len;
	uint32_t readoffset = sscanf(buffer, "%d,%x,%x", &type, &addrs, &len);
	
	StopEmulator(emulator);

	// Add the breakpoint
	for (int i = 0; i < 2; ++i)
	{
		if (i == s_currentCPU || s_currentCPU == -1)
			emulator->AddBreakpoint(s_currentCPU, addrs);
	}

	ResumeEmulator(emulator);

	// Respond with an ACK on successful breakpoint addition
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbremovebreakpoint(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'z'
	buffer++;

	// Parse the type, address and length
	uint32_t type, addrs, len;
	uint32_t readoffset = sscanf(buffer, "%d,%x,%x", &type, &addrs, &len);

	StopEmulator(emulator);

	for (int i = 0; i < 2; ++i)
	{
		if (i == s_currentCPU || s_currentCPU == -1)
			emulator->RemoveBreakpoint(s_currentCPU, addrs);
	}

	ResumeEmulator(emulator);

	// Respond with an ACK on successful breakpoint removal
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbstopemulator(CEmulator* emulator)
{
	StopEmulator(emulator);

	for (int i = 0; i < 2; ++i)
	{
		if (i == s_currentCPU || s_currentCPU == -1)
			emulator->AddBreakpoint(s_currentCPU, emulator->m_cpu[i]->m_PC);
	}

	ResumeEmulator(emulator);
}

void gdbsendstopreason(socket_t gdbsocket, int cpu, uint32_t stopaddres, CEmulator* emulator)
{
	CRV32 *core = emulator->m_cpu[cpu];
	struct STaskContext* contextpool = (struct STaskContext *)emulator->m_bus->GetHostAddress(DEVICE_MAIL);
	struct STaskContext& ctx = contextpool[cpu];

	char response[128];
	snprintf(response, 128, "T05;thread:%d;stopped;reason:breakpoint;pc:0x%08X;", cpu*TASK_MAX+ctx.currentTask+1, stopaddres);
	gdbplainpacket(gdbsocket, response);
}

void gdbprocesscommand(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Check the command type
	switch (buffer[0])
	{
		case 0x03:
			// Ctrl-C packet
			gdbstopemulator(emulator);
			break;
		case '?':
			gdbresponsepacket(gdbsocket, "S00");
			break;
		case 'X':
			// Read binary data
			gdbbinarypacket(gdbsocket, emulator, buffer);
			break;
		case 'P':
			// Write single register
			gdbsetreg(gdbsocket, emulator, buffer);
			break;
		case 'D':
			// Disconnect request
			StopEmulator(emulator);
			for (int i = 0; i < 2; ++i)
			{
				if (i == s_currentCPU || s_currentCPU == -1)
					emulator->RemoveAllBreakpoints(i);
			}
			ResumeEmulator(emulator);
			gdbresponsepacket(gdbsocket, "OK");
			fprintf(stderr, "Detached from debugger\n");
			break;
		case 'T':
			// Skip 'T'
			buffer++;
			gdbsetcurrentthread(gdbsocket, emulator, buffer);
			break;
		case 'H':
			// Set thread - Hc or Hg
			if (buffer[1] == 'c')
			{
				//emulator->m_cpu[???]->SetCurrent('c');
				gdbresponsepacket(gdbsocket, "OK");
			}
			else if (buffer[1] == 'g')
			{
				// Skip 'Hg'
				buffer+=2;
				gdbsetcurrentthread(gdbsocket, emulator, buffer);
			}
			break;
		case 'g':
			gdbreadregisters(gdbsocket, emulator, buffer);
			break;
		case 'G':
			// Read registers
			break;
		case 'm':
			// Read memory
			gdbreadmemory(gdbsocket, emulator, buffer);
			break;
		case 'M':
			// Write memory
			gdbwritememory(gdbsocket, emulator, buffer);
			break;
		case 'c':
			// Continue
			break;
		case 's':
			// Step
			gdbstep(gdbsocket, emulator, buffer);
			break;
		case 'Z':
			// Insert breakpoint
			gdbaddbreakpoint(gdbsocket, emulator, buffer);
			break;
		case 'z':
			// Remove breakpoint
			gdbremovebreakpoint(gdbsocket, emulator, buffer);
			break;
		case 'v':
			// vCont
			if (strstr(buffer, "vCont") == buffer)
				gdbvcont(gdbsocket, emulator, buffer);
			else if (strstr(buffer, "vMustReplyEmpty") == buffer)
				gdbresponsepacket(gdbsocket, "");
			else if (strstr(buffer, "vKill") == buffer)
				gdbkillprocess(gdbsocket, s_currentCPU, s_currentTask, emulator, buffer);
			else
				fprintf(stderr, "Unknown v command: %s\n", buffer);
			break;
		case 'q':
			// Query
			gdbprocessquery(gdbsocket, emulator, buffer);
			break;
		default:
			// Unknown command
			fprintf(stderr, "Unknown sequence start: %s\n", buffer);
			break;
	}
}
