#include <stdio.h>
#include <string.h>
#include "vpu.h"

CVPU::CVPU()
{
	m_vrampointer = nullptr;
}

CVPU::~CVPU()
{
}

void CVPU::Reset()
{
}

void CVPU::Tick(CClock& cpuclock)
{
	// Pull cmd from fifo and process
	if (m_fifo.size())
	{
		uint32_t cmd = m_fifo.front();
		switch (cmd)
		{
			case 0x00000000:
				// SETVPAGE
				break;
			case 0x00000001:
				// SETPAL
				break;
			case 0x00000002:
				// VMODE
				break;
			default:
				break;
		}
		m_fifo.pop();
	}

	// TODO: Update fake video output on SDL side
}

void CVPU::Read(uint32_t address, uint32_t& data)
{
	// TODO: command FIFO read will return device status (vsync in this case)
	data = 0;
}

void CVPU::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// Command FIFO write
	m_fifo.push(word);
}
