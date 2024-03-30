#include "basesystem.h"
#include "gpio.h"
#include <stdio.h>

#define GPIO_OUTPUT_IO_0    4
#define GPIO_OUTPUT_IO_1    5
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))

#define GPIO_INPUT_IO_0     9
#define GPIO_INPUT_IO_1     10
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

int main()
{
    printf("GPIO test - polling\n");
	uint32_t state = 0;

	*GPIO_OUTPUTENABLE = GPIO_OUTPUT_PIN_SEL;
	*GPIO_INPUTENABLE = GPIO_INPUT_PIN_SEL;

	int count = 0;
	uint32_t prevval = (*GPIO_DATA) & GPIO_INPUT_PIN_SEL;
	do
	{
		// Did we receive anything form GPIO9 or GPIO10?
		uint32_t val = (*GPIO_DATA) & GPIO_INPUT_PIN_SEL;
		if (val != prevval)
		{
			prevval = val;
			printf("inputs: %.8x\n", val);
			// Write output state
			*GPIO_DATA = (state%2) ? 0xFFFFFFFF : 0x00000000;
			// Flip states of GPIO[4] and GPIO[5] between Low or High
			++state;
			++count;
		}
	} while (count < 10);

	*GPIO_OUTPUTENABLE = 0;
	*GPIO_INPUTENABLE = 0;

    return 0;
}
