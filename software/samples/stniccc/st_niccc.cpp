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

#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cur_byte_address = 0;

FILE *s_fp;
uint8_t *filedata;
EVideoContext s_vctx;
uint8_t *s_framebufferB;
uint8_t *s_framebufferA;
uint8_t *s_rasterBuffer;

static uint8_t next_byte(void)
{
	uint8_t result;
	// fread(&result, 1, 1, s_fp);
	result = filedata[cur_byte_address];
	++cur_byte_address;
	return result;
}

static uint16_t next_word(void)
{
/* In the ST-NICCC file,  
* words are stored in big endian format.
* (see DATA/scene_description.txt).
*/
	uint16_t hi = (uint16_t)next_byte();    
	uint16_t lo = (uint16_t)next_byte();
	return (hi << 8) | lo;
}

static inline void map_vertex(int16_t* X, int16_t* Y)
{
	*X = *X << 1; // for a larger picture (but too slow)
	*Y = *Y << 1;
}

static void clear(void) {
	SPrimitive primA;
	primA.x0 = 0;
	primA.y0 = 0;
	primA.x1 = 0;
	primA.y1 = 200;
	primA.x2 = 256;
	primA.y2 = 200;
	RPUPushPrimitive(&prim);
	RPUSetColor(0);
	RPURasterizePrimitive();

	prim.x0 = 256;
	prim.y0 = 200;
	prim.x1 = 256;
	prim.y1 = 0;
	prim.x2 = 0;
	prim.y2 = 0;
	RPUPushPrimitive(&prim);
	RPUSetColor(0);
	RPURasterizePrimitive();
}

/* 
* Polygon color map
*/
uint32_t cmap[16];

/*
* Current frame's vertices coordinates (if frame is indexed).
*/
int16_t  X[255];
int16_t  Y[255];

/*
* Current polygon vertices, lain out as
* xi = poly[2*i], yi = poly[2*i+1]
*/
int16_t   poly[30];

/*
* Masks for frame flags.
*/
#define CLEAR_BIT   1
#define PALETTE_BIT 2
#define INDEXED_BIT 4

/*
* Reads a frame's polygonal description from
* file and rasterizes the polygons using
* FemtoGL.
* returns 0 if last frame.
*   See DATA/scene_description.txt for the 
* ST-NICCC file format.
*   See DATA/test_ST_NICCC.c for an example
* program.
*/
static int read_frame(void)
{
	uint8_t frame_flags = next_byte();

	// Update palette data.
	if(frame_flags & PALETTE_BIT)
	{
		uint16_t colors = next_word();
		for(int b=15; b>=0; --b)
		{
			if(colors & (1 << b))
			{
				int rgb = next_word();
				// Get the three 3-bits per component R,G,B
				int b3 = (rgb & 0x007);
				int g3 = (rgb & 0x070) >> 4;
				int r3 = (rgb & 0x700) >> 8;

				// Re-encode them as 24bpp color
				cmap[15-b] = (uint8_t)((b3<<6) | (g3<<3) | r3);
			}
		}
	}

	if(frame_flags & CLEAR_BIT)
		clear(); 

	// Update vertices
	if(frame_flags & INDEXED_BIT)
	{
		uint8_t nb_vertices = next_byte();
		for(int v=0; v<nb_vertices; ++v)
		{
			X[v] = next_byte();
			Y[v] = next_byte();
			//map_vertex(&X[v],&Y[v]);
		}
	}

	// Draw frame's polygons
	for(;;)
	{
		uint8_t poly_desc = next_byte();

		// Special polygon codes (end of frame,
		// seek next block, end of stream)

		if(poly_desc == 0xff)
			break; // end of frame

		if(poly_desc == 0xfe)
		{
			// Go to next 64kb block
			// (TODO: with fseek !)
			while(cur_byte_address & 65535)
				next_byte();
			return 1; 
		}

		if(poly_desc == 0xfd)
			return 0; // end of stream

		uint8_t nvrtx = poly_desc & 15;
		uint8_t poly_col = poly_desc >> 4;
		for(int i=0; i<nvrtx; ++i)
		{
			if(frame_flags & INDEXED_BIT)
			{
				uint8_t index = next_byte();
				poly[2*i]   = X[index];
				poly[2*i+1] = Y[index];
			}
			else
			{
				int16_t x,y;
				x = next_byte();
				y = next_byte();
				//map_vertex(&x,&y);
				poly[2*i]   = x;
				poly[2*i+1] = y;
			}
		}

		// Decompose into a triangle fan and rasterize
		for (int i=0;i<nvrtx-2;++i)
		{
			SPrimitive prim;
			prim.x0 = poly[2*0];
			prim.y0 = poly[2*0+1];
			prim.x1 = poly[2*(i+2)];
			prim.y1 = poly[2*(i+2)+1];
			prim.x2 = poly[2*(i+1)];
			prim.y2 = poly[2*(i+1)+1];

			RPUPushPrimitive(&prim);
			RPUSetColor(cmap[poly_col]);
			RPURasterizePrimitive();
		}
	}
	return 1; 
}

