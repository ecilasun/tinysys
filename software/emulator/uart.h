#pragma once

#include <stdint.h>
#include <deque>
#include "memmappeddevice.h"

class CUART : public MemMappedDevice
{
public:
	CUART() {}
	~CUART() {}

	uint32_t m_uartirq{ 0 };
	uint32_t m_controlword{ 0 };
	uint32_t m_clockSkip{ 0 };

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
	void Read16(uint32_t address, uint32_t* data) override final { data[0] = 0; }
	void Write16(uint32_t address, uint32_t* data) override final {}
	void Tick(CBus* bus);

	std::deque<uint8_t> m_byteinqueue;
	std::deque<uint8_t> m_byteoutqueue;
	void QueueByte(uint8_t byte);
};
