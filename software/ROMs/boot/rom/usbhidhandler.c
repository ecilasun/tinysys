#include "usbhidhandler.h"
#include "usbhost.h"
#include "usbserial.h"
#include "encoding.h"
#include "max3421e.h"
#include "keyringbuffer.h"
#include "leds.h"
#include "rombase.h"
#include "keyboard.h"

static uint32_t s_devicePollInterval = 10;
static uint8_t s_deviceProtocol = HID_PROTOCOL_NONE;
static uint8_t s_hidClass = 0;
static uint64_t s_nextPoll = 0;

static int32_t *s_mposxy_buttons = (int32_t*)MOUSE_POS_AND_BUTTONS;
static int32_t *s_jposxy_buttons = (int32_t*)JOYSTICK_POS_AND_BUTTONS;

static uint8_t *s_currentkeymap = (uint8_t*)(KEYBOARD_KEYTRACK_BASE);
static uint8_t *s_prevkeymap = (uint8_t*)(KEYBOARD_KEYTRACK_BASE+256);
static uint32_t *s_keyinputgeneration = (uint32_t*)KEYBOARD_INPUT_GENERATION;

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
		hirq_sendback |= bmHXFRDNIRQ;
	}
	else if (irq&bmBUSEVENTIRQ)
	{
		// bus reset complete, or bus resume signalled
		hirq_sendback |= bmHXFRDNIRQ;
	}
	/*else
	{
		kprintf("irq(unknown):%x\n", irq);
	}*/

	if (hirq_sendback)
		MAX3421WriteByte(rHIRQ, hirq_sendback);
}

void InitializeUSBHIDData()
{
	// Key map
	for (int i=0; i<256; ++i)
	{
		s_currentkeymap[i] = 0;
		s_prevkeymap[i] = 0;
	}
	*s_keyinputgeneration = 0x0;

	// LED output
	for (int i=0; i<8; ++i)
		s_devicecontrol[i] = 0;
}

enum EUSBDeviceState HandleKeyboard(enum EUSBDeviceState _currentState)
{
	enum EUSBDeviceState returnState = _currentState;

	uint8_t *keyboarddata = (uint8_t*)KERNEL_TEMP_MEMORY;

	// Key report
	uint8_t rcode = USBReadHIDData(s_deviceAddress, s_controlEndpoint, 8, keyboarddata, 0x0, HID_REPORTTYPE_INPUT, 1);

	if (rcode == 0)
	{
		// Reflect into current keymap
		for (uint32_t i=2; i<8; ++i)
		{
			uint8_t keyIndex = keyboarddata[i];
			if (keyIndex != 0)
				s_currentkeymap[keyIndex] = 1;
		}

		// Generate keyup / keydown flags
		uint16_t modifierState = keyboarddata[0]<<8;
		// 7  6  5  4  3  2  1  0
		// RG RA RS RC LG LA LS LC
		//uint8_t isGraphics = modifierState&0x8800 ? 1:0;
		//uint8_t isAlt = modifierState&0x4400 ? 1:0;
		uint8_t isShift = modifierState&0x2200 ? 1:0;
		uint8_t isControl = modifierState&0x1100 ? 1:0;
		uint8_t isCaps = isShift | (s_devicecontrol[0]&0x02);
		uint16_t *keystates = GetKeyStateTable();
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
				uint32_t incoming;
				if (((i==HKEY_C) && isControl) || (i==HKEY_PAUSE))
					incoming = 3; // EXT (CTRL+C) or PAUSE key
				else
					incoming = KeyScanCodeToASCII(i, isCaps);
				if (incoming) // Add only valid entries
					KeyRingBufferWrite(&incoming, sizeof(uint32_t));
			}
		}

		// Remember current state
		__builtin_memcpy(s_prevkeymap, s_currentkeymap, 256);

		// TODO: Reset only after key repeat rate (~200 ms)
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
				returnState = DEVS_ERROR;
		}

		// Increment generation counter once we're done
		*s_keyinputgeneration = (*s_keyinputgeneration) + 1;
	}
	else
	{
		if (rcode == hrSTALL)
		{
			uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
			rcode = USBControlRequest(s_deviceAddress, s_controlEndpoint, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, 0, 64);
			if (rcode == hrSTALL)
				returnState = DEVS_ERROR;
		}
		else if (rcode != hrNAK)
		{
			// Only stop if not NAK
			USBErrorString(rcode);
			returnState = DEVS_ERROR;
		}
	}

	return returnState;
}

enum EUSBDeviceState HandleMouse(enum EUSBDeviceState _currentState)
{
	enum EUSBDeviceState returnState = _currentState;

	uint8_t *mousedata = (uint8_t*)KERNEL_TEMP_MEMORY;

	// X/Y/Wheel/Button
	uint8_t rcode = USBReadHIDData(s_deviceAddress, 1, 4, mousedata, 0x0, HID_REPORTTYPE_INPUT, 2);

