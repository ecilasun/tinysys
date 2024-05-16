#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "core.h"
#include "apu.h"
#include "vpu.h"

#include "xmp.h"

#define BUFFER_WORD_COUNT 1024		// buffer size (max: 2048 bytes i.e. 1024 words)
#define BUFFER_SIZE_IN_BYTES (BUFFER_WORD_COUNT*2*sizeof(short))

static short *apubuffer;

static EVideoContext vx;
static EVideoSwapContext sc;

void draw_wave()
{
	VPUClear(&vx, 0x00000000);

	for (uint32_t i=0; i<256; ++i)
	{
		int16_t L = 120 + (apubuffer[i*2+0]>>8);
		int16_t R = 120 + (apubuffer[i*2+1]>>8);
		L = L<0 ? 0 : (L>239 ? 239 : L);
		R = R<0 ? 0 : (R>239 ? 239 : R);
		sc.writepage[i+32 + L*320] = 0x37;
		sc.writepage[i+32 + R*320] = 0x27;
	}

	//VPUWaitVSync();
	CFLUSH_D_L1;
	VPUSwapPages(&vx, &sc);
}

void PlayXMP(const char *fname)
{
	xmp_context ctx;
	struct xmp_module_info mi;
	struct xmp_frame_info fi;
	int row, i;

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

		row = -1;
		int playing = 1;
		while (playing) // size == 2*BUFFER_WORD_COUNT, in bytes
		{
			playing = xmp_play_buffer(ctx, apubuffer, BUFFER_SIZE_IN_BYTES, 0) == 0;

			// Make sure the writes are visible by the DMA
			CFLUSH_D_L1;

			// Fill current write buffer with new mix data
			APUStartDMA((uint32_t)apubuffer);

			// Draw the waveform in the mix buffer so we don't clash with apu buffer
			draw_wave();

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

	PlayXMP(fullpath);

	printf("Playback complete\n");

	return 0;
}
