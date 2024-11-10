/** \file
 * Joystick example
 *
 * \ingroup examples
 * This example demonstrates how to read the joystick position and buttons.
 */

#include "basesystem.h"
#include "core.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

static int32_t *s_jposxy_buttons = (int32_t*)JOYSTICK_POS_AND_BUTTONS;

int main(int argc, char *argv[])
{
	uint32_t prevstate[4] = {0x11,0x11,0x11,0x11};
	while(1)
	{
		if (__builtin_memcmp(prevstate, s_jposxy_buttons, 4*sizeof(uint32_t)))
		{
			__builtin_memcpy(prevstate, s_jposxy_buttons, 4*sizeof(uint32_t));
			printf("X:%X Y:%X B0:%X B1:%X\n", (unsigned int)s_jposxy_buttons[0], (unsigned int)s_jposxy_buttons[1], (unsigned int)s_jposxy_buttons[2], (unsigned int)s_jposxy_buttons[3]);
		}
		TaskYield();
	}

	return 0;
}
