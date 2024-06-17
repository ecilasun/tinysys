#pragma once

#include "bus.h"
#include "rv32.h"

class CEmulator
{
public:
	CEmulator() {}
	~CEmulator() {}

	bool Reset(const char* romFile);
	bool Step();
	void UpdateVideoLink(uint32_t* pixels, int pitch);
	void QueueByte(uint8_t byte);

	CBus m_bus;
	CRV32 m_cpu;

	uint8_t *m_rombin = nullptr;
	uint32_t m_romsize = 0;
};
