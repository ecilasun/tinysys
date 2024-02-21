#pragma once

#define TRU_HARDWARE

#include <inttypes.h>

#define RASTERCMD_OUTADDRS			0x00000000
#define RASTERCMD_PUSHVERTEX0		0x00000001
#define RASTERCMD_PUSHVERTEX1		0x00010001
#define RASTERCMD_PUSHVERTEX2		0x00020001
#define RASTERCMD_SETRASTERCOLOR	0x00000002
#define RASTERCMD_RASTERIZEPRIM		0x00000003
#define RASTERCMD_FLUSHCACHE		0x00001003
#define RASTERCMD_INVALIDATECACHE	0x00002003
#define RASTERCMD_BARRIER			0x00000004

#pragma pack(push,1)
struct SPrimitive
{
	int16_t x0,y0;
	int16_t x1,y1;
	int16_t x2,y2;
};
#pragma pack(pop)

struct TRUVec2
{
	int32_t x, y;
};

struct ERasterizerContext
{
	uint32_t m_rasterOutAddrressCacheAligned;
	uint32_t m_color;
	int32_t m_minx, m_maxx;
	int32_t m_miny, m_maxy;
	int32_t a01, a12, a20;
	int32_t b01, b12, b20;
	int32_t w0_row;
	int32_t w1_row;
	int32_t w2_row;
	struct TRUVec2 v0, v1, v2;
};

// Hardware rasterizer
void TRUSetTileBuffer(struct ERasterizerContext *_rc, const uint32_t _TRUTileBuffer16ByteAligned);
void TRUPushPrimitive(struct ERasterizerContext *_rc, struct SPrimitive* _primitive);
void TRURasterizePrimitive(struct ERasterizerContext *_rc);
void TRUSetColor(struct ERasterizerContext *_rc, const uint8_t _colorIndex);
void TRUFlushCache(struct ERasterizerContext *_rc);
void TRUInvalidateCache(struct ERasterizerContext *_rc);
void TRUBarrier(struct ERasterizerContext *_rc);
uint32_t TRUPending(struct ERasterizerContext *_rc);
void TRUWait(struct ERasterizerContext *_rc);
