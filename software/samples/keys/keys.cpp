/** \file
 * File reception utility for TinyOS
 * \ingroup TinyOS
 * This command should be placed in sys/bin and will be initiated remotely to kick off a file transfer.
 */

#include "basesystem.h"
#include "core.h"
#include "serialinringbuffer.h"
#include "keyboard.h"
#include "joystick.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	// Wait for UART chatter to finish
	E32Sleep(HUNDRED_MILLISECONDS_IN_TICKS);

	printf("Sample to parse key scancodes from tinyremote app\nPlease press ESC to exit\n");

	int done = 0;
	uint32_t oldcountKey = 0xAAAABBBB;
	uint32_t oldcountJoystick = 0xAAAABBBB;
	while(!done)
	{
		volatile struct SKeyboardState* keyState = KeyboardGetState();
		volatile struct SJoystickState* joystickState = JoystickGetState();

		uint8_t ascii = keyState->ascii;

		// Did key state change?
		if (oldcountKey != keyState->count && keyState->ascii != 0x0)
		{
			oldcountKey = keyState->count;

			uint32_t modifiers = keyState->modifiers;

			char modstring[512] = {0};
			if (modifiers & 0x0001)
				strcat(modstring, "left shift ");
			if (modifiers & 0x0002)
				strcat(modstring, "right shift ");
			if (modifiers & 0x0040)
				strcat(modstring, "left ctrl ");
			if (modifiers & 0x0080)
				strcat(modstring, "right ctrl ");
			if (modifiers & 0x0100)
				strcat(modstring, "left alt ");
			if (modifiers & 0x0200)
				strcat(modstring, "right alt ");
			if (modifiers & 0x0400)
				strcat(modstring, "left gui ");
			if (modifiers & 0x0800)
				strcat(modstring, "right gui ");
			if (modifiers & 0x1000)
				strcat(modstring, "num lock ");
			if (modifiers & 0x2000)
				strcat(modstring, "caps lock ");
			if (modifiers & 0x4000)
				strcat(modstring, "mode ");
			if (modifiers & 0x8000)
				strcat(modstring, "scroll lock ");
			printf("#%d:ScanCode:%02X ASCII:%02X\nState:%s\nModstate:%X\nModifiers:%s\n", keyState->count, keyState->scancode, ascii, keyState->state ? "down" : "up", (unsigned int)modifiers, modstring);
		}

		if (ascii == 27) // ESC
			done = 1;

		if (oldcountJoystick != joystickState->count)
		{
			oldcountJoystick = joystickState->count;

			printf("Axis: ");
			for (int i = 0; i < 6; i++)
				printf("%f ", joystickState->axis[i]);
			printf("\nButtons: %04X\n", joystickState->buttons);
		}
	}

    return 0;
}
