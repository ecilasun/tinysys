#include "basesystem.h"
#include "uart.h"
#include "task.h"
#include <stdio.h>

int main()
{
    printf("UART test\n");

	do
	{
		UARTSendData('H');
		UARTSendData('e');
		UARTSendData('l');
		UARTSendData('l');
		UARTSendData('o');
		UARTSendData(' ');
		UARTSendData('w');
		UARTSendData('o');
		UARTSendData('r');
		UARTSendData('l');
		UARTSendData('d');
		UARTSendData('!');
		UARTSendData(0);

		while (UARTGetStatus() & UARTSTA_RXFIFO_VALID)
		{
			uint32_t data = UARTReceiveData();
			printf("%c", char(data&0xFF));
		}

		E32Sleep(ONE_SECOND_IN_TICKS);

		TaskYield();
	} while (1);

    return 0;
}
