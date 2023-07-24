#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include <stdio.h>
#include <string.h>
#include <random>

// NOTE: This is a really slow way to rasterize
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-practical-implementation.html

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;

static inline int32_t max(int32_t x, int32_t y) { return x>y?x:y; }
static inline int32_t min(int32_t x, int32_t y) { return x<y?x:y; }

struct sVec2
{
	int32_t x, y;
};

int32_t edgeFunction(const sVec2 &v0, const sVec2 &v1, const sVec2 &p)
{
	// Same as what our hardware does
	int32_t A = (p.y - v0.y);
	int32_t B = (p.x - v0.x);
	int32_t dx = (v0.x - v1.x);
	int32_t dy = (v1.y - v0.y);
	return A*dx + B*dy;
}

void raster(uint8_t* _fb, const sVec2 &v0, const sVec2 &v1, const sVec2 &v2)
{
	int32_t minx = max(0, min(v0.x, min(v1.x, v2.x)));
	int32_t miny = max(0, min(v0.y, min(v1.y, v2.y)));
	int32_t maxx = min(319, max(v0.x, max(v1.x, v2.x)));
	int32_t maxy = min(239, max(v0.y, max(v1.y, v2.y)));

	uint8_t V = rand()%255;

	//int32_t attr0 = vertex 0 attribute
	//int32_t attr1 = vertex 1 attribute
	//int32_t attr2 = vertex 2 attribute

	//int32_t area = edgeFunction(v0, v1, v2);

	// Evaluate the gradient once outside
	/*sVec2 topleft = {minx,miny};
	sVec2 topleftpX = {minx+1,miny};
	sVec2 topleftpY = {minx,miny+1};
	int32_t w0 = edgeFunction(v1, v2, topleft);
	int32_t w1 = edgeFunction(v2, v0, topleft);
	int32_t w2 = edgeFunction(v0, v1, topleft);
	int32_t dx12 = edgeFunction(v1, v2, topleftpX) - w0;
	int32_t dy12 = edgeFunction(v1, v2, topleftpY) - w0;
	int32_t dx20 = edgeFunction(v2, v0, topleftpX) - w1;
	int32_t dy20 = edgeFunction(v2, v0, topleftpY) - w1;
	int32_t dx01 = edgeFunction(v0, v1, topleftpX) - w2;
	int32_t dy01 = edgeFunction(v0, v1, topleftpY) - w2;

	for each X step, add dx12 to w0 (and respectively w1 and w2)
	at the end of the line, reset w0 to start value+line*dy12 to
	*/

	for (int32_t j = miny; j < maxy; ++j)
	{
		for (int32_t i = minx; i < maxx; ++i)
		{
			sVec2 p = {i,j};//{i + 0.5f, j + 0.5f};
			int32_t w0 = edgeFunction(v1, v2, p);
			int32_t w1 = edgeFunction(v2, v0, p);
			int32_t w2 = edgeFunction(v0, v1, p);
			// Hardware only checks sign bit, same deal here
			// Eventually it'll calculate the gradients via
			// gx=edge(x+1,y)-edge(x,y)
			// and
			// gy=edge(x,y+1)-edge(x,y)
			// once for the top left corner
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				/*w0 /= area;
				w1 /= area;
				w2 /= area;
				int32_t ipolAttr = w0 * attr0 + w1 * attr1 + w2 * attr2;*/
				_fb[i+j*320] = V;
			}
		}
	}
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

			for (int i=0;i<32;++i)
			{
				sVec2 v0 = sVec2{rand()%256, rand()%256};
				sVec2 v1 = sVec2{rand()%256, rand()%256};
				sVec2 v2 = sVec2{rand()%256, rand()%256};
				raster(writepage, v0, v1, v2);
			}

			// Make sure writes are visible
			CFLUSH_D_L1;

			++cycle;
		}
	}
	else
	{
		uint32_t cycle = 0;
		memset(s_framebufferA, 0x07, 320*240); // Gray
		memset(s_framebufferB, 0x07, 320*240);
		CFLUSH_D_L1;

		while (1)
		{
			uint8_t *readpage = (cycle%2) ? s_framebufferA : s_framebufferB;
			uint8_t *writepage = (cycle%2) ? s_framebufferB : s_framebufferA;

			GPUSetWriteAddress(&vx, (uint32_t)writepage);
			GPUSetScanoutAddress(&vx, (uint32_t)readpage);

			GPUClearScreen(&vx, 0x07070707); // Gray for visibility

			for (int i=0; i<32; ++i)
			{
				SPrimitive prim;
				prim.x0 = rand()%256;
				prim.y0 = rand()%256;
				prim.x1 = rand()%256;
				prim.y1 = rand()%256;
				prim.x2 = rand()%256;
				prim.y2 = rand()%256;
				uint8_t V = rand()%255;
				
				RPUSetPrimitive(&prim);
				RPUSetColor(V);

				int16_t minx = max(0, min(prim.x0, min(prim.x1, prim.x2)))/4;
				int16_t miny = max(0, min(prim.y0, min(prim.y1, prim.y2)))/4;
				int16_t maxx = min(319, max(prim.x0, max(prim.x1, prim.x2)))/4;
				int16_t maxy = min(239, max(prim.y0, max(prim.y1, prim.y2)))/4;

				// CPU write - Mark one pixel out of 16 as debug aid
				/*for (int16_t j = miny; j < maxy; ++j)
				{
					for (int16_t i = minx; i < maxx; ++i)
					{
						uint32_t tileIndex = i+j*80;
						uint32_t tileAddress = tileIndex*16;
						writepage[tileAddress] = V;
					}
				}*/

				// RPU write - 128bit writes per tile from RPU side
				// This version does the sweep in software, to be
				// ported to hardware once writes work as expected.
				for (int16_t j = miny; j < maxy; ++j)
				{
					for (int16_t i = minx; i < maxx; ++i)
					{
						// 128bit aligned address for 16 pixels' worth of output
						// Hardware generates 4x4 pixel masks but the output is
						// linear in memory so it's going to be 16 pixels, packed side by side
						// on the same scanline if viewed as raw output.
						// Here we rewind and start each triangle from offset zero
						// for debug purposes.
						uint32_t tileIndex = i+j*80;
						uint32_t tileAddress = tileIndex*16;
						RPUSetTileAddress((uint32_t)writepage + tileAddress);
						RPURasterizeTile(i*4, j*4);
					}
				}
			}

			++cycle;
		}
	}

	return 0;
}
