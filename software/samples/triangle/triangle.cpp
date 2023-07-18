#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include <stdio.h>
#include <random>

static uint8_t *framebuffer;

static inline int16_t max(int16_t x, int16_t y) { return x>y?x:y; }
static inline int16_t min(int16_t x, int16_t y) { return x<y?x:y; }

struct sVec2
{
	int16_t x, y;
};

int16_t edgeFunction(const sVec2 &a, const sVec2 &b, const sVec2 &c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

void raster(const sVec2 &v0, const sVec2 &v1, const sVec2 &v2)
{
	//int16_t area = edgeFunction(v0, v1, v2);

	int16_t minx = min(v0.x, min(v1.x, v2.x));
	int16_t miny = min(v0.y, min(v1.y, v2.y));
	int16_t maxx = max(v0.x, max(v1.x, v2.x));
	int16_t maxy = max(v0.y, max(v1.y, v2.y));

	uint8_t V = rand()%255;

	for (int16_t j = miny; j < maxy; ++j)
	{
		for (int16_t i = minx; i < maxx; ++i)
		{
			sVec2 p = {i,j};//{i + 0.5f, j + 0.5f};
			int16_t w0 = edgeFunction(v1, v2, p);
			int16_t w1 = edgeFunction(v2, v0, p);
			int16_t w2 = edgeFunction(v0, v1, p);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				/*w0 /= area;
				w1 /= area;
				w2 /= area;
				int16_t ipolAttr = w0 * attr0[0] + w1 * attr1[0] + w2 * attr2[0];*/
				framebuffer[i+j*320] = V;
			}
		}
	}

	CFLUSH_D_L1;
}

int main(int argc, char *argv[])
{
	printf("Software rasterization test\n");

	framebuffer = GPUAllocateBuffer(320*240);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);

	GPUSetWriteAddress(&vx, (uint32_t)framebuffer);
	GPUSetScanoutAddress(&vx, (uint32_t)framebuffer);
	GPUSetDefaultPalette(&vx);

	while (1)
	{
		sVec2 v0 = sVec2{int16_t(rand()%320), int16_t(rand()%240)};
		sVec2 v1 = sVec2{int16_t(rand()%320), int16_t(rand()%240)};
		sVec2 v2 = sVec2{int16_t(rand()%320), int16_t(rand()%240)};

		raster(v0, v1, v2);
	}

	return 0;
}
