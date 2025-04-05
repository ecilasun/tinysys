#include <stdio.h>
#include <string.h>
#include "apu.h"
#include "bus.h"
#include "bitutil.h"

CAPU::CAPU()
{
	m_audioData[0] = (uint32_t*)malloc(0x1000*sizeof(uint32_t));
	m_audioData[1] = (uint32_t*)malloc(0x1000*sizeof(uint32_t));

	memset(m_audioData[0], 0, 0x1000*sizeof(uint32_t));
	memset(m_audioData[1], 0, 0x1000*sizeof(uint32_t));
}

CAPU::~CAPU()
{
}

void CAPU::Reset()
{
	m_currentbuffer	= 0;
}

void CAPU::Tick(CBus* bus)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	// Pull cmd from fifo and process
	switch (m_state)
	{
		case 0:
		{
			// Idle
			if (!m_fifo.empty())
			{
				m_state = 1;
				m_cmd = m_fifo.front();
				m_fifo.pop();
			}
		}
		break;

		case 1:
		{
			// Dispatch
			switch (m_cmd)
			{
				case APUCMD_BUFFERSIZE:
				{
					// APUBUFFERSIZE
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_apuwordcount = SelectBitRange(m_data, 9,0)+1;
						m_state = 0;
					}
				}
				break;
				case APUCMD_START:
				{
					// APUSTART
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_sourceAddress = m_data;
						m_state = 2;
					}
				}
				break;
				case APUCMD_SETRATE:
				{
					// APUSETRATE
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						switch (SelectBitRange(m_data, 1, 0))
						{
							case 0b00: m_rateselector = 0b0100; break; // 44.1KHz
							case 0b01: m_rateselector = 0b0010; break; // 22.05KHz
							case 0b10: m_rateselector = 0b0001; break; // 11.025KHz
							case 0b11: m_rateselector = 0b0000; break; // Quiet
						}
						m_state = 0;
					}
				}
				break;
				default:
					m_state = 0;
				break;
			}
		}
		break;

		case 2:
		{
			// Kick audio data copy
			uint32_t* target = m_audioData[m_currentbuffer ^ 1];
			//printf("emit audio from buffer#%d at %llx for %d words\n", m_currentbuffer^1, (uint64_t)target, m_apuwordcount);
			uint32_t addr = m_sourceAddress;
			for (uint32_t i = 0; i < m_apuwordcount; i++)
			{
				uint32_t data;
				bus->Read(addr, data);
				*target++ = data;
				addr += 4;
			}
			m_state = 0;
		}
		break;

		default:
			// Unknown state / finalize
			m_state = 0;
		break;
	}
}

void CAPU::Read(uint32_t address, uint32_t& data)
{
	// NOTE: Flip m_currentbuffer from audio callback
	data = m_currentbuffer;
}

void CAPU::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// Command FIFO writes dirty the video output
	std::lock_guard<std::mutex> lock(m_mutex);
	m_fifo.push(word);
}
