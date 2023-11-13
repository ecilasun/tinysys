#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include <random>

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;
 
static inline int32_t max(int32_t x, int32_t y) { return x>=y?x:y; }
static inline int32_t min(int32_t x, int32_t y) { return x<=y?x:y; }

struct sVec2
{
	int32_t x, y;
};

int32_t edgeFunction(const sVec2 &v0, const sVec2 &v1, int32_t &A, int32_t &B, const sVec2 &p)
{
	A = v0.y - v1.y;
	B = v1.x - v0.x;
	int32_t C = v0.x*v1.y - v0.y*v1.x;

	// Value at upper left corner of 4x4 tile
	// Add A to the following to advance left or B to advance down one pixel
	return A*p.x + B*p.y + C;
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
		sVec2 v0, v1, v2, p;
		memset(sc.writepage, 0x07, 320*240); // Clear to gray

		// Generate a rotating primitive
		const float L = 140.f;
		float X0 = cosf(R)*L;
		float Y0 = sinf(R)*L;
		float X1 = cosf(R+3.92699082f)*L; // 225 degrees
		float Y1 = sinf(R+3.92699082f)*L;
		float X2 = cosf(R+1.57079633f)*L; // 90 degrees
		float Y2 = sinf(R+1.57079633f)*L;

		// Vertices
		v0.x = 160 + X0;
		v0.y = 120 + Y0;
		v1.x = 160 + X1;
		v1.y = 120 + Y1;
		v2.x = 160 + X2;
		v2.y = 120 + Y2;

// Input assembler (IA)
// Input: v0,v1,v2
// Output: a12,b12,a20,b20,a01,b01,bounds to multiple RS units

		// Generate AABB
		int32_t minx = min(v0.x, min(v1.x, v2.x));
		int32_t miny = min(v0.y, min(v1.y, v2.y));
		int32_t maxx = max(v0.x, max(v1.x, v2.x));
		int32_t maxy = max(v0.y, max(v1.y, v2.y));

		// Clip
		minx = max(0,min(319, minx));
		miny = max(0,min(239, miny));
		maxx = max(0,min(319, maxx));
		maxy = max(0,min(239, maxy));

		// Generate edge functions for min AABB corner
		p.x = minx;
		p.y = miny;
		int32_t a01, a12, a20;
		int32_t b01, b12, b20;
		// OPTIMIZATION: If we subtract minx/miny from all vertices,
		// we can use 0/0 for the initial value for p which
		// means we could avoid the extra 2 multiplies in the wN_row terms
		int32_t w0_row = edgeFunction(v1, v2, a12, b12, p);
		int32_t w1_row = edgeFunction(v2, v0, a20, b20, p);
		int32_t w2_row = edgeFunction(v0, v1, a01, b01, p);

// Sweep Rasterizer (RS)
// Input: a12,b12,a20,b20,a01,b01,bounds
// Output: barycentric interpolated color fragment

		int32_t c0 = 0x00;
		int32_t c1 = 0xFF;
		int32_t c2 = 0xC0;

		// Sweep and fill pixels
		uint8_t *rasterOut = sc.writepage;
		for (int32_t y = miny; y<=maxy; ++y)
		{
			int32_t w0 = w0_row;
			int32_t w1 = w1_row;
			int32_t w2 = w2_row;
			rasterOut = sc.writepage + y*320;
			for (int32_t x = minx; x<=maxx; ++x)
			{
				int32_t bary = w0*c0 + w1*c1 + w2*c2;
				if (w0<0 && w1<0 && w2<0) // on edge or inside primitive
					rasterOut[x] = bary;
				else // outside primitive
					rasterOut[x] = 0x0C;
				w0 += a12;
				w1 += a20;
				w2 += a01;
			}
			w0_row += b12;
			w1_row += b20;
			w2_row += b01;
		}

// Output merger (OM)
// Input: color fragments from RS units
// Output: final color values per tile

		CFLUSH_D_L1;

		R += 0.01f;

		GPUWaitVSync();
		GPUSwapPages(&vx, &sc);
	}

	return 0;
}
