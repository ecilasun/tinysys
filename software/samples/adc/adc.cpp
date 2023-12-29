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

	struct EVideoSwapContext sc;
	sc.cycle = 0;
	sc.framebufferA = framebufferA;
	sc.framebufferB = framebufferB;
	VPUSwapPages(&vx, &sc);

	int cycle = 0;
	char tmpstr[128];
	while (1)
	{
		// Read temperature
		temperaturebuffer[x] = ADCGetRawTemperature();
		float tmp = ADCRawTemperatureToCentigrade(temperaturebuffer[x]);

		// Next sample slot
		x++;

		// Flush data cache at last pixel so we can see a coherent image
		if (x==320)
		{
			// Write page as words for faster block copy
			uint32_t *writepageword = (uint32_t*)sc.writepage;

			// Clear screen to white
			for (uint32_t i=0;i<80*240;++i)
				writepageword[i] = 0x0F0F0F0F;

			// Show temperature value
			for (uint32_t i=0;i<320;++i)
			{
				uint32_t y = (temperaturebuffer[i]/17)%240;
				sc.writepage[i + y*320] = 0x0A; // Green
			}

			x = 0;
			CFLUSH_D_L1;

			int L = snprintf(tmpstr, 127, "Temperature: %f C", tmp);
			VPUPrintString(&vx, 0x00, 0x0F, 8, 8, tmpstr, L);

			++cycle;
			VPUWaitVSync();
			VPUSwapPages(&vx, &sc);
		}
	}

	return 0;
}
