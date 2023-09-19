#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <cmath>

#include "basesystem.h"
#include "core.h"
#include "gpu.h"

uint8_t *framebuffer;

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

int tilex = 0;
int tiley = 0;

// http://blog.recursiveprocess.com/2014/04/05/mandelbrot-fractal-v2/
void mandelbrotFloat(float ox, float oy, float sx)
{
   int R = int(27.71f-5.156f*logf(sx));

   for (int y = 0; y < 16; ++y)
   {
      int row = y + tiley*16;
      for (int x = 0; x < 16; ++x)
      {
         int col = x + tilex*16;

         int M = evalMandel(R, col, row, ox, oy, sx);
         int c;
         if (M < 2)
         {
            c = 0;
         }
         else
         {
            float ratio = float(M)/float(R);
            c = int(1.f*ratio*255);
         }
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

int main()
{
   // Set up frame buffer
   // NOTE: Video scanout buffer has to be aligned at 64 byte boundary
   framebuffer = GPUAllocateBuffer(320*240);
   struct EVideoContext vx;
   vx.m_vmode = EVM_320_Wide;
   vx.m_cmode = ECM_8bit_Indexed;
   GPUSetVMode(&vx, EVS_Enable);
   GPUSetWriteAddress(&vx, (uint32_t)framebuffer);
   GPUSetScanoutAddress(&vx, (uint32_t)framebuffer);
   GPUClear(&vx, 0x03030303);

   // Grayscale palette
   for (uint32_t i=0; i<256; ++i)
   {
      int j = 255-i;
      GPUSetPal(i, j, j, j);
   }

   float R = 4.0E-5f + 0.01f; // Step once to see some detail due to adaptive code
   float X = -0.235125f;
   float Y = 0.827215f;

   printf("Mandelbrot test\n");

   while(1)
   {
      // Generate one line of mandelbrot into offscreen buffer
      // NOTE: It is unlikely that CPU write speeds can catch up with GPU DMA transfer speed, should not see a flicker
      mandelbrotFloat(X,Y,R);

      tilex++;
      if (tilex == 20)
      {
         tilex = 0;
         tiley++;
      }
      if (tiley == 15)
      {
         tiley = 0;
         // Flush leftover writes
         CFLUSH_D_L1;
         // Zoom
         R += 0.001f;
      }
   }

   return 0;
}
