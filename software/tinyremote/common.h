#pragma once

#include <stdint.h>

uint8_t GetMask(int index);
uint8_t GetScancode(int index);

uint32_t YUVtoRGBX32(int y, int u, int v);

struct Axis6
{
	float leftx;
	float lefty;
	float rightx;
	float righty;
	float lefttrigger;
	float righttrigger;
};
