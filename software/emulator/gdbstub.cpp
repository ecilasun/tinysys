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
	int32_t kernelError;			// Current kernel error
	int32_t kernelErrorData[3];		// Data relevant to the crash
	int32_t hartID;					// Id of the HART where this task context runs
};

// ------------------------------------------------------------

void StopEmulatorThread(CEmulator* emulator)
{
	emulator->m_debugStop = 1;
	do {} while (!emulator->m_debugAck);
}

void ResumeEmulatorThread(CEmulator* emulator)
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
	char response[16384];
	snprintf(response, 16384, "$%s#%02x", buffer, gdbchecksum(buffer));
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif

#ifdef GDB_COMM_DEBUG
	fprintf(stderr, "< %s\n", response);
#endif
}

void gdbresponsepacket(socket_t gdbsocket, const char* buffer)
{
	char response[16384];
	snprintf(response, 16384, "+$%s#%02x", buffer, gdbchecksum(buffer));
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
#ifdef GDB_COMM_DEBUG
	fprintf(stderr, "< %s\n", response);
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
#ifdef GDB_COMM_DEBUG
	fprintf(stderr, "< %s\n", response);
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
#ifdef GDB_COMM_DEBUG
	fprintf(stderr, "< %s\n", response);
#endif
}

void gdbreadthreads(socket_t gdbsocket, CEmulator* emulator, const char* buffer)
{
	char response[16384];
	snprintf(response, 16384, "l<?xml version=\"1.0\"?>\n<threads>\n");

	StopEmulatorThread(emulator);

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

	ResumeEmulatorThread(emulator);

	strcat(response, "</threads>\n");
	gdbresponsepacket(gdbsocket, response);
}

void gdbprocessquery(socket_t gdbsocket, CEmulator* emulator, const char* buffer)
{
	// Check the query type
	if (strstr(buffer, "qAttached") == buffer)
	{
		// Attached query : 0-new process, 1-attached
		int pid = 0;
		const char* pidstr = strchr(buffer, ':');
		if (pidstr)
			sscanf(buffer, ":%x", &pid);
		fprintf(stderr, "Attached pid: %d\n", pid);
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
		gdbresponsepacket(gdbsocket, "PacketSize=1000;qXfer:threads:read+;swbreak+;"); // QNonStop+
	}
	else if (strstr(buffer, "qSymbol") == buffer)
	{
		// No symbol query support
		gdbresponsepacket(gdbsocket, "");
	}
	else if (strstr(buffer, "qfThreadInfo") == buffer)
	{
		// Thread info query
		fprintf(stderr, "Thread info\n");
	}
	else if (strstr(buffer, "qC") == buffer)
	{
		// we don't support this
		gdbresponsepacket(gdbsocket, "");
	}
	/*else if (strstr(buffer, "QNonStop") == buffer)
	{
		// Enter/Exit non-stop mode
		gdbresponsepacket(gdbsocket, "OK");
	}*/
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

	//> $vCont;s:1;c#c1

	// Parse commands
	char* command = strtok(buffer, ";");
	int respond = 0;
	while (command != NULL)
	{
		if (strstr(command, "?") == command)
		{
			// vCont query
			gdbresponsepacket(gdbsocket, "vCont;s;S;c;C;");
		}
		else if (strstr(command, "c") == command)
		{
			StopEmulatorThread(emulator);

			// Continue on current or all CPUs
			for (int i = 0; i < 2; ++i)
			{
				if (i == s_currentCPU || s_currentCPU == -1)
					emulator->Continue(s_currentCPU);
			}

			ResumeEmulatorThread(emulator);
			respond = 1;
		}
		else if (strstr(command, "s") == command)
		{
			StopEmulatorThread(emulator);

			// Step on current or all CPUs
			for (int i = 0; i < 2; ++i)
			{
				if (i == s_currentCPU || s_currentCPU == -1)
					emulator->StepToNext(s_currentCPU);
			}

			ResumeEmulatorThread(emulator);
			respond = 1;
		}
		else
		{
			// Unknown command
#if defined(GDB_COMM_DEBUG)
			fprintf(stderr, "Unknown vCont: %s\n", command);
#endif
		}
		command = strtok(NULL, ";");
	}

	if (respond)
		gdbresponseack(gdbsocket);
}

