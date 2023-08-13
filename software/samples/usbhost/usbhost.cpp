#include "basesystem.h"
#include "usbhid.h"
#include "leds.h"
#include "ringbuffer.h"
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
static uint8_t s_currentkeymap[256];
static uint8_t s_prevkeymap[256];
static uint16_t s_keystates[256];
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

		for (int i=0; i<256; ++i)
		{
			s_currentkeymap[i] = 0;
			s_prevkeymap[i] = 0;
			s_keystates[i] = 0;
		}
		for (int i=0; i<8; ++i)
			s_devicecontrol[i] = 0;

		uint8_t m3421rev = MAX3421ReadByte(rREVISION);
		if (m3421rev != 0xFF)
			printf("MAX3421(host) rev# %d\n",m3421rev);
		else
			printf("MAX3421(host) disabled\n");

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
							uint8_t keydata[8];

							// The device says 'no faster than 10ms' which means I could poll it at 20ms intervals
							nextPoll = currentTime + 20*ONE_MICROSECOND_IN_TICKS;

							// TODO: Keyboard state should go to kernel memory from which applications can poll.
							// That mechanism should ultimately replace the ringbuffer approach used for UART input.

							// Use maxpacketsize of the endpoint(8), the proper device address(1) and endpoint index(0 at 0x81)
							uint8_t rcode = USBReadHIDData(s_address, keydata);
							if (rcode == 0)
							{
								// Reflect into current keymap
								for (uint32_t i=2; i<8; ++i)
								{
									uint8_t keyIndex = keydata[i];
									if (keyIndex != 0)
										s_currentkeymap[keyIndex] = 1;
								}

								// Generate keyup / keydown flags
								uint16_t modifierState = keydata[0]<<8;
								// 7  6  5  4  3  2  1  0
								// RG RA RS RC LG LA LS LC
								//uint8_t isGraphics = modifierState&0x8800 ? 1:0;
								//uint8_t isAlt = modifierState&0x4400 ? 1:0;
								uint8_t isShift = modifierState&0x2200 ? 1:0;
								//uint8_t isControl = modifierState&0x1100 ? 1:0;
								uint8_t isCaps = isShift | (s_devicecontrol[0]&0x02);
								for (uint32_t i=0; i<256; ++i)
								{
									uint16_t keystate = 0;
									uint8_t prevstate = s_prevkeymap[i];
									uint8_t currentstate = s_currentkeymap[i];
									if (!prevstate && currentstate) keystate |= 1; // key down
									if (prevstate && !currentstate) keystate |= 2; // key up
									//if (prevstate && currentstate) keystate |= 4; // repeat

									// Update up/down state map alongside current modifier state
									s_keystates[i] = keystate | modifierState;

									// Insert down keys into input fifo in scan order
									// NOTE: Could be moved out of here
									if (keystate&1)
									{
										// Insert capital/lowercase ASCII code into input fifo
										uint32_t incoming = HIDScanToASCII(i, isCaps);
										RingBufferWrite(&incoming, sizeof(uint32_t));
									}
								}

								// Remember current state
								__builtin_memcpy(s_prevkeymap, s_currentkeymap, 256);

								// Reset only after key repeat rate (~200 ms)
								__builtin_memset(s_currentkeymap, 0, 256);

								// Toggle LEDs based on locked key state change
								// numlock:0x01
								// caps:0x02
								// scrolllock:0x04
								// Any of the lock keys down?
								uint8_t lockstate = ((s_keystates[0x39]&1)?0x02:0x00) | ((s_keystates[0x53]&1)?0x01:0x00) | ((s_keystates[0x47]&1)?0x04:0x00);
								if (lockstate)
								{
									// Toggle previous state
									s_devicecontrol[0] ^= lockstate;
									// Reflect to device
									rcode = USBWriteHIDData(s_address, s_devicecontrol);
									if (rcode)
										devState = DEVS_ERROR;
								}
							}
							else
							{
								// This appears to happen after a while :/
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
