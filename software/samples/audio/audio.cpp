/** \file
 * Audio generation example.
 * \ingroup examples
 * This example demonstrates how to generate a simple audio tone and play it back using the APU.
 * The example generates a simple stereo tone, please connect headphones or speakers to the audio output of the board.
 */

#include <stdio.h>
#include <math.h>
#include <cstdlib>

#include "core.h"
#include "apu.h"
#include "task.h"

static short *apubuffer;

#define NUM_CHANNELS 2			// Stereo
#define BUFFER_SAMPLES 512		// buffer size

// Approximation of an old school phone ring tone
int main()
{
	apubuffer = (short*)APUAllocateBuffer(BUFFER_SAMPLES*NUM_CHANNELS*sizeof(short));
	printf("APU mix buffer at 0x%.4x\n", (unsigned int)apubuffer);

	APUSetBufferSize(BUFFER_SAMPLES);
	APUSetSampleRate(ASR_22_050_Hz);
	uint32_t prevframe = APUFrame();

	float offset = 0.f;
	do{
		// Generate individual waves for each channel
		for (uint32_t i=0;i<BUFFER_SAMPLES;++i)
		{
			apubuffer[i*NUM_CHANNELS+0] = short(16384.f*sinf(offset+2.f*3.1415927f*float(i)/12.f));
			apubuffer[i*NUM_CHANNELS+1] = short(16384.f*cosf(offset+2.f*3.1415927f*float(i*2)/38.f));
		}

		// Make sure the writes are visible by the audio DMA
		CFLUSH_D_L1();

		// Fill current write buffer with new mix data
		APUStartDMA((uint32_t)apubuffer);

		// Wait for the APU to finish playing back current read buffer
		uint32_t currframe;
		do
		{
			currframe = APUFrame();
		} while (currframe == prevframe);

		// Once we reach this point, the APU has switched to the other buffer we just filled, and playback resumes uninterrupted

		// Remember this frame for next time
		prevframe = currframe;

		// Yield to OS
		TaskYield();

		offset += 1.f;

	} while(1);

	return 0;
}
