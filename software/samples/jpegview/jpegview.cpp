/** \file
 * JPEG image viewer example
 *
 * \ingroup examples
 * This example demonstrates how to decode a JPEG image and display it on the screen.
 * It uses the NanoJPEG library to decode the JPEG image.
 */

#include "core.h"
#include "vpu.h"
#include "sdcard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "nanojpeg.h"

uint16_t *image;

#define min(_x_,_y_) (_x_) < (_y_) ? (_x_) : (_y_)
#define max(_x_,_y_) (_x_) > (_y_) ? (_x_) : (_y_)

inline uint32_t ftoui4sat(float value)
{
  uint32_t retval;
  asm (
    "mv a1, %1;"
    ".insn 0xc2058553;" // fcvtswu4sat a0, a1 // note A0==cpu.x10, A1==cpu.x11
    "mv %0, a0; "
    : "=r" (retval)
    : "r" (value)
    : "a0", "a1"
  );
  return retval;
}

void DecodeJPEG(const char *fname)
{
	njInit();

	FILE *fp = fopen(fname, "rb");
	if (fp)
	{
		// Grab file size
		fpos_t pos, endpos;
		fgetpos(fp, &pos);
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &endpos);
		fsetpos(fp, &pos);
		uint32_t fsize = (uint32_t)endpos;

		printf("Reading %ld bytes\n", fsize);
		uint8_t *rawjpeg = (uint8_t *)malloc(fsize);
		fread(rawjpeg, fsize, 1, fp);
		fclose(fp);

		printf("Decoding image\n");
		nj_result_t jres = njDecode(rawjpeg, fsize);

		if (jres == NJ_OK)
		{
			int W = njGetWidth();
			int H = njGetHeight();

			int iW = W>=640 ? 640 : W;
			int iH = H>=480 ? 480 : H;

			uint8_t *img = njGetImage();
			if (njIsColor())
			{
				// Copy, dither and convert to indexed color
				for (int y=0;y<iH;++y)
				{
					for (int x=0;x<iW;++x)
					{
						uint32_t red = ftoui4sat(float(img[(x+y*W)*3+0])/255.f);
						uint32_t green = ftoui4sat(float(img[(x+y*W)*3+1])/255.f);
						uint32_t blue = ftoui4sat(float(img[(x+y*W)*3+2])/255.f);
						image[x+y*640] = MAKECOLORRGB12(red, green, blue);
					}
				}
			}
			else
			{
				// Grayscale
				for (int j=0;j<iH;++j)
					for (int i=0;i<iW;++i)
					{
						uint8_t V = img[i+j*W]>>4;
						image[i+j*640] = MAKECOLORRGB12(V,V,V);
					}
			}
			// Finish memory writes to display buffer
			CFLUSH_D_L1;
		}

		free(rawjpeg);
	}
	else
		printf("Could not open file %s\n", fname);

	njDone();
}
int main(int argc, char** argv )
{
	// Set aside space for the decompressed image
	// NOTE: Video scanout buffer has to be aligned at 64 byte boundary
	image = (uint16_t*)VPUAllocateBuffer(640*480*2);

	struct EVideoContext vx;
	vx.m_vmode = EVM_640_Wide;
	vx.m_cmode = ECM_16bit_RGB;
	VPUSetVMode(&vx, EVS_Enable);
	VPUSetWriteAddress(&vx, (uint32_t)image);
	VPUSetScanoutAddress(&vx, (uint32_t)image);
	VPUClear(&vx, 0x03030303);

	if (argc<=1)
		DecodeJPEG("sd:test.jpg");
	else
		DecodeJPEG(argv[1]);

	// Hold while we view the image
	while(1){}

	return 0;
}
