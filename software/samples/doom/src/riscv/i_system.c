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
#include "keyboard.h"

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
	// Do not read same input twice
	static uint32_t prevGen = 0xFFFFFFFF;
	uint32_t keyGen = GetKeyStateGeneration();
	if (keyGen == prevGen)
		return;
	prevGen = keyGen;

	// Post event for all pressed / released keys
	uint16_t *keystates = GetKeyStateTable();

	event_t event;
	int key = HKEY_RESERVED;
	uint16_t updown = 0;
	if ((updown = keystates[HKEY_ENTER]&3)!=0)			{ event.data1 = KEY_ENTER; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_ENTER; }
	if ((updown = keystates[HKEY_RETURN]&3)!=0)			{ event.data1 = KEY_ENTER; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_RETURN; }
	if ((updown = keystates[HKEY_RIGHTARROW]&3)!=0)		{ event.data1 = KEY_RIGHTARROW; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_RIGHTARROW; }
	if ((updown = keystates[HKEY_LEFTARROW]&3)!=0)		{ event.data1 = KEY_LEFTARROW; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_LEFTARROW; }
	if ((updown = keystates[HKEY_DOWNARROW]&3)!=0)		{ event.data1 = KEY_DOWNARROW; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_DOWNARROW; }
	if ((updown = keystates[HKEY_UPARROW]&3)!=0)		{ event.data1 = KEY_UPARROW; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_UPARROW; }
	if ((updown = keystates[HKEY_RIGHTSHIFT]&3)!=0)		{ event.data1 = KEY_RSHIFT; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_RIGHTSHIFT; }
	if ((updown = keystates[HKEY_LEFTSHIFT]&3)!=0)		{ event.data1 = KEY_RSHIFT; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_LEFTSHIFT; }
	if ((updown = keystates[HKEY_RIGHTALT]&3)!=0)		{ event.data1 = KEY_RALT; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_RIGHTALT; }
	if ((updown = keystates[HKEY_LEFTALT]&3)!=0)		{ event.data1 = KEY_RALT; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_LEFTALT; }
	if ((updown = keystates[HKEY_RIGHTCONTROL]&3)!=0)	{ event.data1 = KEY_RCTRL; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_RIGHTCONTROL; }
	if ((updown = keystates[HKEY_LEFTCONTROL]&3)!=0)	{ event.data1 = KEY_RCTRL; event.type = (updown&1) ? ev_keydown : ev_keyup; key = HKEY_LEFTCONTROL; }

	// None of the above, check for characters
	if (key == HKEY_RESERVED)
	{
		for(int i=0; i<255; ++i)
		{
			if (keystates[i])
			{
				updown = keystates[i]&3;
				event.data1 = KeyScanCodeToASCII(i, 0); // always lowercase
				event.type = (updown&1) ? ev_keydown : ev_keyup;
				key = i;
			}
		}
	}

	if (key != HKEY_RESERVED)
		D_PostEvent(&event);
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
