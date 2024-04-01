#include "basesystem.h"
#include "gpio.h"
#include "gpioringbuffer.h"
#include "task.h"
#include <stdio.h>

// This sample works in combination with the gpio_example from ESP32-IDF samples
// The ESP32 code will write to pins 18 and 19 (which are our GPIO 10 and 11)
// On seeing a change, this code will send out alternating values on GPIO 4 and 5
// (which are pins 4 and 5 on the ESP side), causing the sample to output them.
//
// NOTE: Reads may not see both pins activate at the same time, WiP

#define GPIO_OUTPUT_IO_0    ESP32_IO4
#define GPIO_OUTPUT_IO_1    ESP32_IO5
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))

#define GPIO_INPUT_IO_0     ESP32_IO18
#define GPIO_INPUT_IO_1     ESP32_IO19
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

int main()
{
    printf("GPIO test - polling ESP32 GPIO pins...\n");

	// Enable input and output on our selected pins
	*GPIO_OUTPUTENABLE = GPIO_OUTPUT_PIN_SEL;
	*GPIO_INPUTENABLE = GPIO_INPUT_PIN_SEL;

	int count = 0;
	uint32_t prevval = 0xFFFFFFFF;
	do
	{
		uint32_t data;
		while (GPIORingBufferRead(&data, sizeof(uint32_t)))
		{
			// Did we receive anything form GPIO9 or GPIO10?
			if (data != prevval)
			{
				prevval = data;

				printf("Inputs: %.8x\n", data);

				// Write suitable output states (4 for 9, 5 for 10)
				uint32_t outval = 0;
				outval |= data&(1ULL<<GPIO_INPUT_IO_0) ? (1ULL<<GPIO_OUTPUT_IO_0) : 0;
				outval |= data&(1ULL<<GPIO_INPUT_IO_1) ? (1ULL<<GPIO_OUTPUT_IO_1) : 0;
				printf("Outputs: %.8x\n", outval);

				*GPIO_DATA = outval;

				++count;
			}
		}
		TaskYield();
	} while (count < 10);

	// Disable input and output on all pins
	*GPIO_OUTPUTENABLE = 0;
	*GPIO_INPUTENABLE = 0;

    return 0;
}
