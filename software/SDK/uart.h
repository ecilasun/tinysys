#pragma once

#include <inttypes.h>

#include "basesystem.h"
#if defined(BUILDING_ROM)
#include "mini-printf.h"
#else
#include "tbm_printf.h"
#endif

// Control register bits
#define UARTCTL_RESET_TX_FIFO	0x00000001
#define UARTCTL_RESET_RX_FIFO	0x00000002
#define UARTCTL_RESERVED0		0x00000004
#define UARTCTL_RESERVED1		0x00000008
#define UARTCTL_ENABLEINTERRUPT	0x00000010

// Status register bits
#define UARTSTA_RXFIFO_VALID		0x00000001
#define UARTSTA_RXFIFO_FULL			0x00000002
#define UARTSTA_TXFIFO_EMPTY		0x00000004
#define UARTSTA_TXFIFO_FULL			0x00000008
#define UARTSTA_INTERRUPTENABLED	0x00000010
//#define UARTSTA_ERR_OVERRUN			0x00000020
//#define UARTSTA_ERR_FRAME			0x00000040
//#define UARTSTA_ERR_PARITY			0x00000080

void UARTInterceptSetState(int state);

uint32_t UARTGetStatus();
void UARTSetControl(uint32_t ctl);

uint32_t UARTReceiveData();
void UARTSendByte(uint8_t data);
void UARTSendBlock(uint8_t *data, uint32_t numBytes);

int UARTPrintf(const char *fmt, ...);