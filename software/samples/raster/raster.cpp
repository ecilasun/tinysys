#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include <random>

// NOTE: The algorithms here are aimed at hardware an will not make
// sense for a CPU, so please do not read too much into how the code looks.
// In hardware, most flow control is gone, writes are 128bit wide with a 16bit byte write mask.
// Zero bits of the write mask essentially means leave existing memory contents at that byte intact
// so we don't need any special read-write loop.
// Another neat thing is that we can do all of this entirely asynchronous to the CPU,
// and if all goes well, deliver one 16 byte write queued up per clock cycle on average.

// The tile layout is 16x1 (1 row of 16 adjacent 8 bit pixels) since that is the easiest
// format we can use for straightforward scan-out, without having to 'resolve' for instance a 4x4 tile
// written in a complicated pattern in memory to make access linear.
// This means the screen is divided horizontally into 16 pixel columns as a work unit.

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;
 
static inline int32_t max(int32_t x, int32_t y) { return x>=y?x:y; }
static inline int32_t min(int32_t x, int32_t y) { return x<=y?x:y; }

static uint8_t s_texture[] = {
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28,
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
	0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
	0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x28,
	0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x28,
	0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x28,
	0x00, 0x0F, 0x00, 0x00, 0x00, 0x30, 0x00, 0x28,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x28 };

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
	printf("Hardware rasterization prototype in software\n");

	s_framebufferB = GPUAllocateBuffer(320*240); // Or think of it as 1280*64 for tiles
	s_framebufferA = GPUAllocateBuffer(320*240);

	struct EVideoContext vx;
	vx.m_vmode = EVM_320_Wide;
	vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);
	GPUSetDefaultPalette(&vx);

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
		memset(sc.writepage, 0x11, 320*240); // Clear to dark gray (so that we can see the tile edges)

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

		// Round x to multiples of 4 pixels
		minx = minx&0xFFFFFFFFC;
		maxx = (maxx+3)&0xFFFFFFFFC;

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

		int32_t au0 = 0x00, av0 = 0x00;
		int32_t au1 = 0xFF, av1 = 0x00;
		int32_t au2 = 0xFF, av2 = 0xFF;

		// Sweep and fill pixels
		uint32_t *rasterOut = (uint32_t*)sc.writepage;
		for (int32_t y = miny; y<=maxy; ++y)
		{
			// Top of row
			int32_t w0 = w0_row;
			int32_t w1 = w1_row;
			int32_t w2 = w2_row;
			rasterOut = (uint32_t*)(sc.writepage + y*320);

			// Assume 4 pixel combined writes
			// For hardware the following is 16-wide instead of 4-wide
			// and all barycentrics + masks are calculated in parallel
			// for the 16x1 pixel tile.
			for (int32_t x = minx; x<=maxx; x+=4)
			{
				int32_t bval = 0;
				int32_t baryu, baryv;

				// This is one pixel's worth of processing
				// In hardware we'll replicate this 16 times for a 16x1 tile
				uint32_t val0 = (w0&w1&w2)<0 ? 0x000000FF : 0x00000000; // Output in hardware is a single bit (sign bit of w0&w1&w2)
				baryu = abs(w0*au0 + w1*au1 + w2*au2)>>18;
				baryv = abs(w0*av0 + w1*av1 + w2*av2)>>18;
				w0 += a12; // We don't need to do this addition in hardware to pass to the next unit
				w1 += a20; // since each unit has its own scaled multiple of a12/a20/a01 (mul by pixel index)
				w2 += a01;
				bval |= s_texture[(baryu%8)+8*(baryv%8)];

				uint32_t val1 = (w0&w1&w2)<0 ? 0x0000FF00 : 0x00000000;
				baryu = abs(w0*au0 + w1*au1 + w2*au2)>>18;
				baryv = abs(w0*av0 + w1*av1 + w2*av2)>>18;
				w0 += a12;
				w1 += a20;
				w2 += a01;
				bval |= s_texture[(baryu%8)+8*(baryv%8)]<<8;

				uint32_t val2 = (w0&w1&w2)<0 ? 0x00FF0000 : 0x00000000;
				baryu = abs(w0*au0 + w1*au1 + w2*au2)>>18;
				baryv = abs(w0*av0 + w1*av1 + w2*av2)>>18;
				w0 += a12;
				w1 += a20;
				w2 += a01;
				bval |= s_texture[(baryu%8)+8*(baryv%8)]<<16;

				uint32_t val3 = (w0&w1&w2)<0 ? 0xFF000000 : 0x00000000;
				baryu = abs(w0*au0 + w1*au1 + w2*au2)>>18;
				baryv = abs(w0*av0 + w1*av1 + w2*av2)>>18;
				w0 += a12;
				w1 += a20;
				w2 += a01;
				bval |= s_texture[(baryu%8)+8*(baryv%8)]<<24;

				// In hardware this will be a 16 bit write strobe for 128bit SDRAM writes
				uint32_t wmask = (val0 | val1 | val2 | val3);

				// Hardware can write 16 pixels at once, so this'll be wider
				if (wmask != 0) // This is not necessary in hardware. Instead we'll shift direction when we encounter an edge and try to skip zero masks
					rasterOut[x/4] = bval & wmask;
			}

			// Next row
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
