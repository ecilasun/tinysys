// Boot ROM

#include "rvcrt0.h"
#include "leds.h"
#include "basesystem.h"

int main()
{
	while (1)
	{
		// TODO: Main ROM loop

		LEDSetState(0xF);
		E32Sleep(ONE_SECOND_IN_TICKS);

		LEDSetState(0x0);
		E32Sleep(ONE_SECOND_IN_TICKS);
	}

	return 0;
}
