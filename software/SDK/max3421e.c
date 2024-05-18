/**
 * @file max3421e.c
 * 
 * @brief This file contains the MAX3421E USB host controller driver functions
 */

#include "max3421e.h"
#include "basesystem.h"

volatile uint32_t *IO_USBATRX = (volatile uint32_t* )DEVICE_USBA; // Receive fifo
volatile uint32_t *IO_USBASTATUS = (volatile uint32_t* )(DEVICE_USBA+4); // Output FIFO state

#define ASSERT_MAX3421_CS *IO_USBATRX = 0x100;
#define RESET_MAX3421_CS *IO_USBATRX = 0x101;

static uint32_t statusF = 0;
static uint32_t sparebyte = 0;

/**
 * @brief Get the state of the GPX pin
 * 
 * The GPX pin is used to signal the presence of a USB device.
 * 
 * @return 1 if the GPX pin is high, 0 if the GPX pin is low
 */
uint8_t MAX3421GetGPX()
{
	return (*IO_USBASTATUS)&0x4;
}

/**
 * @brief Check if the MAX3421E output FIFO is empty
 * 
 * @return 1 if the output FIFO is empty, 0 if the output FIFO is not empty
 */
uint8_t MAX3421OutFifoNotEmpty()
{
	return (*IO_USBASTATUS)&0x2;
}

/**
 * @brief Check if the MAX3421E receive FIFO is not empty
 * 
 * @return 1 if the receive FIFO is not empty, 0 if the receive FIFO is empty
 */
uint8_t MAX3421ReceiveFifoNotEmpty()
{
	return (*IO_USBASTATUS)&0x1;
}

/**
 * @brief Write a byte to the MAX3421E SPI interface
 * 
 * @param outbyte Byte to write
 * @return Byte read from the MAX3421E
 */
uint8_t __attribute__ ((noinline)) MAX3421SPIWrite(const uint8_t outbyte)
{
	*IO_USBATRX = outbyte;
	return *IO_USBATRX;
}

/**
 * @brief Read a byte from the MAX3421E SPI interface
 * 
 * @param command Command byte to read
 * @return Byte read from the MAX3421E
 */
uint8_t MAX3421ReadByte(uint8_t command)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command);
	sparebyte = MAX3421SPIWrite(0x00);
	RESET_MAX3421_CS

	return sparebyte;
}

/**
 * @brief Write a command to the MAX3421E SPI interface
 * 
 * @param command Command byte to write
 * @param data Data byte to write
 */
void MAX3421WriteByte(uint8_t command, uint8_t data)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command | 0x02);
	sparebyte = MAX3421SPIWrite(data);
	RESET_MAX3421_CS
}

/**
 * @brief Send a command and read multiple byte response from the MAX3421E SPI interface
 * 
 * @param command Command to send
 * @param length Number of bytes to read
 * @param buffer Buffer to store the read bytes
 */
void MAX3421ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command);
	for (int i=0; i<length; i++)
		buffer[i] = MAX3421SPIWrite(0x00);
	RESET_MAX3421_CS
}

/**
 * @brief Send a command and write multiple byte parameter to the MAX3421E SPI interface
 * 
 * @param command Command to send
 * @param length Number of bytes to write
 * @param buffer Buffer containing the bytes to write
 */
void MAX3421WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	statusF = MAX3421SPIWrite(command | 0x02);
	for (int i=0; i<length; i++)
		sparebyte = MAX3421SPIWrite(buffer[i]);
	RESET_MAX3421_CS
}

/**
 * @brief Reset the MAX3421E USB host controller and enable interrupts on the MAX3421E
 * 
 * @return 1 if the reset was successful, 0 if the reset failed after 512 attempts
 */
int MAX3421CtlReset()
{
	// Reset MAX3421E by setting res high then low
	MAX3421WriteByte(rUSBCTL, bmCHIPRES);
	MAX3421WriteByte(rUSBCTL, 0);

	E32Sleep(3*ONE_MILLISECOND_IN_TICKS);

	// Wait for oscillator OK interrupt for the 12MHz external clock
	uint8_t rd = 0;
	uint32_t cnt = 0;
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
