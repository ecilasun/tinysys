// Factory test ROM
// Only has access to UART port

#include "rvcrt0.h"
#include "basesystem.h"
#include <stdint.h>

volatile uint32_t *UARTRECEIVE = (volatile uint32_t* ) (DEVICE_UART+0x00);
volatile uint32_t *UARTTRANSMIT = (volatile uint32_t* ) (DEVICE_UART+0x04);
volatile uint32_t *UARTSTATUS = (volatile uint32_t* ) (DEVICE_UART+0x08);
volatile uint32_t *UARTCONTROL = (volatile uint32_t* ) (DEVICE_UART+0x0C);

#define UARTSTA_RXFIFO_VALID 0x00000001

int main()
{
	*UARTTRANSMIT = 'H';
	*UARTTRANSMIT = 'e';
	*UARTTRANSMIT = 'l';
	*UARTTRANSMIT = 'l';
	*UARTTRANSMIT = 'o';
	*UARTTRANSMIT = ' ';
	*UARTTRANSMIT = 'W';
	*UARTTRANSMIT = 'o';
	*UARTTRANSMIT = 'r';
	*UARTTRANSMIT = 'l';
	*UARTTRANSMIT = 'd';
	*UARTTRANSMIT = '!';
	*UARTTRANSMIT = '\n';

	while (1)
	{
		if ((*UARTSTATUS) & UARTSTA_RXFIFO_VALID)
		{
			uint8_t rcvData = (uint8_t)((*UARTRECEIVE) & 0x000000FF);
			*UARTTRANSMIT = rcvData;
		}
	}
}
