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
	uint8_t *bufferA = VPUAllocateBuffer(W*H*3);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);

	// Set buffer B as output
	VPUSetWriteAddress(&vx, (uint32_t)bufferA);
	VPUSetScanoutAddress(&vx, (uint32_t)bufferC);
	VPUSetDefaultPalette(&vx);

	// Fill buffer A with some data
	for (uint32_t y=0;y<H*3;++y)
	{
		for (uint32_t x=0;x<W;++x)
		{
			if (((x/19)&1)^(((y+x)/21)&1))
				bufferA[x+y*W] = (x^y)%255;
			else
				bufferA[x+y*W] = 0x0;
		}
	}

	// Fill buffer B with background data
	for (uint32_t y=0;y<H;++y)
		for (uint32_t x=0;x<W;++x)
			bufferB[x+y*W] = y%255;

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

	int32_t offset = 0;
	int32_t dir = 2;
	int32_t averagetime = 131072;
	int32_t outstats = 0;
	uint64_t starttime = E32ReadTime();
	uint32_t prevvsync = VPUReadVBlankCounter();
	int ox = 32, dx = 1;
	int oy = 32, dy = 2;
	while (1)
	{
		uint64_t endtime = E32ReadTime();
		averagetime = (averagetime + (uint32_t)(endtime-starttime))/2;

		if (outstats % 512 == 0)
			printf("DMA clocks (average): %ld\n", averagetime);
		++outstats;

		starttime = E32ReadTime();

		uint32_t fulloffset = 0;
		for (uint32_t full=0;full<fullDMAs;++full)
		{
			DMACopy4K((uint32_t)(bufferB+fulloffset), (uint32_t)bufferC+fulloffset);
			fulloffset += 256*16; // 16 bytes for each 256-block, total of 4K
		}
		if (leftoverDMA!=0)
			DMACopy((uint32_t)(bufferB+fulloffset), (uint32_t)(bufferC+fulloffset), leftoverDMA);

		// Do the full 4K DMA blocks first
		fulloffset = 0;
		for (uint32_t full=0;full<fullDMAs;++full)
		{
			DMACopyAutoByteMask4K((uint32_t)(bufferA+offset*W+fulloffset), (uint32_t)bufferC+fulloffset);
			fulloffset += 256*16; // 16 bytes for each 256-block, total of 4K
		}

		// Queue up last DMA block less than 4K in size
		if (leftoverDMA!=0)
			DMACopyAutoByteMask((uint32_t)(bufferA+offset*W+fulloffset), (uint32_t)(bufferC+fulloffset), leftoverDMA);

		// Copy some unaligned data
		//DMACopyUnaligned();

		// Copy some unaligned masked data
		//DMACopyUnalignedMask();

		// Tag for DMA sync (essentially an item in FIFO after last DMA so we can check if DMA is complete when this drains)
		DMATag(0x0);

		// Wait until there are no more DMA operations in flight.
		// Since we need all writes visible by the CPU we need to use 'cpucoherent' flag here.
		DMAWait(CPUCoherent);

		// Try to overlay some CPU image onto the DMA surface.
		// At this point the 'cpucoherent' flag has invalidated the data cache so we can
		// reload what the DMA has written, and add our data on top.
		for (int y = 0; y<96; ++y)
			for (int x = 0; x<96; ++x)
				bufferC[(x+ox)+(y+oy)*320] = x^y;
		ox += dx;
		oy += dy;
		if (ox>222 || ox<0) dx = -dx;
		if (oy>142 || oy<0) dy = -dy;

		// Scan-out hardware can only see the actual RAM contents so we need to make sure
		// we've flushed everything in cache to RAM before we can see a stable image.
		CFLUSH_D_L1;

		// Wait for vsync on the CPU side.
		// Ideally one would install a vsync handler and swap pages based on that instead of stalling.
		// TODO: vblank interrupt support.
		if (argc<=1)
		{
			uint32_t currentvsync;
			do {
				currentvsync = VPUReadVBlankCounter();
			} while (currentvsync == prevvsync);
			prevvsync = currentvsync;
		}

		// Handle scroll of the inner region
		offset = (offset+dir);
		if (offset == 0 || offset == H*2-2) dir = -dir;
	}
	return 0;
}
