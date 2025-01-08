/*
 * i_system.c
 *
 * System support code
 *
 * Copyright (C) 1993-1996 by id Software, Inc.
 * Copyright (C) 2021 Sylvain Munaut
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"

#include "d_main.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_sound.h"
#include "i_video.h"

#include "i_system.h"

#include "basesystem.h"
#include "core.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyboard.h"
#include "joystick.h"

void
I_Init(void)
{
	I_InitSound();

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;
}


byte *
I_ZoneBase(int *size)
{
	/* Give 8M to DOOM */
	*size = 8 * 1024 * 1024;
	return (byte *) malloc (*size);
}

// returns time in 1/TICRATEth second tics
int
I_GetTime (void)
{
	return (ClockToMs(E32ReadTime())*TICRATE)/1000;
}

static void
I_GetRemoteEvent(void)
{
	uint8_t byte;
	int have_joystick = 0;
	if (SerialInRingBufferRead(&byte, 1))
	{
		// Keyboard packet
		if (byte == '^')
		{
			uint8_t scandata[KEYBOARD_PACKET_SIZE];
			ReadKeyState(scandata);

			uint8_t scancode = scandata[KEYBOARD_SCANCODE_INDEX];
			uint8_t modifiers_lower = scandata[KEYBOARD_MODIFIERS_LOWER_INDEX];
			uint8_t modifiers_upper = scandata[KEYBOARD_MODIFIERS_UPPER_INDEX];
			uint32_t modifiers = (modifiers_upper << 8) | modifiers_lower;

			// if (ascii == 27) // ESC
			// 	done = 1;

			event_t event;
			// See keyboard scan codes for more info
			switch(scancode)
			{
				case 88:	{ event.data1 = KEY_ENTER; break; }
				case 40:	{ event.data1 = KEY_ENTER; break; }
				case 79:	{ event.data1 = KEY_RIGHTARROW; break; }
				case 80:	{ event.data1 = KEY_LEFTARROW; break; }
				case 81:	{ event.data1 = KEY_DOWNARROW; break; }
				case 82:	{ event.data1 = KEY_UPARROW; break; }
				case 45: 	{ event.data1 = KEY_MINUS; break; }
				case 46:	{ event.data1 = KEY_EQUALS; break; }
				case 41:	{ event.data1 = KEY_ESCAPE; break; }
				case 43:	{ event.data1 = KEY_TAB; break; }
				case 42:	{ event.data1 = KEY_BACKSPACE; break; }
				case 229:	{ event.data1 = KEY_RSHIFT; break; }
				case 228:	{ event.data1 = KEY_RCTRL; break; }
				case 230:	{ event.data1 = KEY_RALT; break; }
				case 226:	{ event.data1 = KEY_LALT; break; }
				case 72:	{ event.data1 = KEY_PAUSE; break; }
				case 58:	{ event.data1 = KEY_F1; break; }
				case 59:	{ event.data1 = KEY_F2; break; }
				case 60:	{ event.data1 = KEY_F3; break; }
				case 61:	{ event.data1 = KEY_F4; break; }
				case 62:	{ event.data1 = KEY_F5; break; }
				case 63:	{ event.data1 = KEY_F6; break; }
				case 64:	{ event.data1 = KEY_F7; break; }
				case 65:	{ event.data1 = KEY_F8; break; }
				case 66:	{ event.data1 = KEY_F9; break; }
				case 67:	{ event.data1 = KEY_F10; break; }
				case 68:	{ event.data1 = KEY_F11; break; }
				case 69:	{ event.data1 = KEY_F12; break; }
				default:	{ event.data1 = KeyboardScanCodeToASCII(scancode, 0); break; }
			}
			event.type = scandata[KEYBOARD_STATE_INDEX]&1 ? ev_keydown : ev_keyup;
			D_PostEvent(&event);
		}

		if (byte == '@')
		{
			uint8_t buttondata[JOYSTICK_BUTTON_PACKET_SIZE];
			ReadButtonState(buttondata);
			ProcessButtonState(buttondata);
			have_joystick = 1;
		}

		if (byte == '%')
		{
			uint8_t axisdata[JOYSTICK_AXIS6_PACKET_SIZE];
			ReadAxisState(axisdata);
			ProcessAxisState(axisdata);
			have_joystick = 1;
		}
	}

	if (have_joystick)
	{
		float axisdata[6];
		uint16_t buttondata;
		JoystickReadState(axisdata, &buttondata);

		event_t event;
		event.type = ev_joystick;
		event.data1 = buttondata;
		event.data2 = (int)axisdata[0];
		event.data3 = (int)axisdata[1];
		D_PostEvent(&event);
	}
}

void
I_StartFrame(void)
{
	/* Nothing to do */
}

void
I_StartTic(void)
{
	I_GetRemoteEvent();
}

ticcmd_t *
I_BaseTiccmd(void)
{
	static ticcmd_t emptycmd;
	return &emptycmd;
}


void
I_Quit(void)
{
	D_QuitNetGame();
	I_ShutdownSound();
	M_SaveDefaults();
	I_ShutdownGraphics();
	exit(0); // NOTE: The environment we're going to return to has been destroyed
}


byte *
I_AllocLow(int length)
{
	byte*	mem;
	mem = (byte *)malloc (length);
	memset (mem,0,length);
	return mem;
}


void
I_Tactile
( int on,
  int off,
  int total )
{
	// UNUSED.
	on = off = total = 0;
}


void
I_Error(char *error, ...)
{
	va_list	argptr;

	// Message first.
	va_start (argptr,error);
	printf ("Error: ");
	vprintf (error, argptr);
	printf ("\n");
	va_end (argptr);

	fflush( stdout );

	// Shutdown. Here might be other errors.
	if (demorecording)
		G_CheckDemoStatus();

	D_QuitNetGame ();
	I_ShutdownGraphics();

	exit(-1);
}
