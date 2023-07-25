#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>
#include <stdio.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp

enum EBusState
{
	SE0,
	SE1,
	FSHOST,
	LSHOST
};

EBusState vbusState = SE0;

void BusProbe()
{
	printf("busprobe:");

	uint8_t bus_sample;
	bus_sample = MAX3421ReadByte(rHRSL); // Get J,K status
	bus_sample &= (bmJSTATUS|bmKSTATUS); // zero the rest of the byte
	switch( bus_sample )
	{
		case bmJSTATUS:
			if((MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 ) {
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				vbusState = FSHOST;
			}
			else {
				MAX3421WriteByte(rMODE, MODE_LS_HOST);        //start low-speed host
				vbusState = LSHOST;
			}
			break;
		case bmKSTATUS:
			if(( MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 )
			{
				MAX3421WriteByte(rMODE, MODE_LS_HOST);       //start low-speed host
				vbusState = LSHOST;
			}
			else
			{
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				vbusState = FSHOST;
			}
			break;
		case bmSE1:              //illegal state
			vbusState = SE1;
			break;
		case bmSE0:              //disconnected state
			vbusState = SE0;
			break;
	}
}

void EnumerateDevice()
{
	
}

int main(int argc, char *argv[])
{
	printf("USB Host sample\n");

	struct SUSBContext s_usbhostctx;
    USBHostSetContext(&s_usbhostctx);

	if (argc>1)
	{
		USBHostInit(1);
	}
	else
	{
		USBHostInit(0);

		uint8_t state = 0;
		MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
		do{
			MAX3421WriteByte(rHCTL, bmSAMPLEBUS);
			state = MAX3421ReadByte(rHRSL);
			state &= (bmJSTATUS|bmKSTATUS);
		} while (state ==0);

		if (state == bmJSTATUS)
		{
			MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmSOFKAENAB);
			printf("full speed device\n");
		}

		if (state == bmKSTATUS)
		{
			MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmLOWSPEED | bmSOFKAENAB);
			printf("low speed device\n");
		}

		EnumerateDevice();

		/*BusProbe();
		MAX3421WriteByte(rHIRQ, bmCONDETIRQ);

		do
		{
			uint8_t irq = MAX3421ReadByte(rHIRQ);
			if (irq&bmFRAMEIRQ)
			{
				MAX3421WriteByte(rHIRQ, bmFRAMEIRQ);
				printf("irq(frame):%d\n", irq);
			}
			else if (irq&bmCONDETIRQ)
			{
				BusProbe();
				MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
			}
			else if (irq&bmSNDBAVIRQ)
			{
				// Ignore send buffer available interrupt
				//printf("irq(sndbav)\n");
				MAX3421WriteByte(rHIRQ, bmSNDBAVIRQ);
			}
			else
			{
				printf("irq(unknown):%d\n", irq);
			}

			switch(vbusState)
			{
				case SE0:
					LEDSetState(0x00);
				break;

				case SE1:
					LEDSetState(0x0F);
				break;

				case FSHOST:
					LEDSetState(0x01);
				break;

				case LSHOST:
					LEDSetState(0x02);
				break;
			}

		} while (1);*/
	}
	

	return 0;
}
