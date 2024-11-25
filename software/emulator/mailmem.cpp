#include <stdio.h>
#include <string.h>
#include "mailmem.h"

static const uint32_t quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

CMailMem::CMailMem()
{
	// Mailbox memory is 16Kbytes
	m_mailmem = (uint32_t*)malloc(4096 * sizeof(uint32_t));
}

CMailMem::~CMailMem()
{
	free(m_mailmem);
}

void CMailMem::Reset()
{
	// Clear memory
	memset(m_mailmem, 0, 4096 * sizeof(uint32_t));
}

void CMailMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t mailslot = (address >> 2);
	mailslot &= 0xFFF;
	data = m_mailmem[mailslot];
}

void CMailMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t olddata;
	uint32_t mailslot = (address >> 2);
	mailslot &= 0xFFF;
	olddata = m_mailmem[mailslot];

	// Expand the wstrobe
	uint32_t fullmask = quadexpand[wstrobe];
	uint32_t invfullmask = ~fullmask;

	// Mask and mix incoming and old data
	uint32_t newword = (olddata & invfullmask) | (word & fullmask);
	m_mailmem[mailslot] = newword;
}
