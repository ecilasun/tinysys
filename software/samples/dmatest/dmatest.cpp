/** \file
 * DMA usage example.
 * \ingroup examples
 * This example demonstrates how to use the DMA engine to copy data between two memory locations.
 * In this case the source is a buffer containing a sprite and the target is the video buffer.
 * Several sprites are drawn, which move around the screen and bounce off the edges.
 * Set NUM_SPRITES to a higher value to find out how many sprites can be drawn before performance drops.
 */

#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "dma.h"

#include <stdlib.h>

// By default we draw 128 sprites, some of which are 64x64 and some are set to 32x32
#define NUM_SPRITES 128

// This function draws a sprite onto a target buffer by copying the sprite data to the target buffer line by line.
// Please note that each line has to be a multiple of 16 bytes.
void DrawSprite(int x, int y, int w, int h, uint8_t *sprite, uint8_t *targetBuffer, int W, int H)
{
	for (int r=0; r<h; ++r)
		DMACopyAutoByteMask((uint32_t)sprite+r*w, (uint32_t)(targetBuffer+(y+r)*320+x), w/16);
}

int main(int argc, char *argv[])
{
	const uint32_t W = 320;
	const uint32_t H = 240;

	// Create source and target buffers (using VPU functions to get aligned buffer addresses)
	uint8_t *videoPageA = VPUAllocateBuffer(W*H);
	uint8_t *videoPageB = VPUAllocateBuffer(W*H);
	uint8_t *backdrop = VPUAllocateBuffer(W*H);
	uint8_t *spriteA = VPUAllocateBuffer(64*64);
	uint8_t *spriteB = VPUAllocateBuffer(32*32);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);

	// Set buffer B as output
	VPUSetWriteAddress(&vx, (uint32_t)videoPageB);
	VPUSetScanoutAddress(&vx, (uint32_t)videoPageA);
	VPUSetDefaultPalette(&vx);

	struct EVideoSwapContext sc;
	sc.cycle = 0;
	sc.framebufferA = videoPageA;
	sc.framebufferB = videoPageB;
	VPUSwapPages(&vx, &sc);

	// A 64x64 donut sprite with mask
	for (int y=0;y<64;++y)
	{
		for (int x=0;x<64;++x)
		{
			int C = (x-31)*(x-31)+(y-31)*(y-31);
			if (C <= 28*28 && C > 15*15)
				spriteA[x+y*64] = 1 + ((x-y)%250);
			else if (C <= 31*31 && C > 28*28)
				spriteA[x+y*64] = 1; // Blue border
			else
				spriteA[x+y*64] = 0; // Zero is transparent for masked DMA
		}
	}

	// A 32x32 donut sprite with mask
	for (int y=0;y<32;++y)
	{
		for (int x=0;x<32;++x)
		{
			int C = (x-15)*(x-15)+(y-15)*(y-15);
			if (C <= 15*15 && C > 7*7)
				spriteB[x+y*32] = 1 + ((x+y)%250);
			else
				spriteB[x+y*32] = 0; // Zero is transparent for masked DMA
		}
	}

	// Fill buffer B with background data
	for (uint32_t y=0;y<H;++y)
		for (uint32_t x=0;x<W;++x)
			backdrop[x+y*W] = (x^y)%255;

	// DMA operations work directly on memory.
	// Therefore, we need to insert a cache flush here so that
	// the writes to buffer A are all written back to RAM.
	CFLUSH_D_L1;

	// Kick a DMA copy to move buffer A contents onto buffer B
	const uint32_t blockCountInMultiplesOf16bytes = (W*H)/16;
	// Figure out how many DMAs this splits into
	const uint32_t leftoverDMA = blockCountInMultiplesOf16bytes%256;
	const uint32_t fullDMAs = blockCountInMultiplesOf16bytes/256;

	// Sprite coordinates and velocities
	int ox[NUM_SPRITES], dx[NUM_SPRITES];
	int oy[NUM_SPRITES], dy[NUM_SPRITES];
	int spriteIndex[NUM_SPRITES];
	for (int i=0;i<NUM_SPRITES;++i)
	{
		ox[i] = 16 + (rand()%128);
		oy[i] = 16 + (rand()%128);
		dx[i] = (rand()%8) - (rand()%8);
		dy[i] = (rand()%8) - (rand()%8);
		if (dx[i]==0 && dy[i]==0)
		{
			dx[i] = 1;
			dy[i] = 1;
		}
		spriteIndex[i] = rand()%2;
	}

	while (1)
	{
		// Copy solid background
		uint32_t fulloffset = 0;
		for (uint32_t full=0;full<fullDMAs;++full)
		{
			DMACopy4K((uint32_t)(backdrop+fulloffset), (uint32_t)sc.writepage+fulloffset);
			fulloffset += 256*16; // Stride is 4K bytes for 256*16 byte blocks
		}
		if (leftoverDMA != 0)
			DMACopy((uint32_t)(backdrop+fulloffset), (uint32_t)(sc.writepage+fulloffset), leftoverDMA); // Copy the remaining bytes if any

		// Sprites
		for (int i=0; i<NUM_SPRITES; ++i)
		{
			uint8_t *sprite = spriteIndex[i] ? spriteA : spriteB;
			int swh = spriteIndex[i] ? 64 : 32; // Sprites are square in this case, so one dimension is enough
			DrawSprite(ox[i], oy[i], swh, swh, sprite, sc.writepage, W, H);
		}

		// Tag for DMA sync.
		// This is essentially an item that gets processed after the last DMA, therefore we can wait for FIFO to become empty.
		// This is useful if we want to do something after all DMAs have completed such as overlaying some CPU data onto the DMA surface
		// or toggling scan-out buffers.
		DMATag(0x0);

		// Wait until there are no more DMA operations in flight.
		//DMAWait(CPUCoherent); // Use this only if we're going to write over copied DMAd data from CPU (for example, game UI or text)
		DMAWait(CPUIncoherent); // Use this one if we're not going to write over copied DMA data from CPU

		// We can now overlay some CPU data onto the DMA surface if we need to.
		// At this point the 'cpucoherent' flag has invalidated the data cache so we can
		// reload what the DMA has written, and add our data on top.
		/*for (int y = 0; y<96; ++y)
			for (int x = 0; x<96; ++x)
				sc.writepage[(x+ox)+(y+oy)*320] = x^y;
		// Scan-out hardware can only see the actual RAM contents so we need to make sure
		// we've flushed everything in cache to RAM before we can see a stable image.
		// This is only necessary if we're writing to the buffer on the CPU side.
		// DMA writes are always visible to the scan-out hardware.
		CFLUSH_D_L1;*/

		// Bounce the image around the screen
		for (int i=0;i<NUM_SPRITES;++i)
		{
			ox[i] += dx[i];
			oy[i] += dy[i];
			int swhPlusOne = spriteIndex[i] ? 65 : 33;
			int bx = W-swhPlusOne;
			int by = H-swhPlusOne;
			if (ox[i]<0) {ox[i] = 0; dx[i] = -dx[i];}
			if (oy[i]<0) {oy[i] = 0; dy[i] = -dy[i];}
			if (ox[i]>bx) {ox[i] = bx; dx[i] = -dx[i];}
			if (oy[i]>by) {oy[i] = by; dy[i] = -dy[i];}
		}

		// Wait for vsync before we continue if argv[1] is not set.
		// Ideally one would swap to the next backbuffer after this wait.
		if (argc<=1)
			VPUWaitVSync();
		VPUSwapPages(&vx, &sc);
	}
	return 0;
}
