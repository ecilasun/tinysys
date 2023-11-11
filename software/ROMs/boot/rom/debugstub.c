#include <string.h>

#include "basesystem.h"
#include "usbserial.h"
#include "debugstub.h"
#include "rombase.h"
#include "serialinringbuffer.h"

// https://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html

static const char s_hexdigits[] = "0123456789ABCDEF";
static uint32_t s_debuggerConnected = 0;
static uint32_t s_packetStart = 0;
static uint32_t s_checksumStart = 0;
static uint32_t s_packetComplete = 0;
static uint32_t s_gatherBinary = 0;
static uint32_t s_isBinaryHeader = 0;
static uint32_t s_idx = 0;
static uint32_t s_currentThread = 3; // SYSIDLE
static uint32_t s_binaryAddrs;
static uint32_t s_binaryNumbytes;
static uint32_t s_receivedBytes;
static char s_chk[2];
static char s_packet[256];

uint32_t startswith(const char *_source, const char *_token)
{
	if (strstr(_source, _token) == _source)
		return 1;
	return 0;
}

void uint2dec(const uint32_t val, char *msg)
{
    const char digits[] = "0123456789";

    int d = 1000000000;
    uint32_t enableappend = 0;
    uint32_t m = 0;
    /*if (i<0)
    msg[m++] = '-';*/
    for (int c=0;c<10;++c)
    {
        //uint32_t r = abs(val/d)%10;
        uint32_t r = (val/d)%10;
        // Ignore preceeding zeros
        if ((r!=0) || (enableappend) || (d==1))
        {
            enableappend = 1; // Rest of the digits can be appended
            msg[m++] = digits[r];
        }
        d = d/10;
    }
    msg[m] = 0;
}

uint32_t dec2uint(char *dec)
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

uint32_t hex2uint(char *hex)
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

void int2architectureorderedstring(const uint32_t val, char *regstring)
{
	regstring[6] = s_hexdigits[((val>>28)%16)];
	regstring[7] = s_hexdigits[((val>>24)%16)];
	regstring[4] = s_hexdigits[((val>>20)%16)];
	regstring[5] = s_hexdigits[((val>>16)%16)];
	regstring[2] = s_hexdigits[((val>>12)%16)];
	regstring[3] = s_hexdigits[((val>>8)%16)];
	regstring[0] = s_hexdigits[((val>>4)%16)];
	regstring[1] = s_hexdigits[(val%16)];
	regstring[8] = 0;
}

void byte2architectureorderedstring(const uint8_t val, char *regstring)
{
	regstring[0] = s_hexdigits[((val>>4)%16)];
	regstring[1] = s_hexdigits[(val%16)];
	regstring[2] = 0;
}

void StrChecksum(const char *str, char *checksumstr)
{
	int checksum = 0;
	int i=0;
	while(str[i]!=0)
	{
		checksum += str[i];
		++i;
	}
	checksum = checksum%256;

	checksumstr[0] = s_hexdigits[((checksum>>4)%16)];
	checksumstr[1] = s_hexdigits[(checksum%16)];
	checksumstr[2] = 0;
}

void SendAck()
{
	USBSerialWrite("+");
}

void SendNack()
{
	USBSerialWrite("-");
}

void SendDebugPacket(const char *packetString)
{
	char checksumstr[3];
	StrChecksum(packetString, checksumstr);

	USBSerialWrite("$");
	USBSerialWrite(packetString);
	USBSerialWrite("#");
	USBSerialWrite(checksumstr);
}

