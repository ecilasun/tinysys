#include <stdio.h>
#include <cstdlib>

#include "core.h"
#include "vpu.h"
#include "xadc.h"

int main( int argc, char **argv )
{
	uint32_t x = 0;

	uint32_t temperaturebuffer[320];

	uint8_t *framebufferB = VPUAllocateBuffer(320*240);
	uint8_t *framebufferA = VPUAllocateBuffer(320*240);

	struct EVideoContext vx;
	vx.m_vmode = EVM_320_Wide;
	vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);

	// Set buffer B as output
	VPUSetWriteAddress(&vx, (uint32_t)framebufferA);
	VPUSetScanoutAddress(&vx, (uint32_t)framebufferB);
	VPUSetDefaultPalette(&vx);

	int cycle = 0;
	while (1)
	{
		// Read temperature
		temperaturebuffer[x] = ADCGetRawTemperature();

		// Next sample slot
		x++;

		// Flush data cache at last pixel so we can see a coherent image
		if (x==320)
		{
			// Video scan-out page
			uint8_t *readpage = (cycle%2) ? framebufferA : framebufferB;
			// Video write page
			uint8_t *writepage = (cycle%2) ? framebufferB : framebufferA;
			// Write page as words for faster block copy
			uint32_t *writepageword = (uint32_t*)writepage;
			// flip the read and write pages
			VPUSetWriteAddress(&vx, (uint32_t)writepage);
			VPUSetScanoutAddress(&vx, (uint32_t)readpage);

			// Clear screen to white
			for (uint32_t i=0;i<80*240;++i)
				writepageword[i] = 0x0F0F0F0F;

			// Show temperature value
			for (uint32_t i=0;i<320;++i)
			{
				uint32_t y = (temperaturebuffer[i]/17)%240;
				writepage[i + y*320] = 0x0A; // Green
			}

			x = 0;
			CFLUSH_D_L1;

			++cycle;
		}
	}

	return 0;
}
