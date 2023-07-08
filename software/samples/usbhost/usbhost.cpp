#include "basesystem.h"
#include "uart.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp
void BusProbe()
{
	UARTWrite("Probing\n");

	uint8_t bus_sample;
	bus_sample = MAX3421ReadByte(rHRSL); // Get J,K status
	bus_sample &= (bmJSTATUS|bmKSTATUS); // zero the rest of the byte
	switch( bus_sample )
	{
		case bmJSTATUS:
			if((MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 ) {
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				UARTWrite("FSHOST\n");
				//vbusState = FSHOST;
			}
			else {
				MAX3421WriteByte(rMODE, MODE_LS_HOST);        //start low-speed host
				UARTWrite("LSHOST\n");
				//vbusState = LSHOST;
			}
			break;
		case bmKSTATUS:
			if(( MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 )
			{
				MAX3421WriteByte(rMODE, MODE_LS_HOST);       //start low-speed host
				UARTWrite("LSHOST\n");
				//vbusState = LSHOST;
			}
			else
			{
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				UARTWrite("FSHOST\n");
				//vbusState = FSHOST;
			}
			break;
		case bmSE1:              //illegal state
			UARTWrite("Illegalstate\n");
			//vbusState = SE1;
			break;
		case bmSE0:              //disconnected state
			UARTWrite("Disconnected\n");
			//vbusState = SE0;
			break;
	}
}

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
	do
	{
		uint8_t irq = MAX3421ReadByte(rHIRQ);
		if (irq&bmFRAMEIRQ)
		{
			UARTWrite("Frame\n");
			MAX3421WriteByte(rHIRQ, bmFRAMEIRQ);
		}
		if (irq&bmCONDETIRQ)
		{
			BusProbe();
			MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
		}
		/*else
		{
			UARTWriteHexByte(irq);
			UARTWrite(" ");
		}*/
	} while (1);
	

	return 0;
}
