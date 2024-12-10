/** \file
 * Mandelbrot set example.
 *
 * \ingroup examples
 * This example demonstrates the use of the VPU frame buffers to render a Mandelbrot set.
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <cmath>

#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "vpu.h"

inline int evalMandel(const int maxiter, int col, int row, float ox, float oy, float sx)
{
	int iteration = 0;

	float c_re = (float(col) - 160.f) / 240.f * sx + ox; // Divide by shortest side of display for correct aspect ratio
	float c_im = (float(row) - 120.f) / 240.f * sx + oy;
	float x = 0.f, y = 0.f;
	float x2 = 0.f, y2 = 0.f;
	while (x2+y2 < 4.f && iteration < maxiter)
	{
		y = c_im + 2.f*x*y;
		x = c_re + x2 - y2;
		x2 = x*x;
		y2 = y*y;
		++iteration;
	}

	return iteration;
}

void mandelbrotFloat(uint8_t* framebuffer, int tx, int ty, int self, float ox, float oy, float sx)
{
// http://blog.recursiveprocess.com/2014/04/05/mandelbrot-fractal-v2/
	int R = int(27.71f-5.156f*logf(sx));

	for (int y = 0; y < 16; ++y)
	{
		int row = y + ty*16;
		for (int x = 0; x < 16; ++x)
		{
			int col = x + tx*16;

			int M = evalMandel(R, col, row, ox, oy, sx);
			float ratio = float(M) / float(R);
			int c = int(ratio*255.f);
			framebuffer[col + (row*320)] = c;
		}
	}

	// Flush tile to memory
	CFLUSH_D_L1;

	// distance	(via iq's shadertoy sample https://www.shadertoy.com/view/lsX3W4)
		// d(c) = |Z|·log|Z|/|Z'|
		//float d = 0.5*sqrt(dot(z,z)/dot(dz,dz))*log(dot(z,z));
	//if( di>0.5 ) d=0.0;
}

void MandelTask()
{
	volatile int *sharedmem = (volatile int*)E32GetScratchpad();
	volatile float *g_R = (float*)(sharedmem);
	volatile uint32_t *image = (uint32_t*)(sharedmem+4);
	volatile float *g_X[2] = {(float*)(sharedmem+8), (float*)(sharedmem+24)};
	volatile float *g_Y[2] = {(float*)(sharedmem+12), (float*)(sharedmem+26)};
	volatile int *g_tX[2] = {sharedmem+16, sharedmem+30};
	volatile int *g_tY[2] = {sharedmem+20, sharedmem+34};
	uint8_t *framebuffer = (uint8_t *)(*image);

	uint32_t self = read_csr(mhartid);
	do
	{
		int tilex = *g_tX[self];
		int tiley = *g_tY[self];

		// Draw one tile
		float X =  *g_X[self];
		float Y =  *g_Y[self];
		float R =  *g_R;
		mandelbrotFloat(framebuffer, tilex, tiley, self, X, Y, R);

		// Here is where load acquire / store conditional would be useful
		tilex++;
		if (tilex >= 20)
		{
			tilex = 0;
			tiley += 2; // Skip every other line (two tasks)
		}
		if (tiley >= 15)
		{
			tiley = self; // 0 or 1 depending on task number
		}

		*g_tX[self] = tilex;
		*g_tY[self] = tiley;

		TaskYield();
	} while (1);
}

int main()
{
	// Set up frame buffer
	// NOTE: Video scanout buffer has to be aligned at 64 byte boundary
	uint8_t *framebuffer = VPUAllocateBuffer(320*240);
	struct EVideoContext vx;
	vx.m_vmode = EVM_320_Wide;
	vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);
	VPUSetWriteAddress(&vx, (uint32_t)framebuffer);
	VPUSetScanoutAddress(&vx, (uint32_t)framebuffer);
	VPUClear(&vx, 0x03030303);

	// Grayscale palette
	for (uint32_t i=0; i<256; ++i)
	{
		int j = (255-i)>>4;
		VPUSetPal(i, j, j, j);
	}

	float R = 4.0E-5f + 0.01f; // Step once to see some detail due to adaptive code
	float X = -0.235125f;
	float Y = 0.827215f;

	printf("Mandelbrot test\n");

	// Set up our shared memory
	volatile int *sharedmem = (volatile int*)E32GetScratchpad();
	volatile float *g_R = (float*)(sharedmem);
	volatile uint32_t *image = (uint32_t*)(sharedmem+4);
	volatile float *g_X[2] = {(float*)(sharedmem+8), (float*)(sharedmem+24)};
	volatile float *g_Y[2] = {(float*)(sharedmem+12), (float*)(sharedmem+26)};
	volatile int *g_tX[2] = {sharedmem+16, sharedmem+30};
	volatile int *g_tY[2] = {sharedmem+20, sharedmem+34};
	*image = (uint32_t)framebuffer;

	// Set up task contexts
	struct STaskContext *taskctx[MAX_HARTS] = { TaskGetContext(0), TaskGetContext(1)};

	// Set up initial values
	// Each task will start at a different line
	*g_X[0] = X; *g_Y[0] = Y; *g_tX[0] = 0; *g_tY[0] = 0;
	*g_X[1] = X; *g_Y[1] = Y; *g_tX[1] = 0; *g_tY[1] = 1;
	*g_R = R;

	// Start tasks
	uint32_t* stackA = new uint32_t[1024];
	uint32_t* stackB = new uint32_t[1024];
	int taskID0 = TaskAdd(taskctx[0], "MandelTask", MandelTask, TS_RUNNING, ONE_MILLISECOND_IN_TICKS, (uint32_t)stackA);
	int taskID1 = TaskAdd(taskctx[1], "MandelTask", MandelTask, TS_RUNNING, ONE_MILLISECOND_IN_TICKS, (uint32_t)stackB);

	while(1)
	{
		// Zoom in when last task reaches the end
		if (*g_tY[1] == 1 && *g_tX[1] == 0)
		{
			*g_R = *g_R + 0.001f;
		}
		TaskYield();
	}

	TaskExitTaskWithID(taskctx[0], taskID0, 0);
	TaskExitTaskWithID(taskctx[1], taskID1, 0);

	return 0;
}
