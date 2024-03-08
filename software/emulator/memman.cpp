#include <stdio.h>
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

void CMemMan::Tick(CClock& cpuclock)
{

}

void CMemMan::CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size)
{
	uint8_t *ddr3 = (uint8_t*)m_devicemem;
	for (uint32_t i=0; i<size; ++i)
		ddr3[i] = bin[i];
	printf("ROM @%.8x (%.8x bytes)\n", resetvector, size);
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
