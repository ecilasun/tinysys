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
    }

    return false;
}
