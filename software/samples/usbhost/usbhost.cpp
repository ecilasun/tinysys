#include "basesystem.h"
#include "uart.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>

int main(int argc, char *argv[])
{
	struct SUSBContext s_usbhostctx;
    USBHostSetContext(&s_usbhostctx);

	UARTWrite("Bringing up USB-A\nUsing polling\n");
	USBHostInit(0);

	UARTWrite("MAX3421 die rev# ");
	UARTWriteHexByte(MAX3421ReadByte(rREVISION));
	UARTWrite("\n");

	UARTWrite("Polling...");
	uint8_t v = 0xFF;
	do
	{
		uint8_t v2 = MAX3421ReadByte(rHIRQ);
		if (v2!=v)
		{
			v=v2;
			UARTWrite("HIRQ: ");
			UARTWriteHexByte(v);
			UARTWrite("\n");
		}
	} while (1);
	

	return 0;
}