	if (rcode == hrSTALL)
	{
		uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
		rcode = USBControlRequest(s_deviceAddress, s_controlEndpoint, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, 0, 64);
		if (rcode == hrSTALL)
			returnState = DEVS_ERROR;
	}
	else if (rcode != hrNAK)
	{
		s_mposxy_buttons[0] += (int32_t)mousedata[1]; // X
		s_mposxy_buttons[1] += (int32_t)mousedata[2]; // Y
		s_mposxy_buttons[2] = mousedata[0]; // Button
		if (s_mposxy_buttons[0] < 0) s_mposxy_buttons[0] = 0;
		if (s_mposxy_buttons[1] < 0) s_mposxy_buttons[1] = 0;
		if (s_mposxy_buttons[0] > 639) s_mposxy_buttons[0] = 639;
		if (s_mposxy_buttons[1] > 479) s_mposxy_buttons[1] = 479;
	}

	return returnState;
}

enum EUSBDeviceState HandleJoystick(enum EUSBDeviceState _currentState)
{
	enum EUSBDeviceState returnState = _currentState;

	uint8_t *joystickdata = (uint8_t*)KERNEL_TEMP_MEMORY;

	uint8_t rcode = USBReadHIDData(s_deviceAddress, 1, 8, joystickdata, 0x0, HID_REPORTTYPE_INPUT, 4);
	if (rcode == hrSTALL)
	{
		uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
		rcode = USBControlRequest(s_deviceAddress, s_controlEndpoint, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, 0, 64);
		if (rcode == hrSTALL)
			returnState = DEVS_ERROR;
	}
	else if (rcode != hrNAK)
	{
		s_jposxy_buttons[0] = (int32_t)joystickdata[3]; // X
		s_jposxy_buttons[1] = (int32_t)joystickdata[4]; // Y
		s_jposxy_buttons[2] = joystickdata[5]; // Buttons #0
		s_jposxy_buttons[3] = joystickdata[6]; // Buttons #1
	}

	return returnState;
}

// NOTE: Only works for PS4 controller for now
/*enum EUSBDeviceState HandleGamepad(enum EUSBDeviceState _currentState)
{
	enum EUSBDeviceState returnState = _currentState;

	uint8_t *gamepaddata = (uint8_t*)KERNEL_TEMP_MEMORY;

	uint8_t rcode = USBReadHIDData(s_deviceAddress, 1, 40, gamepaddata, 0x0, HID_REPORTTYPE_INPUT, 4);
	if (rcode == hrSTALL)
	{
		uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
		rcode = USBControlRequest(s_deviceAddress, s_controlEndpoint, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, 0, 64);
		if (rcode == hrSTALL)
			returnState = DEVS_ERROR;
	}
	else if (rcode != hrNAK)
	{
		// DEBUG: Dump report
		// Assuming PS4 controller report with header == 0x01
		for (uint32_t i=0;i<40;++i)
			kprintf("%x",gamepaddata[i]);
		kprintf("\n");

		//s_jposxy_buttons[0] = (int32_t)gamepaddata[2]; // left X (4 for right)
		//s_jposxy_buttons[1] = (int32_t)gamepaddata[3]; // left Y (5 for right)
		//s_jposxy_buttons[2] = gamepaddata[6]; // Buttons #0
		//s_jposxy_buttons[3] = gamepaddata[7]; // Buttons #1 (one more in 8)
	}

	return returnState;
}*/

void ProcessUSBDevice()
{
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
				devState = DEVS_DETACHED;
			break;

			case SE1:
				// This is an error state
			break;

			case FSHOST:
			case LSHOST:
				// Full or low speed device attached
				if (devState < DEVS_ATTACHED || devState >= DEVS_ERROR)
				{
					devState = DEVS_ATTACHED;
				}
			break;

			case BUSUNKNOWN:
			break;

			case CHIPFAILURE:
				devState = DEVS_ERROR;
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
				//
			break;

			case DEVS_DETACHED:
			{
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
				uint8_t rcode = USBGetDeviceDescriptor(0, 0, &s_hidClass, NULL, NULL);
				// Assign device address
				if (rcode != 0)
					USBErrorString(rcode);
				devState = rcode ? DEVS_ERROR : DEVS_ADDRESSING;
			}
			break;

			case DEVS_ADDRESSING:
			{
				uint8_t rcode = USBAttach(&s_deviceAddress, &s_controlEndpoint);
				uint64_t currentTime = E32ReadTime();
				s_nextPoll = currentTime + s_devicePollInterval*ONE_MILLISECOND_IN_TICKS;

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
				// Keep alive
				olddevState = DEVS_UNKNOWN;

				// TODO: Driver should handle this according to device type
				uint64_t currentTime = E32ReadTime();
				if (currentTime > s_nextPoll)
				{
					s_nextPoll = currentTime + s_devicePollInterval*ONE_MICROSECOND_IN_TICKS;
					if (s_deviceProtocol == HID_PROTOCOL_KEYBOARD)
						devState = HandleKeyboard(devState);
					else if (s_deviceProtocol == HID_PROTOCOL_MOUSE)
						devState = HandleMouse(devState);
					else if (s_deviceProtocol == HID_PROTOCOL_JOYSTICK)
						devState = HandleJoystick(devState);
					/*else if (s_deviceProtocol == HID_PROTOCOL_GAMEPAD)
						devState = HandleGamepad(devState);*/
					else
						devState = DEVS_ERROR;
				}
			}
			break;

			case DEVS_ERROR:
			{
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

int GetUSBDeviceState()
{
	return (int)devState;
}
