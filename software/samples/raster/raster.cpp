#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include <stdio.h>
#include <string.h>
#include <random>

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;

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
	if (argc <= 1)
		printf("Software rasterization test\n");
	else
		printf("Hardware rasterization test\n");

	s_framebufferB = GPUAllocateBuffer(320*240); // Or think of it as 1280*64 for tiles
	s_framebufferA = GPUAllocateBuffer(320*240);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);
	GPUSetWriteAddress(&vx, (uint32_t)s_framebufferA);
	GPUSetScanoutAddress(&vx, (uint32_t)s_framebufferB);
	GPUSetDefaultPalette(&vx);

	if (argc <= 1)
	{
		uint32_t cycle = 0;
		while (1)
		{
			uint8_t *readpage = (cycle%2) ? s_framebufferA : s_framebufferB;
			uint8_t *writepage = (cycle%2) ? s_framebufferB : s_framebufferA;

			GPUSetWriteAddress(&vx, (uint32_t)writepage);
			GPUSetScanoutAddress(&vx, (uint32_t)readpage);

			GPUClearScreen(&vx, 0x07070707); // Gray for visibility

			uint64_t starttime = E32ReadTime();
			for (int i=0; i<64; ++i)
			{
				SPrimitive prim;
				prim.x0 = rand()%320;
				prim.y0 = rand()%240;
				prim.x1 = rand()%320;
				prim.y1 = rand()%240;
				prim.x2 = rand()%320;
				prim.y2 = rand()%240;
				uint8_t V = rand()%256;
				uint32_t wV = (V<<24)|(V<<16)|(V<<8)|(V);

				int32_t minx = min(prim.x0, min(prim.x1, prim.x2))/4;
				int32_t miny = min(prim.y0, min(prim.y1, prim.y2))/4;
				int32_t maxx = max(prim.x0, max(prim.x1, prim.x2))/4;
				int32_t maxy = max(prim.y0, max(prim.y1, prim.y2))/4;

				for (int32_t j = miny; j < maxy; ++j)
				{
					for (int32_t i = minx; i < maxx; ++i)
					{
						// 128bit aligned address for 16 pixels' worth of output
						// Hardware generates 4x4 pixel masks but the output is
						// linear in memory so it's going to be 16 pixels, packed side by side
						// on the same scanline if viewed as raw output.
						// Here we rewind and start each triangle from offset zero
						// for debug purposes.
						uint32_t tileIndex = i+j*80;
						uint32_t tileAddress = tileIndex*16;
						uint32_t *rasterout = (uint32_t*)(writepage + tileAddress);

						uint32_t mask0[4], mask1[4], mask2[4];
						edgeMask(prim.x1, prim.y1, prim.x0, prim.y0, i*4, j*4, mask0);
						edgeMask(prim.x2, prim.y2, prim.x1, prim.y1, i*4, j*4, mask1);
						edgeMask(prim.x0, prim.y0, prim.x2, prim.y2, i*4, j*4, mask2);

						uint32_t m0 = mask0[0] & mask1[0] & mask2[0];
						uint32_t m1 = mask0[1] & mask1[1] & mask2[1];
						uint32_t m2 = mask0[2] & mask1[2] & mask2[2];
						uint32_t m3 = mask0[3] & mask1[3] & mask2[3];

						// Imitate byte write mask + 128bit write in hardware
						// One problem here is that we don't have a true write mask
						// so we have to resort to read-write
						if (m0|m1|m2|m3)
						{
							rasterout[0] = ((~m0)&rasterout[0]) | (m0&wV);
							rasterout[1] = ((~m1)&rasterout[1]) | (m1&wV);
							rasterout[2] = ((~m2)&rasterout[2]) | (m2&wV);
							rasterout[3] = ((~m3)&rasterout[3]) | (m3&wV);
						}
					}
				}
			}
			uint64_t endtime = E32ReadTime();

			printf("Raster time: %ld ms\n", ClockToMs(endtime-starttime));

			// Make sure CPU writes are visible
			CFLUSH_D_L1;

			++cycle;
		}
	}
	else
	{
		uint32_t cycle = 0;
		memset(s_framebufferA, 0x07, 320*240); // Gray
		memset(s_framebufferB, 0x07, 320*240);

		// Make sure CPU writes are visible
		CFLUSH_D_L1;

		while (1)
		{
			uint8_t *readpage = (cycle%2) ? s_framebufferA : s_framebufferB;
			uint8_t *writepage = (cycle%2) ? s_framebufferB : s_framebufferA;

			// Simple graphics output page
			GPUSetWriteAddress(&vx, (uint32_t)writepage);
			GPUSetScanoutAddress(&vx, (uint32_t)readpage);

			// Rasterizer output buffer
			// Ideally this is not the current write page,
			// and gets DMA-untiled onto it once all output is done
			RPUSetTileBuffer((uint32_t)writepage);

			GPUClearScreen(&vx, 0x07070707); // Gray for visibility

			uint64_t starttime = E32ReadTime();
			for (int i=0; i<64; ++i)
			{
				SPrimitive prim;
				prim.x0 = rand()%320;
				prim.y0 = rand()%240;
				prim.x1 = rand()%320;
				prim.y1 = rand()%240;
				prim.x2 = rand()%320;
				prim.y2 = rand()%240;
				uint8_t V = rand()%255;

				RPUPushPrimitive(&prim);
				RPUSetColor(V);
				RPURasterizePrimitive();
			}
			uint64_t endtime = E32ReadTime();

			printf("Raster time: %ld ms\n", ClockToMs(endtime-starttime));

			++cycle;
		}
	}

	return 0;
}
