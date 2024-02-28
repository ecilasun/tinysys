#pragma once

#include "clock.h"
#include "rv32.h"

class CEmulator
{
public:
    CEmulator() {}
    ~CEmulator() {}

    void Reset();
    bool Step();

    CClock m_clock;
	CRV32 m_cpu;
};
