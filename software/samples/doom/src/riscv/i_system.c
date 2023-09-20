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
	uint16_t updown = 0;

	for(int i=0; i<255; ++i)
	{
		updown = keystates[i]&3;
		if (updown)
		{
			switch(i)
			{
				case HKEY_ENTER:		{ event.data1 = KEY_ENTER; break; }
				case HKEY_RETURN:		{ event.data1 = KEY_ENTER; break; }
				case HKEY_RIGHTARROW:	{ event.data1 = KEY_RIGHTARROW; break; }
				case HKEY_LEFTARROW:	{ event.data1 = KEY_LEFTARROW; break; }
				case HKEY_DOWNARROW:	{ event.data1 = KEY_DOWNARROW; break; }
				case HKEY_UPARROW:		{ event.data1 = KEY_UPARROW; break; }
				case HKEY_RIGHTSHIFT:	{ event.data1 = KEY_RSHIFT; break; }
				case HKEY_LEFTSHIFT:	{ event.data1 = KEY_RSHIFT; break; }
				case HKEY_RIGHTALT:		{ event.data1 = KEY_RALT; break; }
				case HKEY_LEFTALT:		{ event.data1 = KEY_RALT; break; }
				case HKEY_RIGHTCONTROL:	{ event.data1 = KEY_RCTRL; break; }
				case HKEY_LEFTCONTROL:	{ event.data1 = KEY_RCTRL; break; }
				default:				{ event.data1 = KeyScanCodeToASCII(i, 0); break; }
			}
			event.type = (updown&1) ? ev_keydown : ev_keyup;
			D_PostEvent(&event);
		}
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
