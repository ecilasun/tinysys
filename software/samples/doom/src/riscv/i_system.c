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
#include "console.h"
#include "usbhost.h"

void
I_Init(void)
{
	I_InitSound();
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
	// Post event for all pressed / released keys
	uint16_t *keystates = (uint16_t*)KEYBOARD_KEYSTATE_BASE;

	for(int i=0; i<255; ++i)
	{
		if (keystates[i])
		{
			uint16_t updown = keystates[i]&3;
			if (updown)
			{
				event_t event;
				if (i==0x4F) // right arrow
					event.data1 = KEY_RIGHTARROW;
				else if (i==0x50) // left arrow
					event.data1 = KEY_LEFTARROW;
				else if (i==0x51) // down arrow
					event.data1 = KEY_DOWNARROW;
				else if (i==0x52) // up arrow
					event.data1 = KEY_UPARROW;
				else if (i==0xE1 || i==0xE5) // left shift / right shift
					event.data1 = KEY_RSHIFT;
				else if (i==0xE2 || i==0xE6) // left alt / right alt
					event.data1 = KEY_RALT;
				else if (i==0xE0 || i==0xE4) // left ctrl / right ctrl
					event.data1 = KEY_RCTRL;
				else
					event.data1 = HIDScanToASCII(i, 0); // always lowercase
				event.type = (updown&1) ? ev_keydown : ev_keyup;
				D_PostEvent(&event);
			}
		}
	}

	// TODO: Joystick support
	//int32_t *s_jposxy_buttons = (int32_t*)JOYSTICK_POS_AND_BUTTONS;
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
