#include "basesystem.h"
#include "gpio.h"
#include <stdio.h>

int main()
{
    printf("GPIO test\n");

	E32Sleep(10000);
	*GPIO_OUTPUTENABLE = 0xFFFFFFFF;

	for (uint32_t i=0; i<10; ++i)
	{
		E32Sleep(10000);
		*GPIO_DATA = 0x00000000;

		E32Sleep(10000);
		*GPIO_DATA = 0x00000001;
	}

	E32Sleep(10000);
	*GPIO_OUTPUTENABLE = 0x00000000;

    return 0;
}
