#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "tru.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include <random>

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;

static void clear(struct ERasterizerContext *rctx) {
	SPrimitive prim;
	prim.x0 = 0;
	prim.y0 = 0;
	prim.x1 = 319;
	prim.y1 = 239;
	prim.x2 = 319;
	prim.y2 = 0;
	TRUPushPrimitive(rctx, &prim);
	TRUSetColor(rctx, 0);
	TRURasterizePrimitive(rctx);

	prim.x0 = 319;
	prim.y0 = 239;
	prim.x1 = 0;
	prim.y1 = 0;
	prim.x2 = 0;
	prim.y2 = 239;
	TRUPushPrimitive(rctx, &prim);
	TRUSetColor(rctx, 0);
	TRURasterizePrimitive(rctx);
}

int main(int argc, char *argv[])
{
	printf("Hardware rasterization demo\n");

	s_framebufferB = VPUAllocateBuffer(320*240);
	s_framebufferA = VPUAllocateBuffer(320*240);

	EVideoContext vx;
	vx.m_vmode = EVM_320_Wide;
	vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);
	VPUSetDefaultPalette(&vx);

	EVideoSwapContext sc;
	sc.cycle = 0;
	sc.framebufferA = s_framebufferA;
	sc.framebufferB = s_framebufferB;
	VPUSwapPages(&vx, &sc);

	ERasterizerContext rctx;
	TRUSetTileBuffer(&rctx, (uint32_t)sc.writepage);

	float R = 0.f;
	while (1)
	{
		clear(&rctx);

		// Generate a rotating primitive
		const float L = 140.f;
		float X0 = cosf(R)*L;
		float Y0 = sinf(R)*L;
		float X1 = cosf(R+3.92699082f)*L; // 225 degrees
		float Y1 = sinf(R+3.92699082f)*L;
		float X2 = cosf(R+1.57079633f)*L; // 90 degrees
		float Y2 = sinf(R+1.57079633f)*L;

		// Vertices
		SPrimitive prim;
		prim.x0 = 160 + X0;
		prim.y0 = 120 + Y0;
		prim.x1 = 160 + X2;
		prim.y1 = 120 + Y2;
		prim.x2 = 160 + X1;
		prim.y2 = 120 + Y1;

		// Single black triangle
		TRUPushPrimitive(&rctx, &prim);
		TRUSetColor(&rctx, 0);
		TRURasterizePrimitive(&rctx);

		prim.x0 = 160 + X0;
		prim.y0 = 120 + Y0;
		prim.x1 = 160 + X1;
		prim.y1 = 120 + Y1;
		prim.x2 = 160 + X2;
		prim.y2 = 120 + Y2;
		TRUPushPrimitive(&rctx, &prim);
		TRUSetColor(&rctx, 0x0C);
		TRURasterizePrimitive(&rctx);

		TRUFlushCache(&rctx);
		TRUInvalidateCache(&rctx);

		R += 0.01f;

		VPUWaitVSync();
		VPUSwapPages(&vx, &sc);
		TRUSetTileBuffer(&rctx, (uint32_t)sc.writepage);
	}

	return 0;
}
