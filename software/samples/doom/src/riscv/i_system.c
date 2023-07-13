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
#include "xadc.h"

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
	static uint32_t latch = 0;
	event_t event;

	// Read the data on ADC port
	// TODO: This will read individual x/y/button channels
	uint32_t val = ANALOGINPUTS[0];
	if (val < 0x20 && !latch) // 5V and not latched yet
	{
		// Once only until we release the 'key'
		latch = 1;
		event.data1 = KEY_RCTRL; // key_fire
		D_PostEvent(&event);

		/*{"key_right",&key_right, KEY_RIGHTARROW},
		{"key_left",&key_left, KEY_LEFTARROW},
		{"key_up",&key_up, KEY_UPARROW},
		{"key_down",&key_down, KEY_DOWNARROW},
		{"key_strafeleft",&key_strafeleft, ','},
		{"key_straferight",&key_straferight, '.'},
		{"key_fire",&key_fire, KEY_RCTRL},
		{"key_use",&key_use, ' '},
		{"key_strafe",&key_strafe, KEY_RALT},
		{"key_speed",&key_speed, KEY_RSHIFT},*/
	}

	// Unlatch when key's released (TODO: can implement auto-repeat by also checking elapsed time)
	if (val > 0x20)
		latch = 0;

	/*event_t event;

	// Any pending keyboard events to handle?
	// NOTE: OS feeds keyboard input to us from an ISR
	swap_csr(mie, MIP_MSIP | MIP_MTIP);
	int updown = UARTRead();
	int R = UARTRead();
	swap_csr(mie, MIP_MSIP | MIP_MEIP | MIP_MTIP);
	if (R)
	{
		uint32_t key = R&0xFF;

		// Check break/make bit
		event.type = updown ? ev_keydown : ev_keyup;// TODO: (val & PS2KEYMASK_BREAKCODE) ? ev_keyup : ev_keydown;
		switch(key)
		{
			case 0x74: event.data1 = KEY_RIGHTARROW; break;
			case 0x6B: event.data1 = KEY_LEFTARROW; break;
			case 0x75: event.data1 = KEY_UPARROW; break;
			case 0x72: event.data1 = KEY_DOWNARROW; break;
			case 0x76: event.data1 = KEY_ESCAPE; break;
			case 0x5A: event.data1 = KEY_ENTER; break;
			case 0x0D: event.data1 = KEY_TAB; break;
			case 0x05: event.data1 = KEY_F1; break;
			case 0x06: event.data1 = KEY_F2; break;
			case 0x04: event.data1 = KEY_F3; break;
			case 0x0C: event.data1 = KEY_F4; break;
			case 0x03: event.data1 = KEY_F5; break;
			case 0x0B: event.data1 = KEY_F6; break;
			case 0x83: event.data1 = KEY_F7; break;
			case 0x0A: event.data1 = KEY_F8; break;
			case 0x01: event.data1 = KEY_F9; break;
			case 0x09: event.data1 = KEY_F10; break;
			case 0x78: event.data1 = KEY_F11; break;
			case 0x07: event.data1 = KEY_F12; break;
			case 0x66: event.data1 = KEY_BACKSPACE; break;
			//case 0x00: event.data1 = KEY_PAUSE; ? break;
			case 0x55: event.data1 = KEY_EQUALS; break;
			case 0x4E: event.data1 = KEY_MINUS; break;
			case 0x59: event.data1 = KEY_RSHIFT; break;
			case 0x14: event.data1 = KEY_RCTRL; break;
			//case 0x11: event.data1 = KEY_RALT; break; // 0xE0+0x11
			case 0x11: event.data1 = KEY_LALT; break;
			default: event.data1 = key;// TODO: PS2ScanToASCII(key, false); break; // Always lowercase
		}

		D_PostEvent(&event);
	}*/
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
