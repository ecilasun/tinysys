#include "usbhidhandler.h"
#include "usbhost.h"
#include "max3421e.h"
#include "leds.h"

// EBusState
static uint32_t* s_probe_result = (uint32_t*)USB_HOST_STATE;

void HandleUSBHID()
{
	uint32_t currLED = LEDGetState();
	LEDSetState(currLED | 0x4);

	uint8_t hirq_sendback = 0;

	uint8_t irq = MAX3421ReadByte(rHIRQ);

	if (irq&bmCONDETIRQ)
	{
		*s_probe_result = (uint32_t)USBBusProbe();
		hirq_sendback |= bmCONDETIRQ;
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

	LEDSetState(currLED);
}
