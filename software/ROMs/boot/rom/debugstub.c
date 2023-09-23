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
static char s_chk[2];
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
	// Grab next token if there's one
	char *token = strtok(s_packet, ";");
	while(token)
	{
		kprintf(":%s\n", token);

		if (strstr(token, "qSupported"))
			SendDebugPacket("qSupported:swbreak+;hwbreak+;multiprocess-;qXfer:threads:read+;PacketSize=512");
		else if (strstr(token, "vMustReplyEmpty"))
			SendDebugPacket("");
		else
			USBSerialWrite("-");
	}
}

void ProcessGDBRequest()
{
	if (s_packetComplete)
	{
		HandlePacket();
		s_packetComplete = 0;
		return;
	}

	uint32_t *rcvCount = (uint32_t *)SERIAL_INPUT_BUFFER;
	uint8_t *rcvData = (uint8_t *)(SERIAL_INPUT_BUFFER+4);
	uint32_t count = *rcvCount;
	if (count == 0)
		return;

	if (s_checksumStart)
	{
		kprintf("C");
		s_chk[s_checksumStart-1] = rcvData[0];
		s_checksumStart++;
		if (s_checksumStart == 3)
		{
			kprintf("\n");
			s_checksumStart = 0;
			s_packetComplete = 1;
		}
	}

	if (s_packetStart && rcvData[0] != '#')
	{
		kprintf(".");
		s_packet[s_idx++] = rcvData[0];
	}

	if (s_packetStart && rcvData[0] == '#')
	{
		kprintf("[EOP]");
		s_packet[s_idx++] = 0; // Zero terminate
		s_checksumStart = 1;
		s_packetStart = 0;
	}

	if (rcvData[0] == '$')
	{
		kprintf("[SOP]");
		s_debuggerConnected = 1;
		s_idx = 0;
		s_packetStart = 1;
	}

	// GDB serial packages are in the form:
	// $packetdata#checksum
	// where $ is the packet header, # is the footer and checksum is a two digit hex checksum.
	// Any data between $ and # is the actual data / command transmitted by GDB.
	// Example:
	// $qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+#df

	// NOTE: Should we push incoming ASCII code into the key ringbuffer,
	// if we're not listening to a debug packet?
	// What happens if the user types a GDB packet by hand?

	// Decay the input once it's read
	*rcvCount = --count;
	return;
}
