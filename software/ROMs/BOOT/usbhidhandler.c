#include "usbhidhandler.h"
#include "usbhost.h"
#include "usbserial.h"
#include "encoding.h"
#include "max3421e.h"
#include "ringbuffer.h"
#include "leds.h"

static uint32_t s_initialized = 0;

static uint32_t s_devicePollInterval = 10;
static uint8_t s_deviceProtocol = HID_PROTOCOL_NONE;
static uint8_t s_hidClass = 0;

static uint8_t *s_currentkeymap = (uint8_t*)(KEYBOARD_KEYTRACK_BASE);
static uint8_t *s_prevkeymap = (uint8_t*)(KEYBOARD_KEYTRACK_BASE+256);
static uint8_t s_devicecontrol[8];
static uint8_t s_deviceAddress = 0;
static uint8_t s_controlEndpoint = 0;

enum EUSBDeviceState olddevState = DEVS_UNKNOWN;
enum EUSBDeviceState devState = DEVS_UNKNOWN;

// EBusState
static uint32_t* s_probe_result = (uint32_t*)USB_HOST_STATE;
enum EBusState old_probe_result = BUSUNKNOWN;

void HandleUSBHID()
{
	uint32_t currLED = LEDGetState();

	uint8_t hirq_sendback = 0;

	uint8_t irq = MAX3421ReadByte(rHIRQ);

	if (irq&bmCONDETIRQ)
	{
		LEDSetState(currLED | 0x4);
		*s_probe_result = (uint32_t)USBBusProbe();
		hirq_sendback |= bmCONDETIRQ;
		LEDSetState(currLED);
	}
	else if (irq&bmFRAMEIRQ)
	{
		//printf()
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
		//printf("bmHXFRDNIRQ\n");
		hirq_sendback |= bmHXFRDNIRQ;
	}
	else if (irq&bmBUSEVENTIRQ)
	{
		// bus reset complete, or bus resume signalled
		//printf("bmBUSEVENTIRQ\n");
		hirq_sendback |= bmHXFRDNIRQ;
	}
	else
	{
		//printf("irq(unknown):%x\n", irq);
	}

	if (hirq_sendback)
		MAX3421WriteByte(rHIRQ, hirq_sendback);
}

