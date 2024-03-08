#ifdef CAT_WINDOWS
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif
#include "emulator.h"

void CEmulator::Reset()
{
    m_clock.Reset();
	m_cpu.SetMem(&m_mem);
}

bool CEmulator::Step()
{
    m_clock.Step();

    // Wire up the clocks to each device
    m_mem.Tick(m_clock);
	m_cpu.Tick(m_clock);

    return false;
}
