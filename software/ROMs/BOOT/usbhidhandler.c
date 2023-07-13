#include "usbhidhandler.h"
#include "usbhid.h"
#include "leds.h"

void HandleUSBHID()
{
	uint32_t currLED = LEDGetState();
	LEDSetState(currLED | 0x4);

	// TODO:

	LEDSetState(currLED);
}
