#pragma once

#include <stdint.h>
#include <queue>
#include "memmappeddevice.h"

class CUART : public MemMappedDevice
{
public:
	CUART() {}
	~CUART() {}

	uint32_t m_uartirq{ 0 };
	uint32_t m_controlword{ 0 };

	void Reset() override final;
	void Tick(CBus* bus) override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;

	std::queue<uint8_t> m_byteinqueue;
	std::queue<uint8_t> m_byteoutqueue;
	void QueueByte(uint8_t byte);
};
