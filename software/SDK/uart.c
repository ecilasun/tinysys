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
	SerialOutRingBufferWrite(&data, 1);
}

/**
 * @brief Send a block of data over the UART
 * 
 * Write a block of data to the serial output ring buffer to be sent over the UART at a later time.
 */
void UARTSendBlock(uint8_t *data, uint32_t numBytes)
{
	SerialOutRingBufferWrite(data, numBytes);
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
	return SerialOutRingBufferWrite(outstring, count);
}

/**
 * @brief Write a byte in hexadecimal format to the UART
 * 
 * Write a byte in hexadecimal format to the serial output ring buffer to be sent over the UART at a later time.
 * 
 * @return Number of bytes written
 */
int UARTWriteHexByte(const uint8_t i)
{
	const char hexdigits[] = "0123456789ABCDEF";
	char msg[] = "  ";

	msg[0] = hexdigits[((i>>4)%16)];
	msg[1] = hexdigits[(i%16)];

	return SerialOutRingBufferWrite(msg, 2);
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
	const char hexdigits[] = "0123456789ABCDEF";
	char msg[] = "        ";

	msg[0] = hexdigits[((i>>28)%16)];
	msg[1] = hexdigits[((i>>24)%16)];
	msg[2] = hexdigits[((i>>20)%16)];
	msg[3] = hexdigits[((i>>16)%16)];
	msg[4] = hexdigits[((i>>12)%16)];
	msg[5] = hexdigits[((i>>8)%16)];
	msg[6] = hexdigits[((i>>4)%16)];
	msg[7] = hexdigits[(i%16)];

	return SerialOutRingBufferWrite(msg, 8);
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
	const char digits[] = "0123456789";
	char msg[] = "                                ";

	int d = 1000000000;
	uint32_t enableappend = 0;
	uint32_t m = 0;

	if (i<0)
		msg[m++] = '-';

	for (int c=0;c<10;++c)
	{
		uint32_t r = ((i/d)%10)&0x7FFFFFFF;
		// Ignore preceeding zeros
		if ((r!=0) || enableappend || d==1)
		{
			enableappend = 1; // Rest of the digits can be appended
			msg[m++] = digits[r];
		}
		d = d/10;
	}
	msg[m] = 0;

	return UARTWrite(msg);
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
