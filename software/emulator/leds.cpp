#include <stdio.h>
#include "leds.h"

void CLEDs::Reset()
{
}

void CLEDs::Tick()
{
}

void CLEDs::Read(uint32_t address, uint32_t& data)
{
	data = m_ledstate;
}

void CLEDs::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	m_ledstate = word;
}
