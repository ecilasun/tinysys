#pragma once

#include <inttypes.h>


// Control register bits
//#define UARTCTL_RESET_TX_FIFO	0x00000001
//#define UARTCTL_RESET_RX_FIFO	0x00000002
//#define UARTCTL_RESERVED0		0x00000004
//#define UARTCTL_RESERVED1		0x00000008
//#define UARTCTL_ENABLEINTERRUPT	0x00000010

// Status register bits
#define UARTSTA_RXFIFO_VALID		0x00000001
//#define UARTSTA_RXFIFO_FULL			0x00000002
//#define UARTSTA_TXFIFO_EMPTY		0x00000004
//#define UARTSTA_TXFIFO_FULL			0x00000008
//#define UARTSTA_INTERRUPTENABLED	0x00000010
//#define UARTSTA_ERR_OVERRUN			0x00000020
//#define UARTSTA_ERR_FRAME			0x00000040
//#define UARTSTA_ERR_PARITY			0x00000080

// R/W port for LED status access
extern volatile uint32_t *UARTRXTX;
//extern volatile uint32_t *UARTRECEIVE;
//extern volatile uint32_t *UARTTRANSMIT;
//extern volatile uint32_t *UARTSTATUS;
//extern volatile uint32_t *UARTCONTROL;

//uint32_t UARTGetStatus();
//void UARTSetControl(uint32_t ctl);

uint32_t UARTReceiveData();
void UARTSendData();
