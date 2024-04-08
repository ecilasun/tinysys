#include "basesystem.h"
#include "serialoutringbuffer.h"
#include "leds.h"

volatile uint32_t *UARTRECEIVE = (volatile uint32_t* ) (DEVICE_UART+0x00);
volatile uint32_t *UARTTRANSMIT = (volatile uint32_t* ) (DEVICE_UART+0x04);
volatile uint32_t *UARTSTATUS = (volatile uint32_t* ) (DEVICE_UART+0x08);
volatile uint32_t *UARTCONTROL = (volatile uint32_t* ) (DEVICE_UART+0x0C);

uint32_t UARTReceiveData()
{
	return *UARTRECEIVE;
}

void UARTSendByte(uint8_t data)
{
	SerialOutRingBufferWrite(&data, 1);
}

void UARTSendBlock(uint8_t *data, uint32_t numBytes)
{
	SerialOutRingBufferWrite(data, numBytes);
}

uint32_t UARTGetStatus()
{
 	return *UARTSTATUS;
}

void UARTSetControl(uint32_t ctl)
{
	*UARTCONTROL = ctl;
}

int UARTWrite(const char *outstring)
{
	uint32_t count = 0;
	while(outstring[count]!=0) { count++; }
	return SerialOutRingBufferWrite(outstring, count);
}

int UARTWriteHexByte(const uint8_t i)
{
	const char hexdigits[] = "0123456789ABCDEF";
	char msg[] = "  ";

	msg[0] = hexdigits[((i>>4)%16)];
	msg[1] = hexdigits[(i%16)];

	return SerialOutRingBufferWrite(msg, 2);
}

int UARTWriteHex(const uint32_t i)
{
	const char hexdigits[] = "0123456789ABCDEF";
	char msg[] = "        ";

	msg[0] = hexdigits[((i>>28)%16)];
	msg[1] = hexdigits[((i>>24)%16)];
	msg[2] = hexdigits[((i>>20)%16)];
	msg[3] = hexdigits[((i>>16)%16)];
	msg[4] = hexdigits[((i>>12)%16)];
	msg[5] = hexdigits[((i>>8)%16)];
	msg[6] = hexdigits[((i>>4)%16)];
	msg[7] = hexdigits[(i%16)];

	return SerialOutRingBufferWrite(msg, 8);
}

int UARTWriteDecimal(const int32_t i)
{
	const char digits[] = "0123456789";
	char msg[] = "                                ";

	int d = 1000000000;
	uint32_t enableappend = 0;
	uint32_t m = 0;

	if (i<0)
		msg[m++] = '-';

	for (int c=0;c<10;++c)
	{
		uint32_t r = ((i/d)%10)&0x7FFFFFFF;
		// Ignore preceeding zeros
		if ((r!=0) || enableappend || d==1)
		{
			enableappend = 1; // Rest of the digits can be appended
			msg[m++] = digits[r];
		}
		d = d/10;
	}
	msg[m] = 0;

	return UARTWrite(msg);
}

void UARTEmitBufferedOutput()
{
	// Copy out from FIFO to send buffer
	uint8_t out;
	while (SerialOutRingBufferRead(&out, 1))
		*UARTTRANSMIT = out;
}
