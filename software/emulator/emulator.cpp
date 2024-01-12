#include "emulator.h"

void CEmulator::Reset()
{
    m_clock.Reset();
}

bool CEmulator::Step()
{
    m_clock.Step();

    // For each device
    if (m_clock.m_edge==RisingEdge)
    {
        // TODO:
        // We want the CPU to be able to boot the ROM image, and work as if it's the real deal
    }

    return false;
}