void ProcessUSBDevice()
{
	uint64_t nextPoll = 0;
	uint16_t* keystates = (uint16_t*)KEYBOARD_KEYSTATE_BASE;

	if (s_initialized == 0)
	{
		s_initialized = 1;
		// Key map
		for (int i=0; i<256; ++i)
		{
			s_currentkeymap[i] = 0;
			s_prevkeymap[i] = 0;
		}

		// LED output
		for (int i=0; i<8; ++i)
			s_devicecontrol[i] = 0;
	}

	enum EBusState probe_result = (enum EBusState)*s_probe_result;
	uint32_t state_changed = probe_result != old_probe_result;

	// Disable MAX3421 interrupts so we don't fall into ISR while processing state
	//MAX3421WriteByte(rCPUCTL, 0);

	if (state_changed)
	{
		old_probe_result = probe_result;
		switch(probe_result)
		{
			case SE0:
				// Regardless of previous state, detach device
				//printf("SE0\n");
				devState = DEVS_DETACHED;
			break;

			case SE1:
				//printf("SE1\n");
				// This is an error state
			break;

			case FSHOST:
			case LSHOST:
				//printf("LSHOST/FSHOST\n");
				// Full or low speed device attached
				if (devState < DEVS_ATTACHED || devState >= DEVS_ERROR)
				{
					devState = DEVS_ATTACHED;
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
				//printf("DEVS_UNKNOWN\n");
			break;

			case DEVS_DETACHED:
			{
				//printf("DEVS_DETACHED\n");
				// We're always device #1
				uint8_t rcode = USBDetach(s_deviceAddress);
				if (rcode != 0)
					USBErrorString(rcode);
				//init: usbinit();
				//waitfordevice: MAX3421WriteByte(rHCTL, bmSAMPLEBUS);
				//illegal: no idea
				devState = rcode ? DEVS_ERROR : DEVS_UNKNOWN;
			}
			break;

			case DEVS_ATTACHED:
			{
				//printf("DEVS_ATTACHED\n");
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
				// Get device descriptor from default address and control endpoint
				uint8_t rcode = USBGetDeviceDescriptor(0, 0, &s_hidClass);
				// Assign device address
				if (rcode != 0)
					USBErrorString(rcode);
				devState = rcode ? DEVS_ERROR : DEVS_ADDRESSING;
			}
			break;

			case DEVS_ADDRESSING:
			{
				//printf("DEVS_ADDRESSING\n");
				uint8_t rcode = USBAttach(&s_deviceAddress, &s_controlEndpoint);
				uint64_t currentTime = E32ReadTime();
				nextPoll = currentTime + s_devicePollInterval*ONE_MILLISECOND_IN_TICKS;

				if (rcode == 0)// && s_deviceClass == HID)
				{
					rcode = USBConfigHID(s_hidClass, s_deviceAddress, s_controlEndpoint);
					if (rcode == 0)
					{
						rcode = USBGetHIDDescriptor(s_deviceAddress, s_controlEndpoint, &s_deviceProtocol);
						if (rcode != 0)
							USBErrorString(rcode);
					}
					else
						USBErrorString(rcode);
				}
				else
					USBErrorString(rcode);
				devState = rcode ? DEVS_ERROR : DEVS_RUNNING;
			}
			break;

			case DEVS_RUNNING:
			{
				//printf("DEVS_RUNNING\n");
				// Keep alive
				olddevState = DEVS_UNKNOWN;

				// TODO: Driver should handle this according to device type
				uint64_t currentTime = E32ReadTime();
				if (currentTime > nextPoll)
				{
					uint8_t keydata[8];

					nextPoll = currentTime + s_devicePollInterval*ONE_MICROSECOND_IN_TICKS;

					// TODO: Keyboard state should go to kernel memory from which applications can poll.
					// That mechanism should ultimately replace the ringbuffer approach used for UART input.

					if (s_deviceProtocol == HID_PROTOCOL_KEYBOARD)
					{
						uint8_t rcode = USBReadHIDData(s_deviceAddress, s_controlEndpoint, 8, keydata, 0x0, HID_REPORTTYPE_INPUT, 1);

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
								keystates[i] = keystate | modifierState;

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
							uint8_t lockstate = ((keystates[0x39]&1)?0x02:0x00) | ((keystates[0x53]&1)?0x01:0x00) | ((keystates[0x47]&1)?0x04:0x00);
							if (lockstate)
							{
								// Toggle previous state
								s_devicecontrol[0] ^= lockstate;
								// Reflect to device
								rcode = USBWriteHIDData(s_deviceAddress, s_controlEndpoint, s_devicecontrol);
								if (rcode)
									devState = DEVS_ERROR;
							}
						}
						else
						{
							if (rcode == hrSTALL)
							{
								uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
								rcode = USBControlRequest(s_deviceAddress, s_controlEndpoint, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, 0, 64);
								if (rcode == hrSTALL)
									devState = DEVS_ERROR;
							}
							else if (rcode != hrNAK)
							{
								// Only stop if not NAK
								USBErrorString(rcode);
								devState = DEVS_ERROR;
							}
						}
					}
					else if (s_deviceProtocol == HID_PROTOCOL_MOUSE)
					{
						// X/Y/Wheel/Button
						uint8_t rcode = USBReadHIDData(s_deviceAddress, 1, 4, keydata, 0x0, HID_REPORTTYPE_INPUT, 2);

						if (rcode == hrSTALL)
						{
							uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
							rcode = USBControlRequest(s_deviceAddress, s_controlEndpoint, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, 0, 64);
							if (rcode == hrSTALL)
								devState = DEVS_ERROR;
						}
						else if (rcode != hrNAK)
						{
							for (uint32_t i=0; i<4; ++i)
								USBSerialWriteHexByte(keydata[i]);
							USBSerialWrite("\n");
						}
					}
					else
					{
						// Nothing to talk about with this device since it's not HID
						//printf("\nNot a HID device\n");
						devState = DEVS_ERROR;
					}
				}
			}
			break;

			case DEVS_ERROR:
			{
				//printf("DEVS_ERROR\n");
				// Report error and stop device
				devState = DEVS_HALT;
			}
			break;

			case DEVS_HALT:
				//printf("DEVS_HALT\n");
			break;
		}
	}

	// Enable MAX3421 interrupts
	//MAX3421WriteByte(rCPUCTL, bmIE);
}
