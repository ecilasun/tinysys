#include <SDL2/SDL.h>
#include "emulator.h"

void CEmulator::Reset()
{
    m_clock.Reset();
}

bool CEmulator::Step()
{
    m_clock.Step();

    // Wire up the clocks to each device
	m_cpu.Tick(m_clock);

    return false;
}
