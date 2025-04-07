// Changes required to run embedded quake on tinysys
// Added by Engin Cilasun

#include <quakembd.h>
#include <string.h>
#include "core.h"
#include "vpu.h"

#define	DISPLAY_WIDTH 320
#define	DISPLAY_HEIGHT 240
//((DISPLAY_WIDTH * 9 + 8) / 16)

static uint8_t* framebuffer;
static struct EVideoContext g_vctx;
static struct EVideoSwapContext g_sctx;

int qembd_get_width()
{
	return DISPLAY_WIDTH;
}

int qembd_get_height()
{
	return DISPLAY_HEIGHT;
}

void qembd_vidinit()
{
	framebuffer = VPUAllocateBuffer(DISPLAY_WIDTH * DISPLAY_HEIGHT);
	memset(framebuffer, 0x0, DISPLAY_WIDTH * DISPLAY_HEIGHT);

	g_vctx.m_cmode = ECM_8bit_Indexed;
	g_vctx.m_vmode = EVM_320_Wide;
	VPUSetVMode(&g_vctx, EVS_Enable);
	VPUSetWriteAddress(&g_vctx, (uint32_t)framebuffer);		// TEST: single buffer
	VPUSetScanoutAddress(&g_vctx, (uint32_t)framebuffer);
}

void qembd_fillrect(uint8_t *src, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
	for (int py = 0; py < ysize; py++) {
		int offset = (y + py) * DISPLAY_WIDTH + x;
		for (int px = 0; px < xsize; px++)
			framebuffer[offset + px] = src[offset + px];
	}
}

void qembd_refresh()
{
	// We'll need this for VPU coherency
	CFLUSH_D_L1();

	// Not sure what else this is supposed to be doing, swap buffers?
}
