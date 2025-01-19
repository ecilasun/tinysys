/*
* Reading the ST-NICCC megademo data stored in
* the SDCard and streaming it to polygons,
* rendered in the framebuffer.
* 
* The polygon stream is a 640K file 
* (C_EXAMPLES/DATA/scene1.dat), that needs to 
* be stored on the SD card. 
*
* More details and links in C_EXAMPLES/DATA/notes.txt
*/

// Please see https://github.com/BrunoLevy/Vectorizer for the original code

#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "dma.h"
#include "io.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cur_byte_address = 0;

FILE *s_fp;
uint8_t *filedata;
EVideoContext s_vctx;
uint8_t *s_framebufferB;
uint8_t *s_framebufferA;
//uint8_t *s_rasterBuffer; // For hardware rasterizer

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

static void clear(uint8_t* buffer)
{
	for(int i=0; i<320*240; ++i)
		buffer[i] = 0x07;
}

void gfx_fillpoly(uint8_t* buffer, int nb_pts, int* points, uint8_t color)
{
    int x_left[256];
    int x_right[256];

    /* Determine clockwise, miny, maxy */
    int clockwise = 0;
    int miny =  1024;
    int maxy = -1024;
    
    for(int i1=0; i1<nb_pts; ++i1)
	{
		int i2=(i1==nb_pts-1) ? 0 : i1+1;
		int i3=(i2==nb_pts-1) ? 0 : i2+1;
		int x1 = points[2*i1];
		int y1 = points[2*i1+1];
		int dx1 = points[2*i2]   - x1;
		int dy1 = points[2*i2+1] - y1;
		int dx2 = points[2*i3]   - x1;
		int dy2 = points[2*i3+1] - y1;
		clockwise += dx1 * dy2 - dx2 * dy1;
		miny = MIN(miny,y1);
		maxy = MAX(maxy,y1);
    }

    /* Determine x_left and x_right for each scaline */
    for(int i1=0; i1<nb_pts; ++i1)
	{
		int i2=(i1==nb_pts-1) ? 0 : i1+1;

		int x1 = points[2*i1];
		int y1 = points[2*i1+1];
		int x2 = points[2*i2];
		int y2 = points[2*i2+1];

		int* x_buffer = ((clockwise > 0) ^ (y2 > y1)) ? x_left : x_right;

        // ensure consistent rasterization of neighboring edges in
        // a triangulation, avoid small gaps
        if(y2 < y1)
		{
            int tmp = y1;
            y1 = y2;
            y2 = tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
        }
        
		int dx = x2 - x1;
		int sx = 1;
		int dy = y2 - y1;
		int sy = 1;
		int x = x1;
		int y = y1;
		int ex;
		
		if(dx < 0)
		{
			sx = -1;
			dx = -dx;
		}
	
		if(dy < 0)
		{
			sy = -1;
			dy = -dy;
		}

		if(y1 == y2)
		{
			x_left[y1]  = MIN(x1,x2);
			x_right[y1] = MAX(x1,x2);
			continue;
		}

		ex = (dx << 1) - dy;

		for(int u=0; u <= dy; ++u)
		{
			x_buffer[y] = x; 
			y += sy;
			while(ex >= 0) {
				x += sx;
				ex -= dy << 1;
			}
			ex += dx << 1;
		}
	}

	for(int y = miny; y <= maxy; ++y)
		for(int x = x_left[y]; x <= x_right[y]; ++x)
			buffer[y * 320 + x] = color;
}

int main(int argc, char** argv)
{
	s_framebufferB = VPUAllocateBuffer(320*240); // Or think of it as 1280*64 for tiles
	s_framebufferA = VPUAllocateBuffer(320*240);
	//s_rasterBuffer = VPUAllocateBuffer(80*60*16); // Rasterizer tile buffer

	//memset(s_rasterBuffer, 0x07, 80*60*16);
	memset(s_framebufferA, 0x07, 320*240);
	memset(s_framebufferB, 0x07, 320*240);

	s_vctx.m_vmode = EVM_320_Wide;
	s_vctx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&s_vctx, EVS_Enable);
	VPUSetDefaultPalette(&s_vctx);

	struct EVideoSwapContext sc;
	sc.cycle = 0;
	sc.framebufferA = s_framebufferA;
	sc.framebufferB = s_framebufferB;
	VPUSwapPages(&s_vctx, &sc);

	char scene_file[512];
	if (argc>=2)
		strcpy(scene_file, argv[1]);
	else
		strcpy(scene_file, "scene1.bin");

	ST_NICCC_IO io;
	ST_NICCC_FRAME frame;
	ST_NICCC_POLYGON polygon;

    if(!st_niccc_open(&io,scene_file,ST_NICCC_READ))
	{
        fprintf(stderr,"could not open data file\n");
        exit(-1);
    }

	for(;;)
	{
        st_niccc_rewind(&io);

		while(st_niccc_read_frame(&io, &frame))
		{
			if(frame.flags & CLEAR_BIT)
				clear(sc.writepage);

			while(st_niccc_read_polygon(&io, &frame, &polygon))
				gfx_fillpoly(sc.writepage, polygon.nb_vertices, polygon.XY, polygon.color);

			VPUWaitVSync();
			VPUSwapPages(&s_vctx, &sc);
		}
	}
}