int main(int argc, char** argv)
{
	s_framebufferB = GPUAllocateBuffer(320*240); // Or think of it as 1280*64 for tiles
	s_framebufferA = GPUAllocateBuffer(320*240);
	s_rasterBuffer = GPUAllocateBuffer(80*60*16); // Rasterizer tile buffer

	memset(s_rasterBuffer, 0x07, 80*60*16);
	memset(s_framebufferA, 0x07, 320*240);
	memset(s_framebufferB, 0x07, 320*240);

	s_vctx.m_vmode = EVM_320_Wide;
	s_vctx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&s_vctx, EVS_Enable);
	GPUSetWriteAddress(&s_vctx, (uint32_t)s_framebufferA);
	GPUSetScanoutAddress(&s_vctx, (uint32_t)s_framebufferB);
	GPUSetDefaultPalette(&s_vctx);
	RPUSetTileBuffer((uint32_t)s_rasterBuffer);

	int target = 0;
	for (int b=0;b<4;++b)
		for (int g=0;g<8;++g)
			for (int r=0;r<8;++r)
				GPUSetPal(target++, r*36, g*36, b*85);

	while(1)
	{
		cur_byte_address = 0;
		s_fp = fopen("sd:/scene1.dat", "rb");
		if (!s_fp)
		{
			printf("Could not open scene1.dat\n");
			return -1;
		}

		// Read the whole thing into memory
		{
			fpos_t pos, endpos;
			fgetpos(s_fp, &pos);
			fseek(s_fp, 0, SEEK_END);
			fgetpos(s_fp, &endpos);
			fsetpos(s_fp, &pos);
			uint32_t fsize = (uint32_t)endpos;
			filedata = (uint8_t*)malloc(fsize);
			fread(filedata, 1, fsize, s_fp);
			fclose(s_fp);
		}

		uint32_t cycle = 0;
		int res = 0;
		do
		{
			uint8_t *readpage = (cycle%2) ? s_framebufferA : s_framebufferB;
			uint8_t *writepage = (cycle%2) ? s_framebufferB : s_framebufferA;

			res = read_frame();

			// Wait for vsync
			uint32_t prevvsync = GPUReadVBlankCounter();
			uint32_t currentvsync;
			do {
				currentvsync = GPUReadVBlankCounter();
			} while (currentvsync == prevvsync);

			// Swap buffers
			GPUSetWriteAddress(&s_vctx, (uint32_t)writepage);
			GPUSetScanoutAddress(&s_vctx, (uint32_t)readpage);

			// Wait for all raster work to finish
			RPUWait();

			// Get the DMA unit to resolve and write output onto the current GPU write page
			DMAResolveTiles((uint32_t)s_rasterBuffer, (uint32_t)writepage);

			++cycle;
		} while(res);
	}
}
