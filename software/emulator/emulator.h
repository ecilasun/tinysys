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
	void QueueBytes(uint8_t *bytes, uint32_t count);
	void QueueByte(uint8_t byte);

	bool IsVideoDirty(){ return m_bus.IsVideoDirty(); }
	void ClearVideoDirty() { m_bus.ClearVideoDirty(); }

	CBus m_bus;

	uint8_t* m_rombin{ nullptr };
	uint32_t m_romsize{ 0 };
	uint32_t m_steps{ 0 };
};
