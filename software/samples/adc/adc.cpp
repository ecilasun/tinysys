#include <stdio.h>
#include <cstdlib>

#include "core.h"
#include "uart.h"
#include "gpu.h"
#include "xadc.h"

int main( int argc, char **argv )
{
	uint32_t x = 0;

    uint32_t ch0buffer[8][320];
    uint32_t temperaturebuffer[320];

	uint8_t *framebufferB = GPUAllocateBuffer(320*240);
	uint8_t *framebufferA = GPUAllocateBuffer(320*240);

	struct EVideoContext vx;
    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);

	// Set buffer B as output
	GPUSetWriteAddress(&vx, (uint32_t)framebufferA);
	GPUSetScanoutAddress(&vx, (uint32_t)framebufferB);
    GPUSetDefaultPalette(&vx);

    int cycle = 0;
    while (1)
    {
        // Read analog inputs
        for (uint32_t ch=0;ch<8;++ch)
            ch0buffer[ch][x] = ANALOGINPUTS[ch];

        // Read temperature
        temperaturebuffer[x] = *XADCTEMP;

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
            GPUSetWriteAddress(&vx, (uint32_t)writepage);
            GPUSetScanoutAddress(&vx, (uint32_t)readpage);

            // Clear screen to white
            for (uint32_t i=0;i<80*240;++i)
                writepageword[i] = 0x0F0F0F0F;

            // Show analog input channels
            for (uint32_t ch=0;ch<8;++ch)
            {
                for (uint32_t i=0;i<320;++i)
                {
                    uint32_t y = (ch0buffer[ch][i]*10)/43; // Map 0...1023 to 0..238
                    y = y>239 ? 239 : y;
                    writepage[i + y*320] = ch; // One color per channel
                }
            }

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
