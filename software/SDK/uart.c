/** @file uart.c
 * 
 *  @brief UART driver
 *
 *  This file contains functions for UART communication.
 */

#include "basesystem.h"
#include "mini-printf.h"
#include "uart.h"

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
 * Write a byte to the serial output.
 */
void UARTSendByte(uint8_t data)
{
	*UARTTRANSMIT = data;
}

/**
 * @brief Send a block of data over the UART
 * 
 * Write a block of data to the serial output.
 */
void UARTSendBlock(uint8_t *data, uint32_t numBytes)
{
	for (uint32_t i = 0; i < numBytes; i++)
		*UARTTRANSMIT = data[i];
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
 * @brief Write a formatted string to the UART
 * 
 * Write a formatted string to the serial output.
 * 
 * @return Number of bytes written
 */
int UARTPrintf(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char *buffer = (char *)UART_OUTPUT_TEMP;
	int len = mini_vsnprintf(buffer, 8192, fmt, va);
	va_end(va);
	if (len)
		UARTSendBlock((uint8_t*)buffer, len);
	return len;
}