#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "dma.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("DMA test\n");

	const uint32_t W = 320;
	const uint32_t H = 240;

	printf("Preparing buffers\n");

	// Create source and target buffers (using VPU functions to get aligned buffer addresses)
	uint8_t *bufferC = VPUAllocateBuffer(W*H);
	uint8_t *bufferB = VPUAllocateBuffer(W*H);
	uint8_t *sprite = VPUAllocateBuffer(64*64);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);

	// Set buffer B as output
	VPUSetWriteAddress(&vx, (uint32_t)bufferB);
	VPUSetScanoutAddress(&vx, (uint32_t)bufferC);
	VPUSetDefaultPalette(&vx);

	// A 64x64 sprite with mask
	for (uint32_t y=0;y<64;++y)
	{
		for (uint32_t x=0;x<64;++x)
		{
			// Draw a donut
			int C = (x-31)*(x-31)+(y-31)*(y-31);
			if (C < 31*31 && C > 15*15)
				sprite[x+y*64] = 1 + ((x-y)%250);
			else
				sprite[x+y*64] = 0; // Zero is transparent for masked DMA
		}
	}

	// Fill buffer B with background data
	for (uint32_t y=0;y<H;++y)
		for (uint32_t x=0;x<W;++x)
			bufferB[x+y*W] = (x^y)%255;

	// DMA operatins work directly on memory.
	// Therefore, we need to insert a cache flush here so that
	// the writes to buffer A are all written back to RAM.
	CFLUSH_D_L1;

	// Kick a DMA copy to move buffer A contents onto buffer B
	const uint32_t blockCountInMultiplesOf16bytes = (W*H)/16;
	// Figure out how many DMAs this splits into
	const uint32_t leftoverDMA = blockCountInMultiplesOf16bytes%256;
	const uint32_t fullDMAs = blockCountInMultiplesOf16bytes/256;
	printf("Initiating copy loop of %ld*256*16byte blocks and %ld*1*16byte block for a total of %ld bytes\n", fullDMAs, leftoverDMA, fullDMAs*4096+leftoverDMA*16);

	//int32_t offset = 0;
	//int32_t dir = 2;
	int32_t averagetime = 20000;
	int32_t outstats = 0;
	uint32_t prevvsync = VPUReadVBlankCounter();
	int ox = 32, dx = 1;
	int oy = 32, dy = 2;
	while (1)
	{
		uint64_t starttime = E32ReadTime();

		uint32_t fulloffset = 0;
		for (uint32_t full=0;full<fullDMAs;++full)
		{
			DMACopy4K((uint32_t)(bufferB+fulloffset), (uint32_t)bufferC+fulloffset);
			fulloffset += 256*16; // 16 bytes for each 256-block, total of 4K
		}
		if (leftoverDMA!=0)
			DMACopy((uint32_t)(bufferB+fulloffset), (uint32_t)(bufferC+fulloffset), leftoverDMA);

		// Copy a 64x64 block of masked sprite data onto the back buffer.
		// Note that minimum DMA size is 16 bytes so the sprite has to be at least 16 pixels wide.
		// There are no height restrictions for sprites.
		for (int r=0; r<64; ++r)
			DMACopyAutoByteMask((uint32_t)sprite+r*64, (uint32_t)(bufferC+(oy+r)*320+ox), 4);

		// Copy some unaligned data
		//DMACopyUnaligned();

		// Copy some unaligned masked data
		//DMACopyUnalignedMask();

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
				bufferC[(x+ox)+(y+oy)*320] = x^y;*/

		// Bounce the image around the screen
		ox += dx;
		oy += dy;
		if (ox>255 ) {ox = 255; dx = -dx;}
		if (ox<0) {ox = 0; dx = -dx;}
		if (oy>175) {oy = 175; dy = -dy;}
		if (oy<0) {oy = 0; dy = -dy;}

		// Scan-out hardware can only see the actual RAM contents so we need to make sure
		// we've flushed everything in cache to RAM before we can see a stable image.
		// This is only necessary if we're writing to the buffer on the CPU side.
		// DMA writes are always visible to the scan-out hardware.
		//CFLUSH_D_L1;

		// Wait for vsync before we continue.
		// Ideally one would swap to the next backbuffer after this wait.
		if (argc<=1)
		{
			uint32_t currentvsync;
			do {
				currentvsync = VPUReadVBlankCounter();
			} while (currentvsync == prevvsync);
			prevvsync = currentvsync;
		}

		uint64_t endtime = E32ReadTime();
		averagetime = (averagetime + (uint32_t)(endtime-starttime))/2;

		if (outstats % 512 == 0)
			printf("DMA clocks (average): %ld\n", averagetime);
		++outstats;
	}
	return 0;
}
