/** \file
 * HBlank interrupt example
 * \ingroup examples
 * This example demonstrates how to use the HBlank interrupt to flip the video buffers.
 * The HBlank interrupt is triggered at the end of each scanline, and can be used to perform
 * tasks that need to be done at the end of each scanline, such as flipping the video buffers.
 */

#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "task.h"
#include "leds.h"

#include <stdlib.h>

static int s_ledstate = 0x1;
static int s_prevFrame = 0xFFFFFFFF;
static struct EVideoContext s_vx;
static struct EVideoSwapContext s_sc;

// This is the HBlank interrupt handler
// It will be called directly within OS context, therefore it has no access to any
// local variables or global variables that are in this code.
// Instead, use mailbox memory to store any data.
void HBlankInterruptHandler()
{
	volatile int *sharedmem = (volatile int*)TaskGetSharedMemory();
	// Let the main code know it can advance to the next frame
	sharedmem[0] += 1;
}

int main(int argc, char *argv[])
{
	const uint32_t W = 320;
	const uint32_t H = 240;

	// Create source and target buffers (using VPU functions to get aligned buffer addresses)
	uint8_t *videoPageA = VPUAllocateBuffer(W*H);
	uint8_t *videoPageB = VPUAllocateBuffer(W*H);

    s_vx.m_vmode = EVM_320_Wide;
    s_vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&s_vx, EVS_Enable);

	// Set buffer B as output
	VPUSetWriteAddress(&s_vx, (uint32_t)videoPageB);
	VPUSetScanoutAddress(&s_vx, (uint32_t)videoPageA);
	VPUSetDefaultPalette(&s_vx);

	s_sc.cycle = 0;
	s_sc.framebufferA = videoPageA;
	s_sc.framebufferB = videoPageB;
	VPUSwapPages(&s_vx, &s_sc);

	// Set up HBlank interrupt for last scanline (479) so it acts as a VBlank interrupt
	VPUSetHBlankHandler((uint32_t)HBlankInterruptHandler);
	VPUSetHBlankScanline(479);
	VPUEnableHBlankInterrupt();

	volatile int *sharedmem = (volatile int*)TaskGetSharedMemory();

	while (1)
	{
		// NOTE: See HBlankInterruptHandler() code about not having access to any task variables
		int currentFrame = sharedmem[0];
		if (currentFrame != s_prevFrame)
		{
			s_prevFrame = currentFrame;

			// We're on line 479, time to flip the buffers
			VPUSwapPages(&s_vx, &s_sc);

			// Clear the backdrop to the swap counter
			VPUClear(&s_vx, s_sc.cycle);

			LEDSetState(s_ledstate++);

			// TODO: Draw your game to the memory address in 's_sc.writepage'
			// Depending on the game, you may want to DMA in the backdrop buffer and then DMA the sprites on top of it
			// after which the UI layer can be drawn, or simply draw everything in software.
		}
	}
	return 0;
}
