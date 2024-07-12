#include <stdio.h>
#include "leds.h"

void CLEDs::Reset()
{
}

void CLEDs::Read(uint32_t address, uint32_t& data)
{
	data = m_ledstate;
}

void CLEDs::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	m_ledstate = word;
	//printf("LEDs: %c%c%c%c (%d)\n", m_ledstate & 8 ? 'O' : '_', m_ledstate & 4 ? 'O' : '_', m_ledstate & 2 ? 'O' : '_', m_ledstate & 1 ? 'O' : '_', m_ledstate);
}
