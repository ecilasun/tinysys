/** \file
 * Debug LED example
 *
 * \ingroup examples
 * This example demonstrates how to use the debug LEDs on the board by cycling through all possible states.
 */

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