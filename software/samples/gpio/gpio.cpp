#include "basesystem.h"
#include "gpio.h"
#include <stdio.h>

#define GPIO_OUTPUT_IO_0    4
#define GPIO_OUTPUT_IO_1    5
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))

int main()
{
    printf("GPIO test\n");

	E32Sleep(ONE_MILLISECOND_IN_TICKS);
	*GPIO_OUTPUTENABLE = GPIO_OUTPUT_PIN_SEL;

	for (uint32_t i=0; i<10; ++i)
	{
		E32Sleep(HUNDRED_MILLISECONDS_IN_TICKS);
		*GPIO_DATA = 0x00000000; // GPIO[4] and GPIO[5] Low

		E32Sleep(HUNDRED_MILLISECONDS_IN_TICKS);
		*GPIO_DATA = 0xFFFFFFFF; // GPIO[4] and GPIO[5] High
	}

	E32Sleep(ONE_MILLISECOND_IN_TICKS);
	*GPIO_OUTPUTENABLE = 0;

    return 0;
}
