#pragma once

#include "bus.h"
#include "rv32.h"

class CEmulator
{
public:
	CEmulator() {}
	~CEmulator();

	bool Reset(const char* romFile, uint32_t resetvector);
	void Step(uint64_t wallclock);
	void UpdateVideoLink(uint32_t* pixels, int pitch, int scanline);
	void QueueBytes(uint8_t *bytes, uint32_t count);
	void QueueByte(uint8_t byte);

	void AddBreakpoint(uint32_t cpu, uint32_t address);
	void RemoveBreakpoint(uint32_t cpu, uint32_t address);

	int m_audioDevice {0};

	int m_debugStop{ 0 };
	int m_debugAck{ 0 };

	CBus* m_bus{ nullptr };
	CRV32* m_cpu[2]{ nullptr, nullptr };

	uint8_t* m_rombin{ nullptr };
	uint32_t m_romsize{ 0 };
	uint32_t m_steps{ 0 };
};
