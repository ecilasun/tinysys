/** @file uart.c
 * 
 *  @brief UART driver
 *
 *  This file contains functions for UART communication.
 */

#include "basesystem.h"
#include "serialoutringbuffer.h"
#include "leds.h"

volatile uint32_t *UARTRECEIVE = (volatile uint32_t* ) (DEVICE_UART+0x00);
volatile uint32_t *UARTTRANSMIT = (volatile uint32_t* ) (DEVICE_UART+0x04);
volatile uint32_t *UARTSTATUS = (volatile uint32_t* ) (DEVICE_UART+0x08);
volatile uint32_t *UARTCONTROL = (volatile uint32_t* ) (DEVICE_UART+0x0C);

// Borrowed from mini-printf (please see mini-printf.c for details and credits)
static unsigned int fast_itoa(int value, unsigned int radix, unsigned int uppercase, unsigned int unsig, char *buffer, unsigned int zero_pad)
{
	char *pbuffer = buffer;
	int	negative = 0;
	unsigned int	i, len;

	/* No support for unusual radixes. */
	if (radix > 16)
		return 0;

	if (value < 0 && !unsig) {
		negative = 1;
		value = -value;
	}

	/* This builds the string back to front ... */
	do {
		int digit = value % radix;
		*(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
		value /= radix;
	} while (value > 0);

	for (i = (pbuffer - buffer); i < zero_pad; i++)
		*(pbuffer++) = '0';

	if (negative)
		*(pbuffer++) = '-';

	*(pbuffer) = '\0';

	/* ... now we reverse it (could do it recursively but will
	 * conserve the stack space) */
	len = (pbuffer - buffer);
	for (i = 0; i < len / 2; i++) {
		char j = buffer[i];
		buffer[i] = buffer[len-i-1];
		buffer[len-i-1] = j;
	}

	return len;
}

/**
 * @brief Receive a byte from the UART
 * 
 * Receive a byte from the memory mapped UART device.
 * 
 * @return Byte received from the UART
 */
uint32_t UARTReceiveData()
{
	return *UARTRECEIVE;
}

/**
 * @brief Send a byte over the UART
 * 
 * Write a byte to the serial output ring buffer to be sent over the UART at a later time.
 */
void UARTSendByte(uint8_t data)
{
	while (!SerialOutRingBufferWrite(&data, 1))
		asm volatile ("nop");
}

/**
 * @brief Send a block of data over the UART
 * 
 * Write a block of data to the serial output ring buffer to be sent over the UART at a later time.
 */
void UARTSendBlock(uint8_t *data, uint32_t numBytes)
{
	while (!SerialOutRingBufferWrite(data, numBytes))
		asm volatile ("nop");
}

/**
 * @brief Get the status register of the UART
 * 
 * Get the status control register of the UART.
 * 
 * @return Status register value
 */
uint32_t UARTGetStatus()
{
 	return *UARTSTATUS;
}

/**
 * @brief Set the control register of the UART
 * 
 * Set some control bits of the UART.
 */
void UARTSetControl(uint32_t ctl)
{
	*UARTCONTROL = ctl;
}

/**
 * @brief Write a string to the UART
 * 
 * Write a string to the serial output ring buffer to be sent over the UART at a later time.
 * 
 * @return Number of bytes written
 */
int UARTWrite(const char *outstring)
{
	uint32_t count = 0;
	while(outstring[count]!=0) { count++; }
	while (!SerialOutRingBufferWrite(outstring, count))
		asm volatile ("nop");
	return count;
}

/**
 * @brief Write a 32-bit integer in hexadecimal format to the UART
 * 
 * Write a 32-bit integer in hexadecimal format to the serial output ring buffer to be sent over the UART at a later time.
 * 
 * @return Number of bytes written
 */
int UARTWriteHex(const uint32_t i)
{
	char msg[16];
	unsigned int len = fast_itoa(i, 16, 1, 1, msg, 0);

	if (len)
	{
		while (!SerialOutRingBufferWrite(msg, len))
			asm volatile ("nop");
	}

	return len;
}

/**
 * @brief Write a 32-bit integer in decimal format to the UART
 * 
 * Write a 32-bit integer in decimal format to the serial output ring buffer to be sent over the UART at a later time.
 * 
 * @return Number of bytes written
 */
int UARTWriteDecimal(const int32_t i)
{
	char msg[16];
	unsigned int len = fast_itoa(i, 10, 1, 1, msg, 0);

	if (len)
	{
		while (!SerialOutRingBufferWrite(msg, len))
			asm volatile ("nop");
	}

	return len;
}

/**
 * @brief Dump serial output ring buffer to UART
 * 
 * Write everything in the serial output ring buffer to the memory mapped UART device.
 * It will consume the entire buffer in one go.
 */
void UARTEmitBufferedOutput()
{
	// Copy out from FIFO to send buffer
	uint8_t out;
	while (SerialOutRingBufferRead(&out, 1))
		*UARTTRANSMIT = out;
}
