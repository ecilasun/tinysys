#include <inttypes.h>
#include <string.h>

#include "basesystem.h"
#include "usbserial.h"
#include "debugstub.h"
#include "rombase.h"

static uint32_t s_debuggerConnected = 0;
static uint32_t s_packetStart = 0;
static uint32_t s_checksumStart = 0;
static uint32_t s_packetComplete = 0;
static uint32_t s_idx = 0;
static uint32_t s_fifocursor = 0;
static char s_chk[2];
static char s_fifo[512];
static char s_packet[512];

void StrChecksum(const char *str, char *checksumstr)
{
	static const char hexdigits[] = "0123456789ABCDEF";

	int checksum = 0;
	int i=0;
	while(str[i]!=0)
	{
		checksum += str[i];
		++i;
	}
	checksum = checksum%256;

	checksumstr[0] = hexdigits[((checksum>>4)%16)];
	checksumstr[1] = hexdigits[(checksum%16)];
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
	if (strstr(s_packet, "qSupported"))
		SendDebugPacket("+qSupported:swbreak+;hwbreak+;multiprocess-;qXfer:threads:read+;PacketSize=255");
	else if (strstr(s_packet, "vMustReplyEmpty"))
		SendDebugPacket("");
	else if (strstr(s_packet, "?")) // Halt reason?
		SendDebugPacket("S05"); // TRAP
	else if (strstr(s_packet, "Hg")) // Hg0: operations apply to thread 0 (-1 -> all threads)
		SendDebugPacket("OK");
	else if (strstr(s_packet, "qTStatus"))
		SendDebugPacket(""); // not supported
	else if (strstr(s_packet, "qC")) // current thread?
		SendDebugPacket(""); // -1
	else if (strstr(s_packet, "qOffsets")) // segment offsets (grab from ELF header)
		SendDebugPacket("Text=0;Data=0;Bss=0;"); // -1
	else if (strstr(s_packet, "g")) //dump GPR contents
		SendDebugPacket("");
	else if (strstr(s_packet, "qXfer")) // qXfer:threads:read::{start,offset}, normally would send l<?xml version=\"1.0\"?>\n<threads> etc
		SendDebugPacket("");
	else
		USBSerialWrite("-");
}

void ProcessChar(char input)
{
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
		s_debuggerConnected = 1;
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
		kprintf("%c", drain);
		ProcessChar(drain);
		if (s_packetComplete)
		{
			HandlePacket();
			s_packetComplete = 0;
		}
	}
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
