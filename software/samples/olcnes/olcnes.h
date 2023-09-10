#pragma once
#include "gpu.h"

namespace olc
{
	struct Pixel
	{
		Pixel() { val = 0x0; }
		Pixel& operator = (const Pixel& v) = default;
		Pixel(const uint8_t r, const uint8_t g, const uint8_t b)
		{
			uint16_t G = (uint16_t)(g>>2);
			uint16_t R = (uint16_t)(r>>3);
			uint16_t B = (uint16_t)(b>>3);
			val = MAKECOLORRGB16(R, G, B);
		}
		uint16_t val;
	};

	struct Sprite
	{
		enum Mode { NORMAL, PERIODIC, CLAMP };
		enum Flip { NONE = 0, HORIZ = 1, VERT = 2 };

		int32_t width = 0;
		int32_t height = 0;
		uint16_t *data = nullptr;

		Sprite(const int w, const int h)
		{
			width = w;
			height = h;
			data = new uint16_t[w*h];
		}
		~Sprite()
		{
			delete data;
		}
		void SetPixel(const int x, const int y, Pixel& pixel)
		{
			data[x+y*width] = pixel.val;
		}
	};
}
