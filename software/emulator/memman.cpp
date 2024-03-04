#include "memman.h"

CMemMan::CMemMan()
{
	// Warning! Allocating 256Mbytes in one go!
	m_devicemem = malloc(256*1024*1024);
}

CMemMan::~CMemMan()
{
    free(m_devicemem);
}

uint32_t CMemMan::FetchInstruction(uint32_t address)
{
	// TODO: Return from I$

	return 0;
}

uint32_t CMemMan::FetchDataWord(uint32_t address)
{
	// TODO: Return from D$

	return 0;
}
