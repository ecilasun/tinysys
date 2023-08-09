#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include <malloc.h>
#include <stdio.h>

// Please see
// https://github.com/felis/ArduinoUSBhost/blob/master/Max3421e.cpp
// https://github.com/electricimp/reference/blob/master/hardware/max3421e/max3421e.device.nut#L1447
// https://github.com/felis/USB_Host_Shield_2.0
// https://github.com/felis/lightweight-usb-host/blob/81ed9d6f9fbefc6b33fdd5dfbd1a9636685f062b/transfer.c
// https://github.com/felis/USB_Host_Shield_2.0/blob/59cc3d287dd1afe8d89856a8d4de0ad8fe9ef3c7/usbhid.h

EBusState old_probe_result = BUSUNKNOWN;
EBusState probe_result = BUSUNKNOWN;

EUSBDeviceState olddevState = DEVS_UNKNOWN;
EUSBDeviceState devState = DEVS_UNKNOWN;

static uint8_t s_address = 0;
static uint8_t s_prevkeydata[8];
static uint8_t s_devicecontrol[8];

void EnumerateDevice()
{
	// TODO:
}

int main(int argc, char *argv[])
{
	uint64_t nextPoll = 0;
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

		for (int i=0;i<8;++i)
			s_prevkeydata[i] = 0;

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
				//printf()
				hirq_sendback |= bmFRAMEIRQ;
			}*/
			/*else if (irq&bmSNDBAVIRQ)
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
					{
						// We're always device #1
						uint8_t rcode = USBDetach(s_address);
						//init: usbinit();
						//waitfordevice: MAX3421WriteByte(rHCTL, bmSAMPLEBUS);
						//illegal: no idea
						devState = rcode ? DEVS_ERROR : DEVS_UNKNOWN;
					}
					break;

					case DEVS_ATTACHED:
					{
						printf("attached\n");
						// Wait 200ms on first attach for settle
						E32Sleep(200*ONE_MILLISECOND_IN_TICKS);
						// Once settled, reset device, wait for reset
						MAX3421WriteByte(rHCTL, bmBUSRST);
						while ((MAX3421ReadByte(rHCTL)&bmBUSRST) != 0) { asm volatile ("nop"); }
						// Start generating SOF
						MAX3421WriteByte(rMODE, MAX3421ReadByte(rMODE) | bmSOFKAENAB);
						E32Sleep(20*ONE_MILLISECOND_IN_TICKS);
						// Wait for first SOF
						while ((MAX3421ReadByte(rHIRQ)&bmFRAMEIRQ) == 0) { asm volatile ("nop"); }
						// Get device descriptor
						uint8_t rcode = USBGetDeviceDescriptor();
						// Assign device address
						devState = rcode ? DEVS_ERROR : DEVS_ADDRESSING;
					}
					break;

					case DEVS_ADDRESSING:
					{
						uint8_t rcode = USBAttach(&s_address);
						nextPoll = E32ReadTime() + 10*ONE_MILLISECOND_IN_TICKS;

						if (rcode == 0)
						{
							rcode = USBConfigHID(s_address);
							if (rcode == 0)
								rcode = USBGetHIDDescriptor(s_address);
						}

						devState = rcode ? DEVS_ERROR : DEVS_RUNNING;
					}
					break;

					case DEVS_RUNNING:
					{
						// Keep alive
						olddevState = DEVS_UNKNOWN;

						// TODO: Driver should handle this according to device type
						uint64_t currentTime = E32ReadTime();
						if (currentTime > nextPoll)
						{
							nextPoll = currentTime + 10*ONE_MICROSECOND_IN_TICKS;

							// Use maxpacketsize of the endpoint(8), the proper device address(1) and endpoint index(0 at 0x81)
							uint8_t keydata[8];
							//uint8_t ep = 0;
							//USBSetAddress(s_address, ep);
							//uint8_t rcode = USBInTransfer(s_address, ep, 8, (char*)keydata, 64);
							uint8_t rcode = USBReadHIDData(s_address, keydata);
							if (rcode == 0)
							{
								// TODO: Handle key held down at repeat rate
								// Did key states change?
								if (__builtin_memcmp(s_prevkeydata, keydata, 8))
								{
									// Store new state for next poll
									__builtin_memcpy(s_prevkeydata, keydata, 8);
									for (uint8_t k=0; k<2; ++k)
										printf("0x%.2x ", keydata[k]);
									for (uint8_t k=2; k<8; ++k)
										printf("%c", HIDScanToASCII(keydata[k], keydata[0]&0x22 ? 1:0));
									printf("\n");

									if (keydata[0]&0x22)
									{
										// numlock:0x01
										// caps:0x02
										// scrolllock:0x04
										s_devicecontrol[0] = 0x02; // Shift
										rcode = USBWriteHIDData(s_address, s_devicecontrol);
										if (rcode)
											devState = DEVS_ERROR;
									}
									else
									{
										s_devicecontrol[0] = 0x00;
										rcode = USBWriteHIDData(s_address, s_devicecontrol);
										if (rcode)
											devState = DEVS_ERROR;
									}
								}
							}
							else
							{
								printf("InTransfer error: 0x%.2x", rcode);
								devState = DEVS_ERROR;
							}
						}
					}
					break;

					case DEVS_ERROR:
					{
						printf("error\n");
						// Report error and stop device
						devState = DEVS_HALT;
					}
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
