#include <string.h>

#include "basesystem.h"
#include "usbserial.h"
#include "debugstub.h"
#include "rombase.h"

static const char s_hexdigits[] = "0123456789ABCDEF";
static uint32_t s_debuggerConnected = 0;
static uint32_t s_packetStart = 0;
static uint32_t s_checksumStart = 0;
static uint32_t s_packetComplete = 0;
static uint32_t s_idx = 0;
static uint32_t s_fifocursor = 0;
static uint32_t s_currentThread = 1; // CMD
static char s_chk[2];
static char s_fifo[512];
static char s_packet[512];

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

void SendDebugPacket(const char *packetString)
{
	char checksumstr[3];
	StrChecksum(packetString, checksumstr);

	USBSerialWrite("+$");
	USBSerialWrite(packetString);
	USBSerialWrite("#");
	USBSerialWrite(checksumstr);
}

void HandlePacket()
{
	char outstring[512];
	if (strstr(s_packet, "qXfer:threads:read::")) // qXfer:threads:read::{start,offset}, normally would send l<?xml version=\"1.0\"?>\n<threads> etc
	{
		char offsetbuf[12];
		int a=0, p=20;
		while (s_packet[p]!=',' && s_packet[p]!=0)
			offsetbuf[a++] = s_packet[p++];
		offsetbuf[a]=0;
		uint32_t offset = dec2uint(offsetbuf);

		if (offset == 0)
		{
			struct STaskContext *taskctx = GetTaskContext();
			strcpy(outstring, "l<?xml version=\"1.0\"?>\n<threads>");
			for (uint32_t i=1; i<taskctx->numTasks; ++i)
			{
				char threadid[10];
				uint2dec(i, threadid);
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
	else if (strstr(s_packet, "vMustReplyEmpty"))
		SendDebugPacket("");
	else if (strstr(s_packet, "qSupported"))
	{
		s_debuggerConnected = 1;
		SendDebugPacket("+qSupported:swbreak+;hwbreak+;multiprocess-;qXfer:threads:read+;PacketSize=255");
	}
	else if (strstr(s_packet, "qOffsets")) // segment offsets (grab from ELF header)
		SendDebugPacket("Text=0;Data=0;Bss=0;"); // No relocation
	else if (strstr(s_packet, "qfThreadInfo"))
		SendDebugPacket("m-l"); // all
	else if (strstr(s_packet, "qsThreadInfo"))
		SendDebugPacket("l"); // no more info
	else if (strstr(s_packet, "qTStatus"))
		SendDebugPacket(""); // not supported
	else if (strstr(s_packet, "qAttached"))
		SendDebugPacket("1"); // locally attached (0 for process created)
	else if (strstr(s_packet, "vCont")) // continue
	{
		struct STaskContext *taskctx = GetTaskContext();
		taskctx->tasks[s_currentThread].ctrlc = 8;
		SendDebugPacket("OK");
	}
	else if (strstr(s_packet, "qSymbol")) // continue
		SendDebugPacket("OK"); // No symtable info required
	else if (strstr(s_packet, "qC")) // current thread?
	{
		int2architectureorderedstring(s_currentThread, outstring);
		SendDebugPacket(outstring);
	}
	else if (strstr(s_packet, "H"))
	{
		// m/M/g/G etc
		// NOTE: Thread id == -1 means 'all threads'
		if (s_packet[1]=='g') // select thread
		{
			char threadbuf[12];
			int a=0, p=2;
			while (s_packet[p]!='#' && s_packet[p]!=0)
				threadbuf[a++] = s_packet[p++];
			threadbuf[a]=0;
			uint32_t idx = hex2uint(threadbuf);
			s_currentThread = idx;
			SendDebugPacket("OK");
		}
		if (s_packet[1]=='c') // continue
		{
			struct STaskContext *taskctx = GetTaskContext();
			taskctx->tasks[s_currentThread].ctrlc = 8; // resume
			SendDebugPacket("OK");
		}
	}
	else if (strstr(s_packet, "p")) // print registers
	{
		char hexbuf[9];
		int r=0,p=1;
		while (s_packet[p]!='#' && s_packet[p]!=0)
			hexbuf[r++] = s_packet[p++];
		hexbuf[r]=0;
		r = hex2uint(hexbuf);

		struct STaskContext *taskctx = GetTaskContext();
		int2architectureorderedstring(r == 0 ? 0 : taskctx->tasks[s_currentThread].regs[r%32], outstring);

		SendDebugPacket(outstring); // Return register data
	}
	else if (strstr(s_packet, "T")) // thread status
	{
		char hexbuf[9];
		int r=0,p=1;
		while (s_packet[p]!='#' && s_packet[p]!=0)
			hexbuf[r++] = s_packet[p++];
		hexbuf[r]=0;
		r = hex2uint(hexbuf);

		s_currentThread = r;

		struct STaskContext *taskctx = GetTaskContext();
		if (taskctx->tasks[s_currentThread].state == TS_PAUSED)
			SendDebugPacket("E05");
		else
			SendDebugPacket("OK");
	}
	else if (strstr(s_packet, "M")) // Set memory, maddr,count
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

		SendDebugPacket("OK");
	}
	else if (strstr(s_packet, "m")) // Read memory, maddr,count
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

		uint32_t ofst = 0;
		for (uint32_t i=0; i<numbytes; ++i)
		{
			uint8_t memval = *(uint8_t*)(addrs+i);
			byte2architectureorderedstring(memval, &outstring[ofst]);
			ofst += 2;
		}

		SendDebugPacket(outstring);
	}
	else if (strstr(s_packet, "g")) // dump GPR contents
	{
		// All registers sent first
		struct STaskContext *taskctx = GetTaskContext();
		int2architectureorderedstring(0x00000000, &outstring[0]); // zero register
		for(uint32_t i=1;i<32;++i)
			int2architectureorderedstring(taskctx->tasks[s_currentThread].regs[i], &outstring[i*8]);

		// PC is sent last
		int2architectureorderedstring(taskctx->tasks[s_currentThread].regs[0], &outstring[32*8]);

		SendDebugPacket(outstring);
	}
	else if (strstr(s_packet, "s")) // single step
	{
		// TODO: need single step support
		//struct STaskContext *taskctx = GetTaskContext();
		//taskctx->tasks[s_currentThread].state == TS_SINGLESTEP;
		SendDebugPacket("");
	}
	else if (strstr(s_packet, "D")) // detach
	{
		s_debuggerConnected = 0;
		SendDebugPacket("OK");
	}
	else if (strstr(s_packet, "c")) // continue
	{
		struct STaskContext *taskctx = GetTaskContext();
		taskctx->tasks[s_currentThread].ctrlc = 8;
		SendDebugPacket("OK");
	}
	else if (strstr(s_packet, "?")) // Halt reason?
	{
		// Does S00 mean we're still running?
		SendDebugPacket("S00");
	}
	else
		USBSerialWrite("-");
}

void ProcessChar(char input)
{
	// Do not queue this one
	if (input == 0x03) // CTRL+C
	{
		// Signal pause
		struct STaskContext *taskctx = GetTaskContext();
		taskctx->tasks[s_currentThread].ctrlc = 1;
		SendDebugPacket("S05");
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

	if (s_packetStart && input != '#')
	{
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
	// Append incoming data to fifo
	uint32_t *rcvCount = (uint32_t *)SERIAL_INPUT_BUFFER;
	uint8_t *rcvData = (uint8_t *)(SERIAL_INPUT_BUFFER+4);
	uint32_t count = *rcvCount;
	if (count != 0)
	{
		// Flip and write to end of fifo
		for (uint32_t i=0; i<count; ++i)
			s_fifo[s_fifocursor++] = rcvData[count-1-i];
		// Make sure to reset for next round
		*rcvCount = 0;
	}

	// Pull incoming data
	while (s_fifocursor != 0)
	{
		char drain = s_fifo[--s_fifocursor];
		ProcessChar(drain);
		if (s_packetComplete)
		{
			HandlePacket();
			s_packetComplete = 0;
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
// Example:
// $qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+#df
// $Hg0#df
// $qTStatus#49
// qXfer:threads:read::0,250
// Hc-1
// qC
// qAttached
// qOffsets

// NOTE: Should we push incoming ASCII code into the key ringbuffer,
// if we're not listening to a debug packet?
// What happens if the user types a GDB packet by hand?

// GDB will post a CTRL+C (\003) character for break signalling
