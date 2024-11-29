/*
 * Copyright (C) 2020 Shotaro Uchida <fantom@xmaker.mx>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <quakedef.h>
#include <quakembd.h>
#include <d_local.h>
#include "vpu.h"

static byte *vid_buffer;
static byte *surfcache;

/* Global allocation for the renderer */
unsigned short d_8to16table[256];
unsigned d_8to24table[256];

void VID_SetPalette(unsigned char *palette)
{
	// Copy palette to G-RAM
	byte r, g, b;
	for (int i=0 ; i<256 ; i++) {
		r = (*palette++)>>4;
		g = (*palette++)>>4;
		b = (*palette++)>>4;
		VPUSetPal(i, r, g, b);
	}
}

void VID_Init(unsigned char *palette)
{
	int vid_surfcachesize;
	int width;
	int height;

	width = qembd_get_width();
	height = qembd_get_height();

	vid_buffer = (byte *) Hunk_HighAllocName(width * height * sizeof (byte), "vid_main");
	if (!vid_buffer)
		qembd_error("Not enough memory for video mode (vid_main)");

	d_pzbuffer = (short *) Hunk_HighAllocName(width * height * sizeof (short), "zbuffer");
	if (!d_pzbuffer)
		qembd_error("Not enough memory for video mode (zbuffer)");

	vid.width = vid.conwidth = width;
	vid.height = vid.conheight = height;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = width;

	vid_surfcachesize = D_SurfaceCacheForRes(vid.width, vid.height);
	surfcache = (byte *) Hunk_HighAllocName(vid_surfcachesize, "surfcache");
	if (!surfcache)
		qembd_error("Not enough memory for video mode");

	D_InitCaches(surfcache, vid_surfcachesize);

	qembd_vidinit();
}

void VID_Shutdown(void)
{
}

void VID_Update(vrect_t *rects)
{
	while (rects) {
		qembd_fillrect(vid_buffer, rects->x, rects->y, rects->width, rects->height);
		rects = rects->pnext;
	}
	qembd_refresh();
}

void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect(int x, int y, int width, int height)
{
}
