#include "basesystem.h"
#include "leds.h"

volatile uint32_t *UARTRECEIVE = (volatile uint32_t* ) (DEVICE_UART+0x00);
volatile uint32_t *UARTTRANSMIT = (volatile uint32_t* ) (DEVICE_UART+0x04);
volatile uint32_t *UARTSTATUS = (volatile uint32_t* ) (DEVICE_UART+0x08);
volatile uint32_t *UARTCONTROL = (volatile uint32_t* ) (DEVICE_UART+0x0C);

uint32_t UARTReceiveData()
{
	return *UARTRECEIVE;
}

void UARTSendData(uint32_t data)
{
	*UARTTRANSMIT = data;
}

uint32_t UARTGetStatus()
{
 	return *UARTSTATUS;
}

void UARTSetControl(uint32_t ctl)
{
	*UARTCONTROL = ctl;
}
