#include "max3420e.h"
#include "basesystem.h"

volatile uint32_t *IO_USBCTRX = (volatile uint32_t* )DEVICE_USBC; // Receive fifo
volatile uint32_t *IO_USBCSTATUS = (volatile uint32_t* )(DEVICE_USBC+4); // Output FIFO state

#define ASSERT_MAX3420_CS *IO_USBCTRX = 0x100;
#define RESET_MAX3420_CS *IO_USBCTRX = 0x101;

static uint32_t statusF = 0;
static uint32_t sparebyte = 0;

uint8_t MAX3420GetGPX()
{
	return (*IO_USBCSTATUS)&0x4;
}

uint8_t MAX3420OutFifoNotEmpty()
{
	return (*IO_USBCSTATUS)&0x2;
}

void MAX3420FlushOutputFIFO()
{
	while ((*IO_USBCSTATUS)&0x2) {};
}

uint8_t MAX3420ReceiveFifoNotEmpty()
{
	return (*IO_USBCSTATUS)&0x1;
}

uint8_t __attribute__ ((noinline)) MAX3420SPIWrite(const uint8_t outbyte)
{
	*IO_USBCTRX = outbyte;
	return *IO_USBCTRX;
}

uint8_t MAX3420ReadByte(uint8_t command)
{
	ASSERT_MAX3420_CS
	statusF = MAX3420SPIWrite(command);
	sparebyte = MAX3420SPIWrite(0x00);
	RESET_MAX3420_CS

	return sparebyte;
}

void MAX3420WriteByte(uint8_t command, uint8_t data)
{
	ASSERT_MAX3420_CS
	statusF = MAX3420SPIWrite(command | 0x02);
	sparebyte = MAX3420SPIWrite(data);
	RESET_MAX3420_CS
}

int MAX3420ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3420_CS
	statusF = MAX3420SPIWrite(command);
	for (int i=0; i<length; i++)
		buffer[i] = MAX3420SPIWrite(0x00);
	RESET_MAX3420_CS
	return 0;
}

void MAX3420WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3420_CS
	statusF = MAX3420SPIWrite(command | 0x02);
	for (int i=0; i<length; i++)
		sparebyte = MAX3420SPIWrite(buffer[i]);
	RESET_MAX3420_CS
}

int MAX3420CtlReset()
{
	// Reset MAX3420E by setting res high then low
	MAX3420WriteByte(rUSBCTL, bmCHIPRES);
	MAX3420WriteByte(rUSBCTL, 0);

	E32Sleep(3*ONE_MILLISECOND_IN_TICKS);

	// Wait for oscillator OK interrupt for the 12MHz external clock
	uint8_t rd = 0, cnt = 0;
	while ((rd & bmOSCOKIRQ) == 0 && cnt != 512)
	{
		rd = MAX3420ReadByte(rUSBIRQ);
		E32Sleep(3*ONE_MILLISECOND_IN_TICKS);
		++cnt;
	}
	MAX3420WriteByte(rUSBIRQ, bmOSCOKIRQ); // Clear IRQ

	if (cnt==512 && rd != 0)
		return 0;
	return 1;
}
