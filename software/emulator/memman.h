#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"

class CMemMan
{
public:
    CMemMan();
    ~CMemMan();

    void *m_devicemem;

    void Tick(CClock& cpuclock);
    void CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size);
    uint32_t FetchInstruction(uint32_t address);
    uint32_t FetchDataWord(uint32_t address);
	void WriteDataWord(uint32_t address, uint32_t word, uint32_t wstrobe);
};
