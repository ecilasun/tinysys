#pragma once

#include <stdint.h>

enum EClockEdge
{
    NoEdge,
    RisingEdge,
    FallingEdge
};

class CClock{
public:
    CClock() {}
    ~CClock() {}

    void Reset();
    void Step();

    uint32_t m_clock{0x0000FFFF};
    EClockEdge m_edge{NoEdge};
};
