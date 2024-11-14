#include <stdio.h>
#include <string.h>
#include "dma.h"
#include "bus.h"
#include "bitutil.h"

CDMA::CDMA()
{
}

CDMA::~CDMA()
{
}

void CDMA::Reset()
{
}

void CDMA::Tick(CSysMem* mem)
{
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
				m_op = m_cmd & 0xFF;
			}
		}
		break;

		case 1:
		{
			// Dispatch
			switch (m_op)
			{
				case 0x00:
				{
					// DMASOURCE
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_dmasourceaddr = m_data;
						m_state = 0;
					}
				}
				break;
				case 0x01:
				{
					// DMATARGET
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_dmatargetaddr = m_data;
						m_state = 0;
					}
				}
				break;
				case 0x02:
				{
					// DMABURST
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_dmaburstcount = (m_data & 0xFF) + 1; // input is -1
						m_state = 0;
					}
				}
				break;
				case 0x03:
				{
					// DMASTART
					m_dmamaskmode = m_cmd & 0x100 ? 1 : 0;
					m_state = m_dmamaskmode ? 3 : 2;
				}
				break;
				case 0x04:
				{
					// DMATAG
					if (!m_fifo.empty())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_state = 0;
					}
				}
				break;
				default:
					m_state = 4;
				break;
			}
		}
		break;

		case 2:
		{
			// Each burst is 16 bytes in hardware (therefore 4 words)
			uint32_t dmalen = m_dmaburstcount;
			for (uint32_t i = 0; i < dmalen; ++i)
			{
				uint32_t data[4];

				uint32_t alignedSource = m_dmasourceaddr & ~0xF;
				mem->Read128bits(alignedSource, data);

				mem->Write128bits(m_dmatargetaddr, data);

				m_dmasourceaddr += 16;
				m_dmatargetaddr += 16;
			}
			m_state = 0;
		}
		break;

		case 3:
		{
			uint8_t *src = (uint8_t*)mem->GetHostAddress(m_dmasourceaddr);
			uint8_t *trg = (uint8_t*)mem->GetHostAddress(m_dmatargetaddr);
			if (m_dmamaskmode)
			{
				uint32_t dmalen = (m_dmaburstcount-1)*16;
				for (uint32_t i=0;i<dmalen;++i)
				{
					if (src[i]!=0) trg[i] = src[i];
				}
			}
			else
			{
				uint32_t dmalen = m_dmaburstcount*16;
				memcpy(trg, src, dmalen);
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

void CDMA::Read(uint32_t address, uint32_t& data)
{
	data = !m_fifo.empty() ? 1 : 0;
}

void CDMA::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// Command FIFO writes dirty the video output
	m_fifo.push(word);
}
