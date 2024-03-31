#include "basesystem.h"
#include "gpio.h"
#include <stdio.h>

// This sample works in combination with the gpio_example from ESP32-IDF samples
// The ESP32 code will write to pins 18 and 19 (which are our GPIO 9 and 10)
// On seeing a change, this code will send out alternating values on GPIO 4 and 5
// (which are pins 4 and 5 on the ESP side), causing the sample to output them.
//
// NOTE: Reads may not see both pins activate at the same time, WiP

#define GPIO_OUTPUT_IO_0    ESP32_IO4
#define GPIO_OUTPUT_IO_1    ESP32_IO5
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))

#define GPIO_INPUT_IO_0     ESP32_IO9
#define GPIO_INPUT_IO_1     ESP32_IO10
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

int main()
{
    printf("GPIO test - polling ESP32 GPIO pins...\n");

	*GPIO_OUTPUTENABLE = GPIO_OUTPUT_PIN_SEL;
	*GPIO_INPUTENABLE = GPIO_INPUT_PIN_SEL;

	int count = 0;
	uint32_t prevval = (*GPIO_DATA) & GPIO_INPUT_PIN_SEL;
	do
	{
		// Did we receive anything form GPIO9 or GPIO10?
		uint32_t inval = (*GPIO_DATA) & GPIO_INPUT_PIN_SEL;
		if (inval != prevval)
		{
			prevval = inval;

			printf("Inputs: %.8x\n", inval);

			// Write suitable output states (4 for 9, 5 for 10)
			uint32_t outval = 0;
			outval |= inval&(1ULL<<GPIO_INPUT_IO_0) ? (1ULL<<GPIO_OUTPUT_IO_0) : 0;
			outval |= inval&(1ULL<<GPIO_INPUT_IO_1) ? (1ULL<<GPIO_OUTPUT_IO_1) : 0;
			printf("Outputs: %.8x\n", outval);

			*GPIO_DATA = outval;

			++count;
		}
	} while (count < 10);

	*GPIO_OUTPUTENABLE = 0;
	*GPIO_INPUTENABLE = 0;

    return 0;
}
