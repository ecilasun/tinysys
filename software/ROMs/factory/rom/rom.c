// Factory test ROM
// Only has access to UART port

#include "rvcrt0.h"
#include <stdint.h>

volatile uint32_t *UARTFRead = (volatile uint32_t*)0x80000000;
volatile uint32_t *UARTFWrite = (volatile uint32_t*)0x80000004;
volatile uint32_t *UARTFByteCount = (volatile uint32_t*)0x80000008;

int main()
{
	*UARTFWrite = 'H';
	*UARTFWrite = 'e';
	*UARTFWrite = 'l';
	*UARTFWrite = 'l';
	*UARTFWrite = 'o';
	*UARTFWrite = ' ';
	*UARTFWrite = 'W';
	*UARTFWrite = 'o';
	*UARTFWrite = 'r';
	*UARTFWrite = 'l';
	*UARTFWrite = 'd';
	*UARTFWrite = '!';
	*UARTFWrite = '\n';

	while(1) {
		if (*UARTFByteCount)
			*UARTFWrite = *UARTFRead;
	}
}
