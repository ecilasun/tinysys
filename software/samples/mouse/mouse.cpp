#include "basesystem.h"
#include "core.h"
#include "gpu.h"
#include "task.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>

static int32_t *s_mposxy_buttons = (int32_t*)MOUSE_POS_AND_BUTTONS;
static EVideoContext vx;
static EVideoSwapContext sc;

uint8_t s_cursor[8*8] = {
	0xF,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0xF,0xF,0x0,0x0,0x0,0x0,0x0,0x0,
	0xF,0xF,0xF,0x0,0x0,0x0,0x0,0x0,
	0xF,0xF,0xF,0xF,0x0,0x0,0x0,0x0,
	0xF,0xF,0xF,0xF,0xF,0x0,0x0,0x0,
	0xF,0xF,0x0,0x0,0x0,0x0,0x0,0x0,
	0xF,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0 };

struct SDrawPoints
{
	int16_t x, y;
};

void plotLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	int16_t dx = (x1 - x0);
	dx = dx < 0 ? -dx : dx;
	int16_t sx = x0 < x1 ? 1 : -1;
	int16_t dy = y1 - y0;
	dy = dy < 0 ? dy : -dy;
	int16_t sy = y0 < y1 ? 1 : -1;
	int16_t error = dx + dy;
	
	while(true)
	{
		sc.writepage[x0+y0*320] = 0x0A;
		if (x0 == x1 && y0 == y1)
			break;
		int16_t e2 = 2 * error;
		if (e2 >= dy)
		{
			if (x0 == x1)
				break;
			error = error + dy;
			x0 = x0 + sx;
		}
		if (e2 <= dx)
		{
			if (y0 == y1)
				break;
			error = error + dx;
			y0 = y0 + sy;
		}
	}
}

int main(int argc, char *argv[])
{
	const uint32_t W = 320;
	const uint32_t H = 240;
	int numPoints = 0;
	int16_t prevX = -1;
	int16_t prevY = -1;

	uint8_t *bufferB = GPUAllocateBuffer(W*H);
	uint8_t *bufferA = GPUAllocateBuffer(W*H);
	uint8_t *s_rasterBuffer = GPUAllocateBuffer(80*60*16); // Tile buffer
	memset(s_rasterBuffer, 0x00, 80*60*16); // Black

	// Draw surface
	SDrawPoints *drawBuffer = (SDrawPoints*)GPUAllocateBuffer(W*H);

    vx.m_vmode = EVM_320_Wide;
    vx.m_cmode = ECM_8bit_Indexed;
	GPUSetVMode(&vx, EVS_Enable);

	GPUSetDefaultPalette(&vx);
	RPUSetTileBuffer((uint32_t)s_rasterBuffer);

	sc.cycle = 0;
	sc.framebufferA = bufferA;
	sc.framebufferB = bufferB;
	GPUSwapPages(&vx, &sc);

	while (1)
	{
		TaskYield();

		// Draw "UI" (red rectangle)
		{
			SPrimitive prim;
			prim.x0 = 0;
			prim.y0 = 0;
			prim.x1 = 319;
			prim.y1 = 239;
			prim.x2 = 319;
			prim.y2 = 0;
			RPUPushPrimitive(&prim);
			RPUSetColor(0x0C);
			RPURasterizePrimitive();

			prim.x0 = 319;
			prim.y0 = 239;
			prim.x1 = 0;
			prim.y1 = 0;
			prim.x2 = 0;
			prim.y2 = 239;
			RPUPushPrimitive(&prim);
			RPUSetColor(0x0C);
			RPURasterizePrimitive();
		}

		// Queue up a flush, wait for all raster work to finish, and resolve onto write page
		RPUFlushCache();
		RPUInvalidateCache();
		RPUBarrier();
		RPUWait();
		DMAResolveTiles((uint32_t)s_rasterBuffer, (uint32_t)sc.writepage);

		// Cursor overlay
		// TODO: Use masked DMA hardware for this to be more efficient
		{
			int16_t mx = (int16_t)s_mposxy_buttons[0];
			int16_t my = (int16_t)s_mposxy_buttons[1];

			// Add new position if we're at a new position with mouse button held
			if (s_mposxy_buttons[2] && prevX!=mx && prevY!=my)
			{
				drawBuffer[numPoints].x = mx;
				drawBuffer[numPoints].y = my;
				numPoints++;
				prevX = mx;
				prevY = my;
			}

			// Draw lines betweeen collected points
			for (int i=0;i<numPoints-1;++i)
				plotLine(drawBuffer[i].x,drawBuffer[i].y,drawBuffer[i+1].x,drawBuffer[i+1].y);

			for (uint32_t y=0;y<8;++y)
				for (uint32_t x=0;x<8;++x)
					if (s_cursor[x+y*8] && (x+mx < 320) && (y+my < 240))
						sc.writepage[(x+mx)+(y+my)*320] = s_cursor[x+y*8];
		}

		// Complete memory writes from CPU side
		CFLUSH_D_L1;

		GPUWaitVSync();
		GPUSwapPages(&vx, &sc);
	}

	return 0;
}
