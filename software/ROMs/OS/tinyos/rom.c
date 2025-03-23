// Boot ROM

#include "basesystem.h"
#include "uart.h"
#include "leds.h"

int main()
{
	while (1)
	{
		LEDSetState(0x3F);
		E32Sleep(ONE_SECOND_IN_TICKS);
		LEDSetState(0x02);
		E32Sleep(ONE_SECOND_IN_TICKS);
	}

	return 0;
} 
