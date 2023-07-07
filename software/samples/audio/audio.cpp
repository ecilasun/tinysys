#include <stdio.h>
#include <math.h>
#include <cstdlib>

#include "core.h"
#include "uart.h"
#include "apu.h"

static short *apubuffer;

#define NUM_CHANNELS 2			// Stereo
#define BUFFER_SAMPLES 512		// buffer size


int main()
{
	apubuffer = (short*)APUAllocateBuffer(BUFFER_SAMPLES*NUM_CHANNELS*sizeof(short));
	UARTWrite("APU mix buffer at 0x");
	UARTWriteHex((unsigned int)apubuffer);
	UARTWrite("\n");

	APUSetBufferSize(BUFFER_SAMPLES);
	APUSetSampleRate(ASR_22050_Hz);
	uint32_t prevframe = APUFrame();

	float offset = 0.f;
	do{
		// Generate individual waves for each channel
		for (uint32_t i=0;i<BUFFER_SAMPLES;++i)
		{
			apubuffer[i*NUM_CHANNELS+0] = short(16384.f*sinf(offset+3.1415927f*float(i)/128.f));
			apubuffer[i*NUM_CHANNELS+1] = short(16384.f*cosf(offset+3.1415927f*float(i*2)/384.f));
		}

		// Make sure the writes are visible by the DMA
		CFLUSH_D_L1;

		// Fill current write buffer with new mix data
		APUStartDMA((uint32_t)apubuffer);
		// Wait for the APU to finish playing back current read buffer
		uint32_t currframe;
		do
		{
			currframe = APUFrame();
		} while (currframe == prevframe);
		prevframe = currframe;
		// Read buffer drained, swap to new read buffer
		APUSwapBuffers();

		offset += 1.f;

	} while(1);

	return 0;
}
