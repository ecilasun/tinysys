#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>
#include <stdio.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp
// https://github.com/electricimp/reference/blob/master/hardware/max3421e/max3421e.device.nut#L1447
// https://github.com/felis/USB_Host_Shield/blob/880773a1cfd5a521f97493b0e4de7f243a2c2925/Usb.cpp#L383

EBusState old_probe_result = BUSUNKNOWN;
EBusState probe_result = BUSUNKNOWN;

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

		// This imitates the interrupt work
		do
		{
			uint8_t hirq_sendback = 0;

			uint8_t irq = MAX3421ReadByte(rHIRQ);

			if (irq&bmCONDETIRQ)
			{
				probe_result = USBBusProbe();
				hirq_sendback |= bmCONDETIRQ;
			}
			/*else if (irq&bmFRAMEIRQ)
			{
				// TODO:
				hirq_sendback |= bmFRAMEIRQ;
			}
			else if (irq&bmSNDBAVIRQ)
			{
				// Ignore send buffer available interrupt for now
				hirq_sendback |= bmSNDBAVIRQ;
			}
			else if (irq&bmHXFRDNIRQ)
			{
				// TODO: response to our SETUP package
				printf("bmHXFRDNIRQ\n");
				hirq_sendback |= bmHXFRDNIRQ;
			}
			else
			{
				printf("irq(unknown):%x\n", irq);
			}*/

			MAX3421WriteByte(rHIRQ, hirq_sendback);

			uint32_t state_changed = probe_result != old_probe_result;

			if (state_changed)
			{
				old_probe_result = probe_result;
				switch(probe_result)
				{
					case SE0:
						// Regardless of previous state, detach device
						printf("SE0\n");
						devState = DEVS_DETACHED;
						LEDSetState(0x00);
					break;

					case SE1:
						printf("SE1\n");
						// This is an error state
						LEDSetState(0x0F);
					break;

					case FSHOST:
					case LSHOST:
						printf("FS/LSHOST\n");
						// Full or low speed device attached
						if (devState < DEVS_ATTACHED || devState >= DEVS_ERROR)
						{
							devState = DEVS_ATTACHED;
							LEDSetState(0x01);
						}
					break;

					case BUSUNKNOWN:
						//
					break;
				}
			}

			// USB task
			if (olddevState != devState)
			{
				olddevState = devState;
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
						printf("attached, wfr\n");
						// Wait 200ms on first attach for settle
						E32Sleep(200*ONE_MILLISECOND_IN_TICKS);
						// Once settled, reset device, wait for reset
						MAX3421WriteByte(rHCTL, bmBUSRST);
						while ((MAX3421ReadByte(rHCTL)&bmBUSRST) != 0) { asm volatile ("nop"); }
						// Start generating SOF
						MAX3421WriteByte(rMODE, MAX3421ReadByte(rMODE) | bmSOFKAENAB);
						E32Sleep(20*ONE_MILLISECOND_IN_TICKS);
						// Wait for first SOF
						printf("wfsof\n");
						while ((MAX3421ReadByte(rHIRQ)&bmFRAMEIRQ) == 0) { asm volatile ("nop"); }
						// Get device descriptor
						uint8_t rcode = USBGetDeviceDescriptor();
						// Assign device address
						devState = rcode ? DEVS_ADDRESSING : DEVS_ERROR;
					}
					break;

					case DEVS_ADDRESSING:
					{
						printf("addressing\n");
						uint8_t rcode = USBAssignAddress();
						devState = rcode ? DEVS_CONFIGURING : DEVS_ERROR;
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
						printf("error\n");
						// Report error and stop device
						devState = DEVS_HALT;
					break;

					case DEVS_HALT:
						//
					break;
				}
			}

		} while (1);
	}
	

	return 0;
}
