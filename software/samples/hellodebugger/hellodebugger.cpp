#include "basesystem.h"
#include <stdio.h>
#include "leds.h"

int main()
{
    printf("Hello, debugger!\n");

	uint32_t ledstate = 0;
	while(1)
	{
		// An infinite loop to test the debugger with
		LEDSetState(ledstate);
		ledstate++;
		E32Sleep(TWO_HUNDRED_FIFTY_MILLISECONDS_IN_TICKS);
	}

    return 0;
}
