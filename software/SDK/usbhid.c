#include "usbhid.h"
#include "basesystem.h"
#include "uart.h"
#include <string.h>

volatile uint32_t *IO_USBATRX = (volatile uint32_t* )DEVICE_USBA; // Receive fifo
volatile uint32_t *IO_USBASTA = (volatile uint32_t* )(DEVICE_USBA+4); // Output FIFO state

static uint32_t statusF = 0;
static uint32_t sparebyte = 0;

#define ASSERT_MAX3421_CS *IO_USBATRX = 0x100;
#define RESET_MAX3421_CS *IO_USBATRX = 0x101;

static struct SUSBContext *s_usbhost = NULL;

void USBHostSetContext(struct SUSBContext *ctx)
{
	s_usbhost = ctx;
}

struct SUSBContext *USBHostGetContext()
{
	return s_usbhost;
}

void MAX3421FlushOutputFIFO()
{
	while ((*IO_USBASTA)&0x1) {}
}

uint8_t MAX3421ReadByte(uint8_t command)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command;   // -> status in read FIFO
	statusF = *IO_USBATRX;   // unused
	*IO_USBATRX = 0;		 // -> read value in read FIFO
	sparebyte = *IO_USBATRX; // output value
	RESET_MAX3421_CS

	return sparebyte;
}

void MAX3421WriteByte(uint8_t command, uint8_t data)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command | 0x02; // -> zero in read FIFO
	statusF = *IO_USBATRX;		// unused
	*IO_USBATRX = data;		   // -> zero in read FIFO
	sparebyte = *IO_USBATRX;	  // unused
	RESET_MAX3421_CS
}

int MAX3421ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command;   // -> status in read FIFO
	statusF = *IO_USBATRX;   // unused
	//*IO_USBCTRX = 0;		 // -> read value in read FIFO
	//sparebyte = *IO_USBCTRX; // output value

	for (int i=0; i<length; i++)
	{
		*IO_USBATRX = 0;		  // send one dummy byte per input desired
		buffer[i] = *IO_USBATRX;  // store data byte
	}
	RESET_MAX3421_CS

	return 0;
}

void MAX3421WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command | 0x02;   // -> status in read FIFO
	statusF = *IO_USBATRX;		  // unused
	//*IO_USBATRX = 0;		 // -> read value in read FIFO
	//sparebyte = *IO_USBATRX; // output value

	for (int i=0; i<length; i++)
	{
		*IO_USBATRX = buffer[i];  // send one dummy byte per input desired
		sparebyte = *IO_USBATRX;  // unused
	}
	RESET_MAX3421_CS
}

void MAX3421CtlReset()
{
	// Reset MAX3420E by setting res high then low
	MAX3421WriteByte(rUSBCTL, bmCHIPRES);
	MAX3421WriteByte(rUSBCTL, 0);

	// Wait for all output to be sent to the device
	MAX3421FlushOutputFIFO();
	E32Sleep(3*ONE_MILLISECOND_IN_TICKS);

	// Wait for oscillator OK interrupt for the 12MHz external clock
	uint8_t rd = 0;
	while ((rd & bmOSCOKIRQ) == 0)
	{
		rd = MAX3421ReadByte(rUSBIRQ);
		E32Sleep(3*ONE_MILLISECOND_IN_TICKS);
	}
	MAX3421WriteByte(rUSBIRQ, bmOSCOKIRQ); // Clear IRQ
}

void USBHostInit(uint32_t enableInterrupts)
{
	// Must set context first
	if (s_usbhost==NULL)
		return;

	MAX3421WriteByte(rPINCTL, bmFDUPSPI | bmINTLEVEL | gpxSOF);
	MAX3421CtlReset();
	//MAX3421WriteByte(rGPIO, 0x0);

	MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST);
	MAX3421WriteByte(rHIEN, bmCONDETIE | bmFRAMEIE);
	MAX3421WriteByte(rHCTL, bmSAMPLEBUS);

	if (enableInterrupts)
	{
		//MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
		MAX3421WriteByte(rCPUCTL, bmIE);
	}

	// Generate descriptor table for a USB serial device
	/*MakeHostDescriptors(s_usbhost);

	// Enable full-duplex, int low, gpxA/B passthrough
	MAX3421WriteByte(rPINCTL, bmFDUPSPI | bmINTLEVEL | gpxSOF);

	// Reset the chip and wait for clock to stabilize
	MAX3421CtlReset();

	// Turn off GPIO
	MAX3421WriteByte(rGPIO, 0x0);

	// Connect
	MAX3421WriteByte(rUSBCTL, bmCONNECT | bmVBGATE);

	if (enableInterrupts)
	{
		MAX3421EnableIRQs();
		// Enable interrupt generation via INT pin
		MAX3421WriteByte(rCPUCTL, bmIE);
	}*/
}