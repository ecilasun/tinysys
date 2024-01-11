#include "clock.h"

void CClock::Reset()
{
    m_clock = 0x0000FFFF;
    m_edge = NoEdge;
}

void CClock::Step()
{
    m_clock ^= 0xFFFFFFFF;
    m_edge = m_clock == 0x0000FFFF ? RisingEdge : FallingEdge;
}
