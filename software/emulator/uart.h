#pragma once

#include <stdint.h>
#include <queue>

class CUART
{
public:
	CUART() {}
	~CUART() {}

	uint32_t m_uartirq{ 0 };
	uint32_t m_controlword{ 0 };

	void Reset();
	void Tick();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

	std::queue<uint8_t> m_byteinqueue;
	std::queue<uint8_t> m_byteoutqueue;
	void QueueByte(uint8_t byte);
};
