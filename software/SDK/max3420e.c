#include "max3420e.h"
#include "basesystem.h"

volatile uint32_t *IO_USBCTRX = (volatile uint32_t* )DEVICE_USBC; // Receive fifo
volatile uint32_t *IO_USBCSTA = (volatile uint32_t* )(DEVICE_USBC+4); // Output FIFO state

#define ASSERT_MAX3420_CS *IO_USBCTRX = 0x100;
#define RESET_MAX3420_CS *IO_USBCTRX = 0x101;

static uint32_t statusF = 0;
static uint32_t sparebyte = 0;

void MAX3420FlushOutputFIFO()
{
	while ((*IO_USBCSTA)&0x1) {}
}

uint8_t MAX3420GetGPX()
{
	return (*IO_USBCSTA)&0x2;
}

uint8_t MAX3420ReadByte(uint8_t command)
{
	ASSERT_MAX3420_CS
	*IO_USBCTRX = command;   // -> status in read FIFO
	statusF = *IO_USBCTRX;   // unused
	*IO_USBCTRX = 0;		 // -> read value in read FIFO
	sparebyte = *IO_USBCTRX; // output value
	RESET_MAX3420_CS

	return sparebyte;
}

void MAX3420WriteByte(uint8_t command, uint8_t data)
{
	ASSERT_MAX3420_CS
	*IO_USBCTRX = command | 0x02; // -> zero in read FIFO
	statusF = *IO_USBCTRX;		// unused
	*IO_USBCTRX = data;		   // -> zero in read FIFO
	sparebyte = *IO_USBCTRX;	  // unused
	RESET_MAX3420_CS
}

int MAX3420ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3420_CS
	*IO_USBCTRX = command;   // -> status in read FIFO
	statusF = *IO_USBCTRX;   // unused
	//*IO_USBCTRX = 0;		 // -> read value in read FIFO
	//sparebyte = *IO_USBCTRX; // output value

	for (int i=0; i<length; i++)
	{
		*IO_USBCTRX = 0;		  // send one dummy byte per input desired
		buffer[i] = *IO_USBCTRX;  // store data byte
	}
	RESET_MAX3420_CS

	return 0;
}

void MAX3420WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3420_CS
	*IO_USBCTRX = command | 0x02;   // -> status in read FIFO
	statusF = *IO_USBCTRX;		  // unused
	//*IO_USBCTRX = 0;		 // -> read value in read FIFO
	//sparebyte = *IO_USBCTRX; // output value

	for (int i=0; i<length; i++)
	{
		*IO_USBCTRX = buffer[i];  // send one dummy byte per input desired
		sparebyte = *IO_USBCTRX;  // unused
	}
	RESET_MAX3420_CS
}

void MAX3420CtlReset()
{
	// Reset MAX3420E by setting res high then low
	MAX3420WriteByte(rUSBCTL, bmCHIPRES);
	MAX3420WriteByte(rUSBCTL, 0);

	// Wait for all output to be sent to the device
	MAX3420FlushOutputFIFO();
	E32Sleep(3*ONE_MILLISECOND_IN_TICKS);

	// Wait for oscillator OK interrupt for the 12MHz external clock
	uint8_t rd = 0;
	while ((rd & bmOSCOKIRQ) == 0)
	{
		rd = MAX3420ReadByte(rUSBIRQ);
		E32Sleep(3*ONE_MILLISECOND_IN_TICKS);
	}
	MAX3420WriteByte(rUSBIRQ, bmOSCOKIRQ); // Clear IRQ
}