void HandlePacket()
{
	char outstring[512];
	struct STaskContext *taskctx = GetTaskContext();

	if (startswith(s_packet, "qXfer:threads:read::")) // qXfer:threads:read::{start,offset}, normally would send l<?xml version=\"1.0\"?>\n<threads> etc
	{
		char offsetbuf[12];
		int a=0, p=20;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			offsetbuf[a++] = s_packet[p++];
		offsetbuf[a]=0;
		uint32_t offset = dec2uint(offsetbuf);

		SendAck();

		if (offset == 0)
		{
			strcpy(outstring, "l<?xml version=\"1.0\"?>\n<threads>");
			for (uint32_t i=0; i<taskctx->numTasks; ++i)
			{
				char threadid[10];
				uint2dec(i+1, threadid);
				strcat(outstring, "<thread id=\"");
				strcat(outstring, threadid);
				strcat(outstring, "\" core=\"0\" name=\"");
				strcat(outstring, taskctx->tasks[i].name);
				strcat(outstring, "\"></thread>\n");
			}
			strcat(outstring, "</threads>\n");

			SendDebugPacket(outstring);
		}
		else
			SendDebugPacket(""); // ???
	}
	else if (startswith(s_packet, "vMustReplyEmpty"))
	{
		SendAck(); // Does 'empty' include an ack?
		SendDebugPacket("");
	}
	/*else if (startswith(s_packet, "QNonStop:"))
	{
		SendAck();
		if (s_packet[9] == '0') // all-stop mode
		{
			uint32_t PC = TaskGetPC(taskctx, threadid);
			TaskInsertBreakpoint(taskctx, threadid, PC);
			SendDebugPacket("OK");
		}
		else if (s_packet[9] == '1') // non-stop mode
		{
			// In this mode we can push async events that occur on our end
			SendDebugPacket("OK");
		}
		else
			SendDebugPacket("");
	}*/
	else if (startswith(s_packet, "qSupported"))
	{
		s_debuggerConnected = 1;
		// TODO: hwbreak+

		SendAck();
		SendDebugPacket("qSupported:swbreak+;multiprocess-;qXfer:threads:read+;qRelocInsn-;exec-events+;vRun+;vStopped+;vContSupported+;PacketSize=255");
	}
	else if (startswith(s_packet, "qOffsets")) // segment offsets (grab from ELF header)
	{
		SendAck();
		SendDebugPacket("Text=0;Data=0;Bss=0;"); // No relocation
	}
	else if (startswith(s_packet, "vRun"))
	{
		// vRun;filename;argument
		// TODO:
		SendNack();
	}
	else if (startswith(s_packet, "vStopped"))
	{
		// TODO: stop reason
		SendAck();
		SendDebugPacket("T05thread:3;");
	}
	else if (startswith(s_packet, "qfThreadInfo"))
	{
		// List of active thread IDs
		// GDB will stop the first thread listed here,
		// so it has to be the user thread if available
		strcpy(outstring, "m ");
		for (uint32_t i=0; i<taskctx->numTasks; ++i)
		{
			char threadid[10];
			uint2dec(i, threadid);
			strcat(outstring, threadid);
			if (i!=taskctx->numTasks-1)
				strcat(outstring, ",");
		}

		SendAck();
		SendDebugPacket(outstring);
	}
	else if (startswith(s_packet, "qsThreadInfo"))
	{
		SendAck();
		SendDebugPacket("l");
	}
	else if (startswith(s_packet, "qTStatus"))
	{
		SendAck();
		SendDebugPacket(""); // No trace state
	}
	else if (startswith(s_packet, "vCtrlC"))
	{
		uint32_t threadid = s_currentThread - 1;
		// Signal pause
		// taskctx->currentTask == 2 -> only user thread can be stopped
		if (threadid == 2)
		{
			uint32_t PC = TaskGetPC(taskctx, threadid);
			TaskInsertBreakpoint(taskctx, threadid, PC);

			SendAck();
			SendDebugPacket("T05thread:3;");
		}
		else
			SendNack();
	}
	else if (startswith(s_packet, "qAttached"))
	{
		SendAck();
		if (taskctx->numTasks == 3)
			SendDebugPacket("1"); // Attached to existing process
		else
			SendDebugPacket("0"); // Created new process
	}
	else if (startswith(s_packet, "vCont?"))
	{
		// We support continue/step/stop/start actions (not the 'sig' ones)
		SendAck();
		SendDebugPacket("vCont;c;C");
	}
	else if (startswith(s_packet, "vCont")) // Continue/step/stop/start
	{
		uint32_t threadid = s_currentThread - 1;

		if (s_packet[6] == 'c') // continue
		{
			uint32_t PC = TaskGetPC(taskctx, threadid);
			TaskRemoveBreakpoint(taskctx, threadid, PC);
		}
		if (s_packet[6] == 's') // step
		{
			uint32_t PC = TaskGetPC(taskctx, threadid);

			// NOTE: For multicore mode, we need to add a new
			// breakpoint before we restore the old one so that
			// the core doesn't run ahead and skip past the
			// next breakpoint before we can add it.
			// On a single core machine this is not a problem.

			// Step the PC
			uint32_t instr = *(uint32_t*)PC;
			uint32_t newPC;
			if ((instr&3) == 0x3) // Uncompressed?
				newPC = PC+4;
			else // Compressed?
				newPC = PC+2;
			// Add breakpoint at next instuction
			TaskInsertBreakpoint(taskctx, threadid, newPC);
			// Restore old instruction
			TaskRemoveBreakpoint(taskctx, threadid, PC);
		}
		/*if (s_packet[6] == 'r') // (re?)start
			taskctx->tasks[threadid]. = ;*/
		if (s_packet[6] == 't') // stop
		{
			uint32_t PC = TaskGetPC(taskctx, threadid);
			TaskInsertBreakpoint(taskctx, threadid, PC);
		}

		SendAck();
		//SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "qSymbol")) // continue
	{
		SendAck();
		SendDebugPacket(""); // No symtable info support
	}
	else if (startswith(s_packet, "vKill")) // quit process
	{
		// Nothing else we can kill, so kill the main user task
		TaskExitTaskWithID(taskctx, 2, 0);

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "qC")) // current thread?
	{
		char threadid[10];
		uint2dec(s_currentThread, threadid);
		strcpy(outstring, "qC ");
		strcat(outstring, threadid);

		SendAck();
		SendDebugPacket(outstring);
	}
	else if (startswith(s_packet, "H"))
	{
		if (s_packet[2] == '-')
		{
			// -1 : all threads, we default to 'running user process'
			s_currentThread = 3;
		}
		else
		{
			char threadbuf[12];
			int a=0, p=2;
			while (s_packet[p]!='#' && s_packet[p]!=0)
				threadbuf[a++] = s_packet[p++];
			threadbuf[a]=0;
			uint32_t idx = hex2uint(threadbuf);
			s_currentThread = idx == 0 ? 2 : idx;
		}

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "P")) // Write register
	{
		char hexbuf[9], valuebuf[12];
		int a=0, r=0, p=1;
		while (s_packet[p]!='=' && s_packet[p]!=0)
			hexbuf[r++] = s_packet[p++];
		hexbuf[r]=0;
		r = hex2uint(hexbuf);
		++p; // Skip = sign
		while (s_packet[p]!='#' && s_packet[p]!=0)
			valuebuf[a++] = s_packet[p++];
		valuebuf[a]=0;
		uint32_t value = hex2uint(valuebuf);

		uint32_t threadid = s_currentThread - 1;

		if (r!=0)
			taskctx->tasks[threadid].regs[r%32] = value;
		else // PC
			taskctx->tasks[threadid].regs[r] = value;

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "p")) // Print register
	{
		char hexbuf[9];
		int r=0,p=1;
		while (s_packet[p]!='#' && s_packet[p]!=0)
			hexbuf[r++] = s_packet[p++];
		hexbuf[r]=0;
		r = hex2uint(hexbuf);

		uint32_t threadid = s_currentThread - 1;
		int2architectureorderedstring(r == 0 ? 0 : taskctx->tasks[threadid].regs[r%32], outstring);

		SendAck();
		SendDebugPacket(outstring); // Return register data
	}
	else if (startswith(s_packet, "T")) // thread status
	{
		char hexbuf[9];
		int r=0,p=1;
		while (s_packet[p]!='#' && s_packet[p]!=0)
			hexbuf[r++] = s_packet[p++];
		hexbuf[r]=0;
		r = hex2uint(hexbuf);

		s_currentThread = r;

		SendAck();

		uint32_t threadid = s_currentThread - 1;
		if (taskctx->tasks[threadid].state == TS_PAUSED)
			SendDebugPacket("E05"); // NOT correct!
		else
			SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "Z0")) // Software breakpoint
	{
		char offsetbuf[12];
		int a=0, p=2;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			offsetbuf[a++] = s_packet[p++];
		offsetbuf[a]=0;
		uint32_t address = dec2uint(offsetbuf);
		/*a = 0;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			offsetbuf[a++] = s_packet[p++];
		offsetbuf[a]=0;
		uint32_t type = dec2uint(offsetbuf);*/

		SendAck();

		uint32_t threadid = s_currentThread - 1;
		if (TaskInsertBreakpoint(taskctx, threadid, address))
			SendDebugPacket("OK");
		else
			SendDebugPacket("E00");
	}
	else if (startswith(s_packet, "z0")) // Remove breakpoint
	{
		char offsetbuf[12];
		int a=0, p=2;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			offsetbuf[a++] = s_packet[p++];
		offsetbuf[a]=0;
		uint32_t address = dec2uint(offsetbuf);
		/*a = 0;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			offsetbuf[a++] = s_packet[p++];
		offsetbuf[a]=0;
		uint32_t type = dec2uint(offsetbuf);*/

		SendAck();

		uint32_t threadid = s_currentThread - 1;
		if (TaskRemoveBreakpoint(taskctx, threadid, address))
			SendDebugPacket("OK");
		else
			SendDebugPacket("E00");
	}
	else if (startswith(s_packet, "X")) // Write binary blob
	{
		// NOTE: The header has been pre-processed

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "M")) // Set memory, maddr,count
	{
		char addrbuf[12], cntbuf[12];
		int a=0,c=0, p=1;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			addrbuf[a++] = s_packet[p++];
		addrbuf[a]=0;
		++p; // skip the comma
		while (s_packet[p]!=':' && s_packet[p]!=0)
			cntbuf[c++] = s_packet[p++];
		cntbuf[c]=0;
		++p; // skip the column

		uint32_t addrs = hex2uint(addrbuf);
		uint32_t numbytes = dec2uint(cntbuf);

		char bytebuf[3];
		bytebuf[2] = 0;
		for (uint32_t i=0; i<numbytes; ++i)
		{
			bytebuf[0] = s_packet[p++];
			bytebuf[1] = s_packet[p++];
			uint32_t byteval = hex2uint(bytebuf);
			*(uint8_t*)(addrs+i) = (uint8_t)byteval;
		}

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "m")) // Read memory, maddr,count
	{
		char addrbuf[12], cntbuf[12];
		int a=0,c=0, p=1;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			addrbuf[a++] = s_packet[p++];
		addrbuf[a]=0;
		++p; // skip the comma
		while (s_packet[p]!='#' && s_packet[p]!=0)
			cntbuf[c++] = s_packet[p++];
		cntbuf[c]=0;

		uint32_t addrs = hex2uint(addrbuf);
		uint32_t numbytes = dec2uint(cntbuf);

		// NOTE: When reading program memory, we need to
		// send across the 'saved' instruction bytes for the
		// breakpoints and not the 'ebreak' instructions.

		uint32_t ofst = 0;
		for (uint32_t i=0; i<numbytes; ++i)
		{
			uint8_t memval = *(uint8_t*)(addrs+i);
			byte2architectureorderedstring(memval, &outstring[ofst]);
			ofst += 2;
		}

		// Might need to update D$ & I$ if this was a write over instruction memory
		CFLUSH_D_L1;
		FENCE_I;

		SendAck();
		SendDebugPacket(outstring);
	}
	else if (startswith(s_packet, "g")) // dump GPR contents
	{
		uint32_t threadid = s_currentThread - 1;

		// All registers sent first
		int2architectureorderedstring(0x00000000, &outstring[0]); // zero register
		for(uint32_t i=1; i<32; ++i)
			int2architectureorderedstring(taskctx->tasks[threadid].regs[i], &outstring[i*8]);

		// PC is sent last (we use the space for zero register for PC)
		int2architectureorderedstring(taskctx->tasks[threadid].regs[0], &outstring[32*8]);

		SendAck();
		SendDebugPacket(outstring);
	}
	else if (startswith(s_packet, "s")) // single step
	{
		// TODO: need single step support
		//taskctx->tasks[threadid].state == TS_SINGLESTEP;

		SendAck();
		SendDebugPacket("");
	}
	else if (startswith(s_packet, "D")) // detach
	{
		s_debuggerConnected = 0;

		// Resume current thread since we're detaching
		uint32_t threadid = s_currentThread - 1;

		uint32_t PC = TaskGetPC(taskctx, threadid);
		TaskRemoveBreakpoint(taskctx, threadid, PC);

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "c")) // continue
	{
		uint32_t threadid = s_currentThread - 1;

		uint32_t PC = TaskGetPC(taskctx, threadid);
		TaskRemoveBreakpoint(taskctx, threadid, PC);

		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "!")) // Extended remote mode
	{
		SendAck();
		SendDebugPacket("OK");
	}
	else if (startswith(s_packet, "?")) // Halt reason?
	{
		// We need to halt the user thread to report the 'stop' reason
		s_currentThread = 3;
		uint32_t threadid = s_currentThread - 1;

		uint32_t PC = TaskGetPC(taskctx, threadid);
		TaskInsertBreakpoint(taskctx, threadid, PC);

		SendAck();
		SendDebugPacket("T05thread:3;"); // GDB_SIGNAL_TRAP (5)
	}
	/*else if (s_packet[0] == 0) // empty with a +, debugger seems to send this, wth?
	{
		SendAck();
		SendDebugPacket("+");
	}*/
	else
	{
		kprintf("UNKOWN: %s\n", s_packet);
		SendNack();
	}
}

