/*
 * i_video.c
 *
 * Video system support code
 *
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../doomdef.h"

#include "../i_system.h"
#include "../v_video.h"
#include "../i_video.h"

#include "basesystem.h"
#include "core.h"
#include "vpu.h"

uint8_t *framebuffer;
struct EVideoContext g_vctx;

void
I_InitGraphics(void)
{
	usegamma = 1;

	// Allocate 40 pixel more since out video output is 240 vs the default 200 pixels here
	framebuffer = VPUAllocateBuffer(SCREENWIDTH*(SCREENHEIGHT+40));
	memset(framebuffer, 0x0, SCREENWIDTH*(SCREENHEIGHT+40));

	g_vctx.m_cmode = ECM_8bit_Indexed;
	g_vctx.m_vmode = EVM_320_Wide;
	VPUSetVMode(&g_vctx, EVS_Enable);
	VPUSetWriteAddress(&g_vctx, (uint32_t)framebuffer);
	VPUSetScanoutAddress(&g_vctx, (uint32_t)framebuffer);
}

void
I_ShutdownGraphics(void)
{
	VPUSetVMode(&g_vctx, EVS_Disable);
}

void
I_SetPalette(byte* palette)
{
	// Copy palette to G-RAM
	byte r, g, b;
	for (int i=0 ; i<256 ; i++) {
		r = gammatable[usegamma][*palette++]>>4;
		g = gammatable[usegamma][*palette++]>>4;
		b = gammatable[usegamma][*palette++]>>4;
		VPUSetPal(i, r, g, b);
	}
}


void
I_UpdateNoBlit(void)
{
	// hmm....
}

void
I_FinishUpdate (void)
{
	// Copy screen to framebuffer
	memcpy(framebuffer, screens[0], SCREENWIDTH*SCREENHEIGHT);
	// Write pending data to memory to ensure all writes are visible to scan-out
	CFLUSH_D_L1;
}


void
I_WaitVBL(int count)
{
	// Wait until we exit current frame's vbcounter and enter the next one
	VPUWaitVSync();
}

void
I_ReadScreen(byte* scr)
{
	// Copy what's on screen
	memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}
