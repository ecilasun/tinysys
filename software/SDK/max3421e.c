#include "max3421e.h"
#include "basesystem.h"

volatile uint32_t *IO_USBATRX = (volatile uint32_t* )DEVICE_USBA; // Receive fifo
volatile uint32_t *IO_USBASTATUS = (volatile uint32_t* )(DEVICE_USBA+4); // Output FIFO state

#define ASSERT_MAX3421_CS *IO_USBATRX = 0x100;
#define RESET_MAX3421_CS *IO_USBATRX = 0x101;

static uint32_t statusF = 0;
static uint32_t sparebyte = 0;

uint8_t MAX3421GetGPX()
{
	return (*IO_USBASTATUS)&0x4;
}

uint8_t MAX3421OutFifoNotEmpty()
{
	return (*IO_USBASTATUS)&0x2;
}

uint8_t MAX3421ReceiveFifoNotEmpty()
{
	return (*IO_USBASTATUS)&0x1;
}

uint8_t __attribute__ ((noinline)) MAX3421SPIWrite(const uint8_t outbyte)
{
	*IO_USBATRX = outbyte;
	return *IO_USBATRX;
}

uint8_t MAX3421ReadByte(uint8_t command)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command);
	sparebyte = MAX3421SPIWrite(0x00);
	RESET_MAX3421_CS

	return sparebyte;
}

void MAX3421WriteByte(uint8_t command, uint8_t data)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command | 0x02);
	sparebyte = MAX3421SPIWrite(data);
	RESET_MAX3421_CS
}

int MAX3421ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command);
	for (int i=0; i<length; i++)
		buffer[i] = MAX3421SPIWrite(0x00);
	RESET_MAX3421_CS
	return 0;
}

void MAX3421WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command | 0x02);
	for (int i=0; i<length; i++)
		sparebyte = MAX3421SPIWrite(buffer[i]);
	RESET_MAX3421_CS
}

int MAX3421CtlReset()
{
	// Reset MAX3421E by setting res high then low
	MAX3421WriteByte(rUSBCTL, bmCHIPRES);
	MAX3421WriteByte(rUSBCTL, 0);

	E32Sleep(3*ONE_MILLISECOND_IN_TICKS);

	// Wait for oscillator OK interrupt for the 12MHz external clock
	uint8_t rd = 0, cnt = 0;
	while ((rd & bmOSCOKIRQ) == 0 && cnt != 512)
	{
		rd = MAX3421ReadByte(rUSBIRQ);
		E32Sleep(3*ONE_MILLISECOND_IN_TICKS);
		++cnt;
	}
	MAX3421WriteByte(rUSBIRQ, bmOSCOKIRQ); // Clear IRQ

	if (cnt==512 && rd != 0)
		return 0;
	return 1;
}
