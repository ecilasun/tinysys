#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include <random>

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;
static uint8_t *s_rasterBuffer;
 
static inline int32_t max(int32_t x, int32_t y) { return x>y?x:y; }
static inline int32_t min(int32_t x, int32_t y) { return x<y?x:y; }

struct sVec2
{
	int32_t x, y;
};

/*int32_t edgeFunction(const sVec2 &v0, const sVec2 &v1, const sVec2 &p)
{
	// Same as what our hardware does
	int32_t A = (p.y - v0.y);
	int32_t B = (p.x - v0.x);
	int32_t dx = (v0.x - v1.x);
	int32_t dy = (v1.y - v0.y);
	return A*dx + B*dy;
}*/

void edgeMask(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t tx, int32_t ty, uint32_t *rmask)
{
	// edge 0
	int32_t dx = (x0 - x1);
	int32_t dy = (y1 - y0);
	int32_t A0 = (ty - y0);
	int32_t B0 = (tx - x0);

	int32_t A1 = A0;
	int32_t B1 = B0+1;
	int32_t A2 = A0;
	int32_t B2 = B0+2;
	int32_t A3 = A0;
	int32_t B3 = B0+3;

	int32_t A4 = A0+1;
	int32_t B4 = B0;
	int32_t A5 = A0+1;
	int32_t B5 = B0+1;
	int32_t A6 = A0+1;
	int32_t B6 = B0+2;
	int32_t A7 = A0+1;
	int32_t B7 = B0+3;

	int32_t A8  = A0+2;
	int32_t B8  = B0;
	int32_t A9  = A0+2;
	int32_t B9  = B0+1;
	int32_t A10 = A0+2;
	int32_t B10 = B0+2;
	int32_t A11 = A0+2;
	int32_t B11 = B0+3;

	int32_t A12 = A0+3;
	int32_t B12 = B0;
	int32_t A13 = A0+3;
	int32_t B13 = B0+1;
	int32_t A14 = A0+3;
	int32_t B14 = B0+2;
	int32_t A15 = A0+3;
	int32_t B15 = B0+3;

	int32_t R0  = B0*dy;
	int32_t R1  = B1*dy;
	int32_t R2  = B2*dy;
	int32_t R3  = B3*dy;
	int32_t R4  = B4*dy;
	int32_t R5  = B5*dy;
	int32_t R6  = B6*dy;
	int32_t R7  = B7*dy;
	int32_t R8  = B8*dy;
	int32_t R9  = B9*dy;
	int32_t R10 = B10*dy;
	int32_t R11 = B11*dy;
	int32_t R12 = B12*dy;
	int32_t R13 = B13*dy;
	int32_t R14 = B14*dy;
	int32_t R15 = B15*dy;

	R0  += A0*dx;
	R1  += A1*dx;
	R2  += A2*dx;
	R3  += A3*dx;
	R4  += A4*dx;
	R5  += A5*dx;
	R6  += A6*dx;
	R7  += A7*dx;
	R8  += A8*dx;
	R9  += A9*dx;
	R10 += A10*dx;
	R11 += A11*dx;
	R12 += A12*dx;
	R13 += A13*dx;
	R14 += A14*dx;
	R15 += A15*dx;

	// Imitate byte write mask + 128bit write in hardware
	rmask[0] = (R15<0?0:0xFF000000) | (R14<0?0:0x00FF0000) | (R13<0?0:0x0000FF00) | (R12<0?0:0x000000FF);
	rmask[1] = (R11<0?0:0xFF000000) | (R10<0?0:0x00FF0000) | (R9<0?0:0x0000FF00)  | (R8<0?0:0x000000FF);
	rmask[2] = (R7<0?0:0xFF000000)  | (R6<0?0:0x00FF0000)  | (R5<0?0:0x0000FF00)  | (R4<0?0:0x000000FF);
	rmask[3] = (R3<0?0:0xFF000000)  | (R2<0?0:0x00FF0000)  | (R1<0?0:0x0000FF00)  | (R0<0?0:0x000000FF);
}

int main(int argc, char *argv[])
{
	printf("Hardware rasterization test\n");

	s_framebufferB = GPUAllocateBuffer(320*240); // Or think of it as 1280*64 for tiles
	s_framebufferA = GPUAllocateBuffer(320*240);

	struct EVideoContext vx;
	vx.m_vmode = EVM_320_Wide;
	vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);
	GPUSetDefaultPalette(&vx);

	s_rasterBuffer = GPUAllocateBuffer(80*60*16); // Tile buffer

	// Rasterizer output buffer
	// Ideally this is not the current write page,
	// and gets DMA-untiled onto it once all output is done
	RPUSetTileBuffer((uint32_t)s_rasterBuffer);

	memset(s_framebufferA, 0x00, 320*240); // Clear to black
	memset(s_framebufferB, 0x00, 320*240);

	// Make sure CPU writes are visible in memory
	CFLUSH_D_L1;

	float R = 0.f;

	struct EVideoSwapContext sc;
	sc.cycle = 0;
	sc.framebufferA = s_framebufferA;
	sc.framebufferB = s_framebufferB;
	GPUSwapPages(&vx, &sc);

	while (1)
	{
		if (argc > 1)
		{
			memset(s_rasterBuffer, 0x07, 80*60*16); // Clear to gray
			CFLUSH_D_L1;

			// Single rotating primitive
			SPrimitive prim;

			float X0 = cosf(R)*180.f;
			float Y0 = sinf(R)*180.f;
			float X1 = cosf(R+1.57079633f)*180.f; // 90 degrees
			float Y1 = sinf(R+1.57079633f)*180.f;
			float X2 = cosf(R+3.92699082f)*180.f; // 225 degrees
			float Y2 = sinf(R+3.92699082f)*180.f;

			prim.x0 = 160 + X0;
			prim.y0 = 120 + Y0;
			prim.x1 = 160 + X1;
			prim.y1 = 120 + Y1;
			prim.x2 = 160 + X2;
			prim.y2 = 120 + Y2;

			RPUPushPrimitive(&prim);
			RPUSetColor(0x00); // Black triangle
			RPURasterizePrimitive();

			R += 0.01f;
		}
		else
		{
			for (int i=0; i<512; ++i)
			{
				// Enforce primitives larger than the screen size to test clipping
				SPrimitive prim;
				int16_t x = rand()%320;
				int16_t y = rand()%240;
				prim.x0 = x+(rand()%16);
				prim.y0 = y+(rand()%16);
				prim.x1 = x+(rand()%16);
				prim.y1 = y+(rand()%16);
				prim.x2 = x+(rand()%16);
				prim.y2 = y+(rand()%16);
				uint8_t V = rand()%255;

				RPUPushPrimitive(&prim);
				RPUSetColor(V);
				RPURasterizePrimitive();
			}
		}

		// Make sure to flush rasterizer cache to raster memory before it's read
		RPUFlushCache();
		RPUInvalidateCache();

		// Wait for all raster work to finish
		RPUBarrier();
		RPUWait();

		// Get the DMA unit to resolve and write output onto the current GPU write page
		DMAResolveTiles((uint32_t)s_rasterBuffer, (uint32_t)sc.writepage);

		GPUWaitVSync();
		GPUSwapPages(&vx, &sc);
	}

	return 0;
}
