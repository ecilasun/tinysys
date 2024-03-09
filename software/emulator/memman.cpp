#include <stdio.h>
#include "memman.h"

static const uint32_t quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

CMemMan::CMemMan()
{
	// Warning! Allocating 256Mbytes in one go!
	m_devicemem = malloc(256*1024*1024);
}

CMemMan::~CMemMan()
{
    free(m_devicemem);
}

void CMemMan::Tick(CClock& cpuclock)
{

}

void CMemMan::CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size)
{
	uint32_t *ddr3 = (uint32_t*)m_devicemem;
	uint32_t *rom = (uint32_t*)bin;
	// Convert from cache byte order to memory byte order
	uint32_t base = resetvector>>2;
	for (uint32_t i=0; i<size/4; ++i)
		ddr3[base+i] = rom[i];
	printf("ROM @%.8x (%.8x bytes)\n", resetvector, size);
}

uint32_t CMemMan::FetchInstruction(uint32_t address)
{
	// TODO: Return from I$ instead for consistency of simulation
	uint32_t instruction;
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	instruction = wordmem[address>>2];
	return instruction;
}

uint32_t CMemMan::FetchDataWord(uint32_t address)
{
	// TODO: Return from D$ instead for consistency of simulation
	uint32_t data;
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	data = wordmem[address>>2];
	return data;
}

void CMemMan::WriteDataWord(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// TODO: Use D$ instead for consistency of simulation
	uint32_t olddata;
	uint32_t *wordmem = (uint32_t*)m_devicemem;
	olddata = wordmem[address>>2];

	// Expand the wstrobe
	uint32_t fullmask = quadexpand[wstrobe];
	uint32_t invfullmask = ~fullmask;

	// Mask and mix incoming and old data
	wordmem[address>>2] = (olddata&invfullmask) | (word*fullmask);
}
