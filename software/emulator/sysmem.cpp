#include <stdio.h>
#include <string.h>
#include "sysmem.h"
#include "memmappeddevice.h"

#if defined(CAT_WINDOWS)
#include <pmmintrin.h>
#endif

static const uint32_t quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

CSysMem::CSysMem()
{
	// Warning! Allocating 256Mbytes in one go!
	m_devicemem = malloc(256*1024*1024);
}

CSysMem::~CSysMem()
{
	free(m_devicemem);
}

void CSysMem::Reset()
{
	memset(m_devicemem, 0xCC, 256*1024*1024);
}

void CSysMem::Tick(CBus* bus)
{
}

uint32_t* CSysMem::GetHostAddress(uint32_t address)
{
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	return &wordmem[address>>2];
}

void CSysMem::CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size)
{
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	uint32_t *rom = (uint32_t*)bin;
	// Convert from cache byte order to memory byte order
	uint32_t base = resetvector>>2;
	for (uint32_t i=0; i<size/4; ++i)
		wordmem[base+i] = rom[i];
}

void CSysMem::Read(uint32_t address, uint32_t& data)
{
	// TODO: Return from D$ instead for consistency of simulation
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	data = wordmem[address>>2];
}

void CSysMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// TODO: Use D$ instead for consistency of simulation
	uint32_t olddata;
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	olddata = wordmem[address>>2];

	// Expand the wstrobe
	uint32_t fullmask = quadexpand[wstrobe];
	uint32_t invfullmask = ~fullmask;

	// Mask and mix incoming and old data
	wordmem[address>>2] = (olddata&invfullmask) | (word&fullmask);
}

void CSysMem::Read16(uint32_t address, uint32_t* data)
{
#if defined(CAT_WINDOWS)
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	__m128i *source = (__m128i *)&wordmem[address>>2];
	__m128i *target = (__m128i *)data;
	for (int i=0; i<4; ++i)
		target[i] = source[i];
#else
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	uint64_t *source = (uint64_t *)&wordmem[address>>2];
	uint64_t *target = (uint64_t *)data;
	for (int i=0; i<8; ++i)
		target[i] = source[i];
#endif
}

void CSysMem::Write16(uint32_t address, uint32_t* data)
{
#if defined(CAT_WINDOWS)
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	__m128i *target = (__m128i *)&wordmem[address>>2];
	__m128i *source = (__m128i *)data;
	for (int i=0; i<4; ++i)
		target[i] = source[i];
#else
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	uint64_t *target = (uint64_t *)&wordmem[address>>2];
	uint64_t *source = (uint64_t *)data;
	for (int i=0; i<8; ++i)
		target[i] = source[i];
#endif
}
