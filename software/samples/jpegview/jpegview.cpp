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

// The Bayer matrix for ordered dithering
const uint8_t dither[4][4] = {
{ 0, 8, 2,10},
{12, 4,14, 6},
{ 3,11, 1, 9},
{15, 7,13, 5}
};

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

			int iW = W>=640 ? 639 : W;
			int iH = H>=480 ? 479 : H;

			uint8_t *img = njGetImage();
			if (njIsColor())
			{
				// Copy, dither and convert to indexed color
				for (int y=0;y<iH;++y)
				{
					for (int x=0;x<iW;++x)
					{
						uint8_t R = img[(x+y*W)*3+0]>>4;
						uint8_t G = img[(x+y*W)*3+1]>>4;
						uint8_t B = img[(x+y*W)*3+2]>>4;
						image[x+y*640] = MAKECOLORRGB12(R,G,B);
					}
				}
			}
			else
			{
				// Grayscale
				for (int j=0;j<iH;++j)
					for (int i=0;i<iW;++i)
					{
						uint8_t V = img[i+j*W];
						image[i+j*640] = MAKECOLORRGB12(V,V,V);
					}
			}
			// Finish memory writes to display buffer
			CFLUSH_D_L1;
		}
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