void gdbkillprocess(socket_t gdbsocket, uint32_t hartid, int proc, CEmulator* emulator, char* buffer)
{
	StopEmulatorThread(emulator);

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

	ResumeEmulatorThread(emulator);
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbreadregisters(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	char response[1024];

	// x0
	snprintf(response, 1024, "00000000");

	StopEmulatorThread(emulator);

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
		uint32_t reg = core->m_execPC;
		snprintf(response, 1024, "%s%02X%02X%02X%02X",
			response,
			(reg >> 0) & 0xFF,
			(reg >> 8) & 0xFF,
			(reg >> 16) & 0xFF,
			(reg >> 24) & 0xFF);
	}

	ResumeEmulatorThread(emulator);

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

		StopEmulatorThread(emulator);

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

		ResumeEmulatorThread(emulator);

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

	StopEmulatorThread(emulator);

	if (reg == 32) // PC is a special case and is always at lastgpr+1
		emulator->m_cpu[0]->m_PC = val; /// Hmmm...
	else
		emulator->m_cpu[0]->m_GPR[reg] = val;

	ResumeEmulatorThread(emulator);

#if defined(GDB_COMM_DEBUG)
	fprintf(stderr, "Setting reg %d to %08X\n", reg, val);
#endif

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

#if defined(GDB_COMM_DEBUG)
	fprintf(stderr, "READ @0x%08X %d\n", addrs, len);
#endif

	StopEmulatorThread(emulator);

	// Read the memory
	uint32_t maxlen = (len+1) * 2;
	char* response = new char[maxlen];
	response[0] = 0;

	int wordcount = len>>2;
	int leftover = len%4;

	// Whole words
	for (int i = 0; i < wordcount; i++)
	{
		uint32_t dataword;
		emulator->m_bus->Read(addrs, dataword);

		uint32_t b[4];
		b[3] = SelectBitRange(dataword, 31, 24);
		b[2] = SelectBitRange(dataword, 23, 16);
		b[1] = SelectBitRange(dataword, 15, 8);
		b[0] = SelectBitRange(dataword, 7, 0);

		snprintf(response, maxlen, "%s%02X", response, b[0]);
		snprintf(response, maxlen, "%s%02X", response, b[1]);
		snprintf(response, maxlen, "%s%02X", response, b[2]);
		snprintf(response, maxlen, "%s%02X", response, b[3]);

		addrs += 4;
	}

	// Leftover bytes
	if (leftover > 0)
	{
		uint32_t dataword;
		emulator->m_bus->Read(addrs, dataword);

		uint32_t b[4];
		b[3] = SelectBitRange(dataword, 31, 24);
		b[2] = SelectBitRange(dataword, 23, 16);
		b[1] = SelectBitRange(dataword, 15, 8);
		b[0] = SelectBitRange(dataword, 7, 0);

		for (int i = 0; i < leftover; i++)
			snprintf(response, maxlen, "%s%02X", response, b[i]);
	}

	ResumeEmulatorThread(emulator);

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

#if defined(GDB_COMM_DEBUG)
	fprintf(stderr, "WRITE @0x%08X %d\n", addrs, len);
#endif

	StopEmulatorThread(emulator);

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

	int wordcount = len>>2;
	int leftover = len%4;

	// Whole words
	uint32_t* worddata = (uint32_t*)data;
	for (int i = 0; i < wordcount; i++)
	{
		uint32_t word = worddata[i];
		emulator->m_bus->Write(addrs & 0xFFFFFFF0, word, 0xF);
		addrs+=4;
	}

	// Leftover bytes
	if (leftover > 0)
	{
		uint32_t word = 0;
		for (int i = 0; i < leftover; i++)
			word |= (data[wordcount*4+i] << (i*8));
		emulator->m_bus->Write(addrs & 0xFFFFFFF0, word, 0xF);
	}

	ResumeEmulatorThread(emulator);

	delete[] data;

	// Respond with an ACK on successful memory write
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbstep(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	StopEmulatorThread(emulator);

	// Step to next instruction
	//emulator->m_cpu[0]->SingleStep();

	ResumeEmulatorThread(emulator);

	// Respond with an ACK on successful step
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbaddbreakpoint(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'Z'
	buffer++;

	// Parse the type, address and length
	// 0; software breakpoint
	// 1; hardware breakpoint
	// 2; write watchpoint
	// 3; read watchpoint
	// 4; access watchpoint
	uint32_t type, addrs, len;
	uint32_t readoffset = sscanf(buffer, "%d,%x,%x", &type, &addrs, &len);

	// We can't do other breakpoint types than software
	if (type != 0)
	{
		gdbresponsenack(gdbsocket);
		return;
	}

	// NOTE: We're ignoring the length for now since every instruction is 4 bytes long
	
	StopEmulatorThread(emulator);

	// Add the breakpoint
	for (int i = 0; i < 2; ++i)
	{
		if (i == s_currentCPU || s_currentCPU == -1)
			emulator->AddBreakpoint(0, s_currentCPU, addrs); // non-volatile breakpoint
	}

	ResumeEmulatorThread(emulator);

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

	StopEmulatorThread(emulator);

	for (int i = 0; i < 2; ++i)
	{
		if (i == s_currentCPU || s_currentCPU == -1)
			emulator->RemoveBreakpoint(s_currentCPU, addrs);
	}

	ResumeEmulatorThread(emulator);

	// Respond with an ACK on successful breakpoint removal
	gdbresponsepacket(gdbsocket, "OK");
}

void gdbstopemulator(CEmulator* emulator)
{
	StopEmulatorThread(emulator);

	for (int i = 0; i < 2; ++i)
	{
		if (i == s_currentCPU || s_currentCPU == -1)
			emulator->AddBreakpoint(1, s_currentCPU, emulator->m_cpu[i]->m_execPC); // volatile breakpoint
	}

	ResumeEmulatorThread(emulator);
}

void gdbsendstopreason(socket_t gdbsocket, int cpu, uint32_t stopaddres, CEmulator* emulator)
{
	StopEmulatorThread(emulator);

	CRV32 *core = emulator->m_cpu[cpu];
	struct STaskContext* contextpool = (struct STaskContext *)emulator->m_bus->GetHostAddress(DEVICE_MAIL);
	struct STaskContext& ctx = contextpool[cpu];

	char response[128];
	snprintf(response, 128, "T05;thread:%d;stopped;reason:breakpoint;pc:0x%08X;", cpu*TASK_MAX+ctx.currentTask+1, stopaddres);
	gdbplainpacket(gdbsocket, response);

	ResumeEmulatorThread(emulator);
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
			gdbresponsepacket(gdbsocket, "T00");
			//gdbresponseack(gdbsocket); // No stop reason at this time
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
			StopEmulatorThread(emulator);
			for (int i = 0; i < 2; ++i)
				emulator->RemoveAllBreakpoints(i);
			ResumeEmulatorThread(emulator);
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
		case 'Q':
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
