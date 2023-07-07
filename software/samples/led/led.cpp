#include <math.h>
#include "basesystem.h"
#include "leds.h"

int main()
{
	uint32_t state = 0;
	do{
		LEDSetState(state++);
		E32Sleep(250*ONE_MILLISECOND_IN_TICKS);
	} while (1);

	return 0;
}