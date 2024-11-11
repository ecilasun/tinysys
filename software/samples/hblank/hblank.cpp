/** \file
 * HBlank interrupt example
 * \ingroup examples
 * This example demonstrates how to use the HBlank interrupt to divide
 * the screen into two regions with different resolutions.
 * The top region runs at 320x240 resolution for 240 scanlines,
 * and the bottom region runs at 640x480 resolution for another 240 scanlines.
 * Please note that the actual screen scan-out is always done at 640x480 resolution,
 * therefore the 320x240 resolution is actually pixel doubled by hardware
 * which allows this trick to work.
 */

#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "task.h"
#include "leds.h"

#include <stdlib.h>

static int s_ledstate = 0x1;
static int s_prevFrame = 0xFFFFFFFF;
static struct EVideoSwapContext s_sc;

// This is the HBlank interrupt handler
// It will be called directly within OS context, therefore it has no access to any
// local variables or global variables that are in this code (as all the existing registers & stack will be garbled).
// Instead, use mailbox memory to store any data.
void HBlankInterruptHandler()
{
	volatile int *sharedmem = (volatile int*)TaskGetSharedMemory();

	// Let the main code know it can advance to the next frame
	sharedmem[0] += 1;

	// Change where the hblank triggers based on even/odd frame
	// Also toggle the color palette entry for color 0xBA between red and black
	if (sharedmem[0]%2 == 0)
	{
		// Next time we'll trigger at bottom section of screen
		VPUSetHBlankScanline(240);

		// We're in top section of screen
		// This section of the screen runs at 320x240 resolution
		EVideoContext* vx = (EVideoContext*)&sharedmem[1];
		vx->m_vmode = EVM_320_Wide;
		VPUSetVMode(vx, EVS_Enable);
		// Red strips
		VPUSetPal(0xBA, 0xFF, 0, 0);
	}
	else
	{
		// Next time we'll trigger at top section of screen
		VPUSetHBlankScanline(0);

		// We're in bottom section of screen
		// This section of the screen runs at 640x480 resolution
		EVideoContext* vx = (EVideoContext*)&sharedmem[1];
		vx->m_vmode = EVM_640_Wide;
		VPUSetVMode(vx, EVS_Enable);
		// Blue strips
		VPUSetPal(0xBA, 0, 0, 0xFF);
	}
}

int main(int argc, char *argv[])
{
	// Allocate a buffer large enough for a 640x480 8-bit indexed color image
	const uint32_t W = 640;
	const uint32_t H = 480;

	// We draw to and display from the same buffer (no double buffering)
	uint8_t *videoPageA = VPUAllocateBuffer(W*H);
	// Draw a pattern in the buffer so we can see the resolution change
	{
		uint32_t *vramBaseAsWord = (uint32_t*)videoPageA;
		uint32_t bufferSizeInWords = 480*160;
		for (uint32_t i=0; i<bufferSizeInWords; ++i)
			vramBaseAsWord[i] = 0xBABA0000;
		CFLUSH_D_L1;
	}

	// Initialize contents of shared memory before we install the hblank handler
	volatile int *sharedmem = (volatile int*)TaskGetSharedMemory();
	sharedmem[0] = 0;

	// Set up video context in shared memory
	EVideoContext* vx = (EVideoContext*)&sharedmem[1];
    vx->m_vmode = EVM_320_Wide;
    vx->m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(vx, EVS_Enable);

	// Set buffer B as output
	VPUSetWriteAddress(vx, (uint32_t)videoPageA);
	VPUSetScanoutAddress(vx, (uint32_t)videoPageA);
	VPUSetDefaultPalette(vx);

	s_sc.cycle = 0;
	s_sc.framebufferA = videoPageA;
	s_sc.framebufferB = videoPageA;

	// Prepare for first time display
	VPUSwapPages(vx, &s_sc);

	// Set up HBlank interrupt to trigger in the middle of the screen
	VPUSetHBlankHandler((uint32_t)HBlankInterruptHandler);
	VPUSetHBlankScanline(240); // Bottom section of screen

	// Enable HBlank interrupt (this has to happen after all setup)
	VPUEnableHBlankInterrupt();

	while (1)
	{
		// NOTE: See HBlankInterruptHandler() code about not having access to any task variables
		int currentFrame = sharedmem[0];
		if (currentFrame != s_prevFrame)
		{
			// We're now either at top or bottom section of screen
			s_prevFrame = currentFrame;

			// Change LED state for heartbeat tracking
			LEDSetState(s_ledstate++);
		}
	}

	// NOTE: OS will disable hblank interrupt when the task is terminated, so we don't need to do anything special here
	return 0;
}
