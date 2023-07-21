#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp
void BusProbe()
{
	uint8_t bus_sample;
	bus_sample = MAX3421ReadByte(rHRSL); // Get J,K status
	bus_sample &= (bmJSTATUS|bmKSTATUS); // zero the rest of the byte
	switch( bus_sample )
	{
		case bmJSTATUS:
			if((MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 ) {
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				//vbusState = FSHOST;
			}
			else {
				MAX3421WriteByte(rMODE, MODE_LS_HOST);        //start low-speed host
				//vbusState = LSHOST;
			}
			break;
		case bmKSTATUS:
			if(( MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 )
			{
				MAX3421WriteByte(rMODE, MODE_LS_HOST);       //start low-speed host
				//vbusState = LSHOST;
			}
			else
			{
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				//vbusState = FSHOST;
			}
			break;
		case bmSE1:              //illegal state
			//vbusState = SE1;
			break;
		case bmSE0:              //disconnected state
			//vbusState = SE0;
			break;
	}
}

int main(int argc, char *argv[])
{
	struct SUSBContext s_usbhostctx;
    USBHostSetContext(&s_usbhostctx);

	if (argc>1)
	{
		USBHostInit(1);
	}
	else
	{
		USBHostInit(0);

		do
		{
			uint8_t irq = MAX3421ReadByte(rHIRQ);
			if (irq&bmFRAMEIRQ)
			{
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
	}
	

	return 0;
}
