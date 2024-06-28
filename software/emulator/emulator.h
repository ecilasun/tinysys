#pragma once

#include "bus.h"
#include "rv32.h"

class CEmulator
{
public:
	CEmulator() {}
	~CEmulator();

	bool Reset(const char* romFile);
	bool Step();
	void UpdateVideoLink(uint32_t* pixels, int pitch);
	void QueueBytes(uint8_t *bytes, uint32_t count);
	void QueueByte(uint8_t byte);

#if defined(MEM_DEBUG)
	void FillMemBitmap(uint32_t* pixels);
#endif

	int m_audioDevice {0};

	CBus* m_bus{ nullptr };

	uint8_t* m_rombin{ nullptr };
	uint32_t m_romsize{ 0 };
	uint32_t m_steps{ 0 };
};
