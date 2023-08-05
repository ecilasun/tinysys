#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>
#include <stdio.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp

EBusState oldBusState = BUSUNKNOWN;
EBusState vbusState = BUSUNKNOWN;

EUSBDeviceState olddevState = DEVS_UNKNOWN;
EUSBDeviceState devState = DEVS_UNKNOWN;

// Setup data
uint8_t SUD[8];

void EnumerateDevice()
{
	// TODO:
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
				// Been waiting for config?
				if (devState == DEVS_ATTACHED_WAITINGCONFIG)
				{
					devState = DEVS_ADDRESSING;
					printf("irq(frame):%x\n", irq);
					// TODO: Request device descriptor then go to ADDRESSING state
				}
				MAX3421WriteByte(rHIRQ, bmFRAMEIRQ);
			}
			else if (irq&bmCONDETIRQ)
			{
				vbusState = USBBusProbe();
				MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
			}
			else if (irq&bmSNDBAVIRQ)
			{
				// Ignore send buffer available interrupt for now
				MAX3421WriteByte(rHIRQ, bmSNDBAVIRQ);
			}
			else if (irq&bmHXFRDNIRQ)
			{
				// TODO: response to our SETUP package
				printf("bmHXFRDNIRQ\n");
				MAX3421WriteByte(rHIRQ, bmHXFRDNIRQ);
			}
			else
			{
				printf("irq(unknown):%x\n", irq);
			}

			if (vbusState != oldBusState)
			{
				switch(vbusState)
				{
					case SE0:
						devState = DEVS_DETACHED;
						LEDSetState(0x00);
					break;

					case SE1:
						LEDSetState(0x0F);
					break;

					case FSHOST:
						if (devState != DEVS_ATTACHED)
						{
							MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmSOFKAENAB);
							devState = DEVS_ATTACHED;
							LEDSetState(0x01);
						}
					break;

					case LSHOST:
						if (devState != DEVS_ATTACHED)
						{
							MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmLOWSPEED | bmSOFKAENAB);
							devState = DEVS_ATTACHED;
							LEDSetState(0x02);
						}
					break;

					case BUSUNKNOWN:
						//
					break;
				}
				oldBusState = vbusState;
			}

			// USB task
			if (olddevState != devState)
			{
				switch(devState)
				{
					case DEVS_UNKNOWN:
						// ?
					break;

					case DEVS_DETACHED:
						printf("detached\n");
						//init: usbinit();
						//waitfordevice: MAX3421WriteByte(rHCTL, bmSAMPLEBUS);
						//illegal: no idea
					break;

					case DEVS_ATTACHED:
					{
						printf("attached\n");
						// wait 200ms on first attach for settle
						E32Sleep(200*ONE_MILLISECOND_IN_TICKS);
						// once settled, reset device, wait for reset
						MAX3421WriteByte(rHIRQ, bmBUSEVENTIRQ);
						MAX3421WriteByte(rHCTL, bmBUSRST);
						// then wait for reset
						while ((MAX3421ReadByte(rHCTL)&bmBUSRST) != 0) { asm volatile ("nop"); }
						// wait for FRAME
						printf("wfc\n");
						devState = DEVS_ATTACHED_WAITINGCONFIG;
					}
					break;

					case DEVS_ATTACHED_WAITINGCONFIG:
						// 
					break;

					case DEVS_ADDRESSING:
					{
						// See  https://github.com/felis/lightweight-usb-host/tree/master
						printf("assigning address\n");
						uint8_t addr = 0;
						uint8_t newaddr = 1;
						MAX3421WriteByte(rPERADDR, addr);
						// set an addres (index in internal table) for the device and send it across
						SUD[bmRequestType] = bmREQ_SET;
						SUD[bRequest] = USB_REQUEST_SET_ADDRESS;
						SUD[wValueL] = newaddr;
						SUD[wValueH] = 0;
						SUD[wIndexL] = 0;
						SUD[wIndexH] = 0;
						SUD[wLengthL] = 0;
						SUD[wLengthH] = 0;
						MAX3421WriteBytes(rSUDFIFO, 8, SUD);
						// Send setup package to peripheral
						uint8_t ep = 0;
						MAX3421WriteByte( rHXFR, (tokSETUP|ep));
						devState = DEVS_CONFIGURING;
					}
					break;

					case DEVS_CONFIGURING:
					{
						printf("configuring\n");
						// figure out the correct driver for the device
						// For now we have one device, so give it the address 1
						devState = DEVS_RUNNING;
					}
					break;

					case DEVS_RUNNING:
						// Nothing to do here, driver handles all
					break;

					case DEVS_ERROR:
						// Report error and stop device
					break;
				}
				olddevState = devState;
			}

		} while (1);
	}
	

	return 0;
}
