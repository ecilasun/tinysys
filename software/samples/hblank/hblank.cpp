/** \file
 * HBlank interrupt example
 * \ingroup examples
 * This example demonstrates how to use the HBlank interrupt to draw a colored bar.
 * 
 * /////////////// DANGER! ///////////////
 * NOTE: HBlank interrupts are not recommended for general use. They have no access to any local
 * variables or global variables in their parent executable, as they use the OS stack and environment.
 * They can permanently cause damage to files or lock up the system if not implemented correctly.
 * Recommended way to store variables or context pointer is to use the scratchpad memory via TaskGetSharedMemory()
 * * /////////////// DANGER! ///////////////
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
// local variables or global variables.
// Make sure to use TaskGetSharedMemory() to pull your variables and not from statics / globals of this code.
void HBlankInterruptHandler()
{
	// We are now on the desired scanline

	// First order of business is to set the irq clear latch
	// This will reset the pending interrupt state so that we don't re-trigger the interrupt
	// as soon as we return from this function
	// It also acts as a temporary interrupt disable so the interrupt state won't revert to pending
	VPUHBlankIRQLatchSet();

	// We don't have access to our own stack, so we need to use shared memory to communicate with the main code
	// or track our variables
	volatile int *sharedmem = (volatile int*)TaskGetSharedMemory();

	// Let the main code know it can do something (in this case advance the LED state)
	sharedmem[0] = sharedmem[0] + 1;
	uint32_t frame = sharedmem[0];

	// Here's our video buffer
	uint32_t *videobuffer = (uint32_t *)sharedmem[2];

	// In case we need to access video context to switch video modes on a scanline, we can do that here
	//EVideoContext* vx = (EVideoContext*)&sharedmem[1];

	// NOTE: This is the scanline 'now' not where we triggered the IRQ
	uint32_t scanline = VPUGetScanline();

	uint32_t framedupe = (frame&0xFF) | (frame&0xFF)<<8 | (frame&0xFF)<<16 | (frame&0xFF)<<24;
	if (scanline < 480)
	{
		for (int i=0; i<80; ++i)
			videobuffer[i+scanline*80] = framedupe;
	}

	// Make writes visible to scan-out hardware
	CFLUSH_D_L1;

	// Set new interrupt scanline based on the current frame counter
	VPUSetHBlankScanline(frame%2 ? 100 : 450);

	// Reset the irq clear latch so it doesn't end up clearing our next interrupt
	// This will also re-enables the interrupt state to signal pending
	VPUHBlankIRQLatchReset();
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
	sharedmem[2] = (uint32_t)videoPageA; // video buffer pointer

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
	// WARNING! HBlank interrupt handler has no access to any local variables or global variables in this code
	// as it runs in OS context.
	// Also note that the handler will not throw any exceptions, and can't be stopped via normal means
	// until it's uninstalled.
	VPUSetHBlankHandler((uint32_t)HBlankInterruptHandler);
	VPUSetHBlankScanline(450); // Bottom section of screen

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
