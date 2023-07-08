#include "usbhidhandler.h"
#include "usbhid.h"
#include "leds.h"
#include "uart.h"

void HandleUSBHID()
{
	uint32_t currLED = LEDGetState();
	LEDSetState(currLED | 0x4);

	// TODO:

	LEDSetState(currLED);
}
