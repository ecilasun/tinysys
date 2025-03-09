/** \file
 * Music player example using the libxmp library.
 *
 * \ingroup examples
 * This example demonstrates how to use the libxmp library to play music files.
 * It can play a variety of module formats, such as MOD, S3M, XM, IT.
 * The example uses the APU to mix the audio data and the VPU to display the
 * waveform of the audio data currently being played.
 * Please note that some module formats may be taxing on the tinsys CPU and may not
 * play back smoothly.
 */

#include <complex>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "apu.h"
#include "vpu.h"

#include "xmp.h"

#define BUFFER_WORD_COUNT 1024		// buffer size (max: 2048 bytes i.e. 1024 words)
#define BUFFER_SIZE_IN_BYTES (BUFFER_WORD_COUNT*2*sizeof(short))

static short *apubuffer;

static EVideoContext vx;
static EVideoSwapContext sc;

std::complex<float> outputL[BUFFER_WORD_COUNT*2];
std::complex<float> outputR[BUFFER_WORD_COUNT*2];
int16_t barsL[256];
int16_t barsR[256];

void fft(std::complex<float>* data)
{
    const size_t N = BUFFER_WORD_COUNT;
    const float PI = 3.14159265358979323846f;

    // Bit-reversal permutation
    size_t n = N;
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i < j) {
            std::swap(data[i], data[j]);
        }
        size_t m = n >> 1;
        while (j >= m && m >= 2) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // Cooley-Tukey FFT
    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * PI / len;
        std::complex<float> wlen(cos(angle), sin(angle));
        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1);
            for (size_t j = 0; j < len / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

void draw_wave()
{
	while (1)
	{
		VPUClear(&vx, 0x00000000);

		for (size_t i = 0; i < BUFFER_WORD_COUNT; ++i)
		{
			outputL[i] = std::complex<float>(apubuffer[i*2+0]>>15, 0.0f);
			outputR[i] = std::complex<float>(apubuffer[i*2+1]>>15, 0.0f);
		}
		fft(outputL);
		fft(outputR);

		for (uint32_t i=0; i<BUFFER_WORD_COUNT; i+=4)
		{
			int16_t L = 120 - (int16_t)std::abs(outputL[i]);
			int16_t R = 120 + (int16_t)std::abs(outputR[i]);
			barsL[i>>2] = std::min(239, std::max(0, (barsL[i>>2] + L)/2));
			barsR[i>>2] = std::min(239, std::max(0, (barsR[i>>2] + R)/2));
		}

		for (uint32_t i=0; i<256; ++i)
		{
			sc.writepage[32 + i + barsL[i]*320] = 0x37;
			sc.writepage[32 + i + barsR[i]*320] = 0x27;
		}

		CFLUSH_D_L1;

		VPUWaitVSync();
		VPUSwapPages(&vx, &sc);
	}
}

void PlayXMP(const char *fname)
{
	xmp_context ctx;
	struct xmp_module_info mi;
	struct xmp_frame_info fi;
	int i;

	ctx = xmp_create_context();

	if (xmp_load_module(ctx, fname) < 0)
	{
		printf("Error: cannot load module '%s'\n", fname);
		return;
	}

	APUSetBufferSize(BUFFER_WORD_COUNT); // word count = sample count/2 (i.e. number of stereo sample pairs)
	APUSetSampleRate(ASR_22_050_Hz);
	uint32_t prevframe = APUFrame();

	if (xmp_start_player(ctx, 22050, 0) == 0)
	{
		xmp_get_module_info(ctx, &mi);
		printf("%s (%s)\n", mi.mod->name, mi.mod->type);

		int playing = 1;
		while (playing) // size == 2*BUFFER_WORD_COUNT, in bytes
		{
			playing = xmp_play_buffer(ctx, apubuffer, BUFFER_SIZE_IN_BYTES, 0) == 0;

			// Make sure the writes are visible by the DMA
			CFLUSH_D_L1;

			// Fill current write buffer with new mix data
			APUStartDMA((uint32_t)apubuffer);

			// Wait for the APU to be done with current read buffer which is still playing
			uint32_t currframe;
			do
			{
				// APU will return a different 'frame' as soon as the current buffer reaches the end
				currframe = APUFrame();
			} while (currframe == prevframe);

			// Once we reach this point, the APU has switched to the other buffer we just filled, and playback resumes uninterrupted

			// Remember this frame
			prevframe = currframe;
		}
		xmp_end_player(ctx);

		xmp_release_module(ctx);
		xmp_free_context(ctx);
	}
}

int main(int argc, char *argv[])
{
	apubuffer = (short*)APUAllocateBuffer(BUFFER_SIZE_IN_BYTES);
	memset(apubuffer, 0, BUFFER_SIZE_IN_BYTES);
	printf("\nAPU mix buffer: 0x%.8x\n", (unsigned int)apubuffer);

	char currpath[48] = "sd:/";
	if (getcwd(currpath, 48))
		printf("Working directory:%s\n", currpath);

	char fullpath[128];
	strcpy(fullpath, currpath);
	strcat(fullpath, "/");

	if (argc<=1)
		strcat(fullpath, "test.mod");
	else
		strcat(fullpath, argv[1]);

	uint8_t *bufferB = VPUAllocateBuffer(320*240);
	uint8_t *bufferA = VPUAllocateBuffer(320*240);

    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);

	sc.cycle = 0;
	sc.framebufferA = bufferA;
	sc.framebufferB = bufferB;
	VPUSwapPages(&vx, &sc);
	VPUClear(&vx, 0x00000000);
	VPUSwapPages(&vx, &sc);
	VPUClear(&vx, 0x00000000);

	memset(barsL, 0, 256*sizeof(int16_t));
	memset(barsR, 0, 256*sizeof(int16_t));

	// Always do this from main thread
	struct STaskContext *taskctx1 = TaskGetContext(1);
	uint32_t* stackAddress = new uint32_t[1024];
	int taskID1 = TaskAdd(taskctx1, "draw_wave", draw_wave, TS_RUNNING, QUARTER_MILLISECOND_IN_TICKS, (uint32_t)stackAddress);
	
	PlayXMP(fullpath);

	printf("Playback complete\n");

	return 0;
}
