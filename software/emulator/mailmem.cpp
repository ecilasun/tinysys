#include <stdio.h>
#include <string.h>
#include "mailmem.h"

CMailMem::CMailMem()
{
	// 1024 words
	m_mailmem = (uint32_t*)malloc(1024 * sizeof(uint32_t));
}

CMailMem::~CMailMem()
{
	free(m_mailmem);
}

void CMailMem::Reset()
{
	// Clear memory
	memset(m_mailmem, 0, 1024 * sizeof(uint32_t));
}

void CMailMem::Tick()
{
}

void CMailMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t mailslot = (address >> 2) & 0xFFF;
	data = m_mailmem[mailslot];
}

void CMailMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t mailslot = (address>>2) & 0xFFF;
	m_mailmem[mailslot] = word;
}
