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
    uint32_t FetchInstruction(uint32_t address);
    uint32_t FetchDataWord(uint32_t address);
};
