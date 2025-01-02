#include <stdint.h>

static uint8_t masktable[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

static uint8_t keycodetoscancode[256] =
{
//  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x29, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, // 0
0x24, 0x25, 0x26, 0x27, 0x2d, 0x2e, 0x2a, 0x2b, 0x14, 0x1A, 0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, // 1
0x12, 0x13, 0x2f, 0x30, 0X28, 0xe0, 0x04, 0x16, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x33, // 2
0x34, 0x35, 0xe1, 0x31, 0x1d, 0x1b, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0xe5,    0, // 3
0xe2, 0x2c,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 4
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 5
    0,    0,    0,    0,    0,    0,    0,    0,    0, 0xe4,    0,    0, 0xe6,    0,    0, 0x52, // 6
    0, 0x50, 0x4f,    0, 0x51,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x03, // 7 // 7F -> Ctrl+C (0x03)
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 8
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 9
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // A
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // B
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // C
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // D
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // E
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // F
};

#define clamp(x) (x>255?255:(x<0?0:x))
#define YUV2RO(C, D, E) clamp((298 * (C) + 409 * (E) + 128) >> 8)
#define YUV2GO(C, D, E) clamp((298 * (C) - 100 * (D) - 208 * (E) + 128) >> 8)
#define YUV2BO(C, D, E) clamp((298 * (C) + 516 * (D) + 128) >> 8)

uint32_t YUVtoRGBX32(int y, int u, int v)
{
    y -= 16; u -= 128; v -= 128;
    return (YUV2RO(y,u,v) )<<16 | (YUV2GO(y,u,v) )<<8 | (YUV2BO(y,u,v) ) | 0xFF000000;
}

uint8_t GetMask(int index)
{
	return masktable[index&7];
}

uint8_t GetScancode(int index)
{
	return keycodetoscancode[index];
}