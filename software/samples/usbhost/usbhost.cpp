#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>
#include <stdio.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp

EBusState vbusState = SE0;

void EnumerateDevice()
{
	
}

int main(int argc, char *argv[])
{
	printf("\nUSB Host sample\n");

	struct SUSBContext s_usbhostctx;
    USBHostSetContext(&s_usbhostctx);

	if (argc>1)
	{
		USBHostInit(1);
	}
	else
	{
		USBHostInit(0);

		uint8_t m3421rev = MAX3421ReadByte(rREVISION);
		if (m3421rev != 0xFF)
			printf("MAX3421(host) rev# %d\n",m3421rev);
		else
			printf("MAX3421(host) disabled\n");

		// Clear initial connection detect interrupt
		MAX3421WriteByte(rHIRQ, bmCONDETIRQ);

		do
		{
			uint8_t irq = MAX3421ReadByte(rHIRQ);

			if (irq&bmFRAMEIRQ)
			{
				printf("irq(frame):%x", irq);
				MAX3421WriteByte(rHIRQ, bmFRAMEIRQ);
			}
			else if (irq&bmCONDETIRQ)
			{
				vbusState = USBBusProbe();
				printf("irq(condet)\n");
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
				printf("irq(unknown):%x\n", irq);
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
					MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmSOFKAENAB);
					LEDSetState(0x01);
				break;

				case LSHOST:
					MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmLOWSPEED | bmSOFKAENAB);
					LEDSetState(0x02);
				break;

				case BUSUNKNOWN:
					//
				break;
			}

		} while (1);
	}
	

	return 0;
}
