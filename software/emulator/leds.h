#pragma once

#include <stdint.h>

class CLEDs
{
public:
	CLEDs() {}
	~CLEDs() {}

	uint32_t m_ledstate = 0;

	void Reset();
	void Tick();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
