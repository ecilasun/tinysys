#include <stdio.h>
#include "bus.h"
#include "uart.h"

const uint32_t UARTRECEIVE = DEVICE_UART + 0x00;
const uint32_t UARTTRANSMIT = DEVICE_UART + 0x04;
const uint32_t UARTSTATUS = DEVICE_UART + 0x08;
const uint32_t UARTCONTROL = DEVICE_UART + 0x0C;

void CUART::Reset()
{
	m_byteinqueue = {};
	m_byteoutqueue = {};

	m_uartirq = 0;
	m_controlword = 0b10000; // Interrupts are enabled by default
}

void CUART::Tick(CBus* bus)
{
	std::unique_lock<std::mutex> lock(m_writeMutex);
	m_uartirq = m_byteinqueue.size() && (m_controlword&16) ? 1 : 0; // depends on interrupt enable

	int needflush = 0;
	while (m_byteoutqueue.size())
	{
		// Output to console
		printf("%c", m_byteoutqueue.front());
		m_byteoutqueue.pop_front();
		++needflush;
	}

	if (needflush)
		fflush(stdout);
}

void CUART::Read(uint32_t address, uint32_t& data)
{
	if (address == UARTRECEIVE)
	{
		// Note: Same as hardware, this will block until a byte is available
		// Read status register to check if a byte is available first
		while (m_byteinqueue.size() == 0) {}
		data = m_byteinqueue.front();
		m_byteinqueue.pop_front();
	}
	else if (address == UARTTRANSMIT)
	{
		// Can't read from transmit
		data = 0;
	}
	else if (address == UARTSTATUS)
	{
		// Report if queue is empty or not
		if (m_byteinqueue.size())
			data = 0x01;
		else
			data = 0x00;
	}
	else if (address == UARTCONTROL)
	{
		data = 0;
		data |= m_byteinqueue.size() ? 1 : 0; // data available
		//data |= x ? 2:0; // infifofull
		//data |= x ? 4:0; // outfifoempty
		//data |= x ? 8:0; // outfifofull
		//data |= x ? 16:0; // intenable
	}
	else
		data = 0;
}

void CUART::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	std::lock_guard<std::mutex> lock(m_writeMutex);
	if (address == UARTCONTROL)
	{
		m_controlword = word;
	}
	else if (address == UARTTRANSMIT)
	{
		// Write to transmit
		m_byteoutqueue.push_back(word & 0xFF);
	}
}

void CUART::QueueByte(uint8_t byte)
{
	m_byteinqueue.push_back(byte);
}
