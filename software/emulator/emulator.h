#pragma once

#include "clock.h"
#include "bus.h"
#include "rv32.h"

class CEmulator
{
public:
    CEmulator() {}
    ~CEmulator() {}

    bool Reset(const char* romFile);
    bool Step();

    CClock m_clock;
    CBus m_bus;
	CRV32 m_cpu;

    uint8_t *m_rombin = nullptr;
    uint32_t m_romsize = 0;
};