void ProcessBinaryData(uint8_t input)
{
	// Store directly at target address since this won't fit into packet buffer
	*(uint8_t*)(s_binaryAddrs+s_receivedBytes) = input;
	s_receivedBytes++;

	if (s_receivedBytes == s_binaryNumbytes)
		s_gatherBinary = 0;
}

void ProcessChar(char input)
{
	// Do not queue this one
	if (!s_isBinaryHeader && (input == 0x03)) // CTRL+C
	{
		struct STaskContext *taskctx = GetTaskContext();

		uint32_t threadid = s_currentThread - 1;
		// Signal pause
		// taskctx->currentTask == 2 -> only user thread can be stopped
		if (threadid == 2)
		{
			uint32_t PC = TaskGetPC(taskctx, threadid);
			TaskInsertBreakpoint(taskctx, threadid, PC);

			SendAck();
			SendDebugPacket("T05thread:3;");
		}
		else
			SendNack();
		return;
	}

	if (s_checksumStart)
	{
		s_chk[s_checksumStart-1] = input;
		s_checksumStart++;
		if (s_checksumStart == 3)
		{
			s_checksumStart = 0;
			s_packetComplete = 1;
		}
	}

	if (s_isBinaryHeader && input == ':')
	{
		s_packet[s_idx++] = 0; // Zero terminate

		// Grab length and target address
		char addrbuf[12], cntbuf[12];
		int a=0,c=0, p=1;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			addrbuf[a++] = s_packet[p++];
		addrbuf[a]=0;
		++p; // skip the comma
		while (s_packet[p]!=':' && s_packet[p]!=0)
			cntbuf[c++] = s_packet[p++];
		cntbuf[c]=0;
		++p; // skip the column

		s_binaryAddrs = hex2uint(addrbuf);
		s_binaryNumbytes = hex2uint(cntbuf);
		s_receivedBytes = 0;
		s_isBinaryHeader = 0;

		//kprintf("X @0x%x : 0x%x bytes\n", s_binaryAddrs, s_binaryNumbytes);

		// Do not attempt to read bytes if we don't have any
		s_gatherBinary = (s_binaryNumbytes == 0) ? 0 : 1;
	}

	if (s_packetStart && input != '#')
	{
		// First character is an X, expect binary packet
		if (input == 'X' && s_idx == 0)
			s_isBinaryHeader = 1;

		s_packet[s_idx++] = input;
	}

	if (s_packetStart && input == '#')
	{
		s_packet[s_idx++] = 0; // Zero terminate
		s_checksumStart = 1;
		s_packetStart = 0;
	}

	if (input == '$')
	{
		s_idx = 0;
		s_packetStart = 1;
	}
}

void ProcessGDBRequest()
{
	if (s_packetComplete)
	{
		// Process pending packet
		HandlePacket();
		s_packetComplete = 0;
	}
	else
	{
		// Pull more incoming data
		uint8_t drain;
		while (SerialInRingBufferRead(&drain, 1))
		{
			// Debug output for incoming packet
			kprintf("%c", drain);

			if (s_gatherBinary)
				ProcessBinaryData(drain);
			else
			{
				ProcessChar(drain);
				// Stop spinning here and process the current packet
				if (s_packetComplete)
					break;
			}
		}
	}
}

uint32_t IsDebuggerConnected()
{
	return s_debuggerConnected;
}

// GDB serial packages are in the form:
// $packetdata#checksum
// where $ is the packet header, # is the footer and checksum is a two digit hex checksum.
// Any data between $ and # is the actual data / command transmitted by GDB.

// NOTE: Should we push incoming ASCII code into the key ringbuffer,
// if we're not listening to a debug packet?
// What happens if the user types a GDB packet by hand?

// GDB will post a CTRL+C (\003) character for break signalling
