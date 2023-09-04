#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "task.h"
#include <stdio.h>

static int32_t *s_mposxy_buttons = (int32_t*)MOUSE_POS_AND_BUTTONS;

uint8_t s_cursor[8*8] = {
	0xF,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0xF,0xF,0x0,0x0,0x0,0x0,0x0,0x0,
	0xF,0xF,0xF,0x0,0x0,0x0,0x0,0x0,
	0xF,0xF,0xF,0xF,0x0,0x0,0x0,0x0,
	0xF,0xF,0xF,0xF,0xF,0x0,0x0,0x0,
	0xF,0xF,0x0,0x0,0x0,0x0,0x0,0x0,
	0xF,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0 };

int main(int argc, char *argv[])
{
	const uint32_t W = 320;
	const uint32_t H = 240;

	uint8_t *bufferB = GPUAllocateBuffer(W*H);
	uint8_t *bufferA = GPUAllocateBuffer(W*H);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);

	GPUSetWriteAddress(&vx, (uint32_t)bufferA);
	GPUSetScanoutAddress(&vx, (uint32_t)bufferB);
	GPUSetDefaultPalette(&vx);

	int cycle = 0;
	while (1)
	{
		uint8_t *readpage = (cycle%2) ? bufferA : bufferB;
		uint8_t *writepage = (cycle%2) ? bufferB : bufferA;

		// Wait for vsync
		uint32_t prevvsync = GPUReadVBlankCounter();
		uint32_t currentvsync;
		do {
			currentvsync = GPUReadVBlankCounter();
			TaskYield();
		} while (currentvsync == prevvsync);

		// Swap
		GPUSetWriteAddress(&vx, (uint32_t)writepage);
		GPUSetScanoutAddress(&vx, (uint32_t)readpage);

		// Clear screen
		{
			uint32_t *vramBaseAsWord = (uint32_t*)writepage;
			uint32_t W = 480*80;
			for (uint32_t i=0; i<W; ++i)
				vramBaseAsWord[i] = 0x00000000;
		}

		// TODO: Draw UI

		// Draw cursor
		{
			int32_t mx = s_mposxy_buttons[0];
			int32_t my = s_mposxy_buttons[1];
			for (uint32_t y=0;y<8;++y)
				for (uint32_t x=0;x<8;++x)
					writepage[(x+mx)+(y+my)*320] = s_cursor[x+y*8];
		}

		// Complete writes
		CFLUSH_D_L1;

		// Next frame
		++cycle;
	}

	return 0;
}
