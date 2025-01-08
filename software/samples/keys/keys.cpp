/** \file
 * File reception utility for TinyOS
 * \ingroup TinyOS
 * This command should be placed in sys/bin and will be initiated remotely to kick off a file transfer.
 */

#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "serialinringbuffer.h"
#include "keyboard.h"
#include "joystick.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void cleanupComms()
{
	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 0;
}

int main()
{
	atexit(cleanupComms);

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;

	// Wait for UART chatter to finish
	E32Sleep(HUNDRED_MILLISECONDS_IN_TICKS);

	printf("Sample to parse key scancodes from tinyremote app\nPlease press ESC to exit\n");

	int done = 0;
	while(!done)
	{
		uint8_t byte;
		int have_joystick_data = 0;
		// Check for incoming data
		if (SerialInRingBufferRead(&byte, 1))
		{
			if (byte == '^')
			{
				uint8_t scandata[KEYBOARD_PACKET_SIZE];

				// Read the scan code packet
				ReadKeyState(scandata);

				// Emit the ASCII character corresponding to the scan code
				uint8_t ascii = KeyboardScanCodeToASCII(scandata[KEYBOARD_SCANCODE_INDEX], 0);

				uint8_t modifiers_lower = scandata[KEYBOARD_MODIFIERS_LOWER_INDEX];
				uint8_t modifiers_upper = scandata[KEYBOARD_MODIFIERS_UPPER_INDEX];
				uint32_t modifiers = (modifiers_upper << 8) | modifiers_lower;

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
				printf("ScanCode:%02X ASCII:%02X\nState:%s\nModstate:%X\nModifiers:%s\n", scandata[0], ascii, scandata[KEYBOARD_STATE_INDEX] ? "down" : "up", (unsigned int)modifiers, modstring);

				if (ascii == 27) // ESC
					done = 1;
			}
			else if (byte == '@')
			{
				uint8_t buttondata[JOYSTICK_BUTTON_PACKET_SIZE];
				ReadButtonState(buttondata);
				ProcessButtonState(buttondata);
				have_joystick_data = 1;
			}
			else if (byte == '%')
			{
				uint8_t axisdata[JOYSTICK_AXIS6_PACKET_SIZE];
				ReadAxisState(axisdata);
				ProcessAxisState(axisdata);
				have_joystick_data = 1;
			}
			else
			{
				// This is a non-scan code packet
				printf("Non-scan code packet, exiting\n");
				done = 1;
			}
			
			if (have_joystick_data)
			{
				float axisdata[6];
				uint16_t buttondata;
				JoystickReadState(axisdata, &buttondata);

				printf("Axis: ");
				for (int i = 0; i < 6; i++)
					printf("%f ", axisdata[i]);
				printf("\nButtons: %04X\n", buttondata);
			}
		}
	}

	taskctx->interceptUART = 0;

    return 0;
}
