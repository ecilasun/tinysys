#include <stdio.h>
#include <string.h>
#include "scratchpadmem.h"

static const uint32_t quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

CScratchpadMem::CScratchpadMem()
{
	// Scratchpad memory is 64KBytes
	m_scratchmem = (uint32_t*)malloc(16384 * sizeof(uint32_t));
}

CScratchpadMem::~CScratchpadMem()
{
	free(m_scratchmem);
}

void CScratchpadMem::Reset()
{
	// Clear memory
	memset(m_scratchmem, 0, 16384 * sizeof(uint32_t));
}

void CScratchpadMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t memslot = (address >> 2) & 0xFFFF;
	data = m_scratchmem[memslot];
}

void CScratchpadMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t memslot = (address >> 2) & 0xFFFF;
	uint32_t olddata = m_scratchmem[memslot];

	// Expand the wstrobe
	uint32_t fullmask = quadexpand[wstrobe];
	uint32_t invfullmask = ~fullmask;

	// Mask and mix incoming and old data
	uint32_t newword = (olddata & invfullmask) | (word & fullmask);
	m_scratchmem[memslot] = newword;
}
