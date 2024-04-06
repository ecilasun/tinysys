#include "basesystem.h"
#include "uart.h"
#include "task.h"
#include <stdio.h>

int main()
{
    printf("UART test\n");

	do
	{
		*UARTRXTX = 'H';
		*UARTRXTX = 'e';
		*UARTRXTX = 'l';
		*UARTRXTX = 'l';
		*UARTRXTX = 'o';
		*UARTRXTX = ' ';
		*UARTRXTX = 'w';
		*UARTRXTX = 'o';
		*UARTRXTX = 'r';
		*UARTRXTX = 'l';
		*UARTRXTX = 'd';
		*UARTRXTX = '!';
		*UARTRXTX = 0;

		E32Sleep(ONE_SECOND_IN_TICKS);

		TaskYield();
	} while (1);

    return 0;
}
