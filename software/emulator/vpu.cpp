#include <stdio.h>
#include <string.h>
#include "vpu.h"
#include "bus.h"
#include "bitutil.h"

CVPU::CVPU()
{
}

CVPU::~CVPU()
{
}

void CVPU::Reset()
{
	m_scanoutpointer = 0x0;
	m_videoscanoutenable = 0;
}

void CVPU::UpdateVideoLink(uint32_t* pixels, CBus* bus)
{
	if (m_scanoutpointer)
	{
		if (m_videoscanoutenable)
		{
			// Copy vram scan out pointer contents to SDL surface
			if (m_indexedcolormode)
			{
				// 16bpp
				uint32_t* devicemem = bus->GetHostAddress(m_scanoutpointer);
				for (uint32_t i = 0; i < 640 * 480; i++)
					pixels[i] = devicemem[i];
			}
			else
			{
				for (uint32_t i = 0; i < 640 * 480; i++)
				{
					// 8bpp
					uint8_t* devicemem = (uint8_t*)bus->GetHostAddress(m_scanoutpointer);
					uint32_t color = m_vgapalette[devicemem[i]];
					pixels[i] = color;
				}
			}
		}
	}

	if (m_scanoutpointer == 0x0 || m_videoscanoutenable == 0x0)
	{
		for (uint32_t i = 0; i < 640 * 480; i++)
			pixels[i] = 0xFF0000FF;
	}
}

void CVPU::Tick(CClock& cpuclock)
{
	// Pull cmd from fifo and process
	switch (m_state)
	{
		case 0:
		{
			// Idle
			if (m_fifo.size())
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
				case 0x00000000:
				{
					// SETVPAGE
					if (m_fifo.size())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_state = 2;
					}
				}
				break;
				case 0x00000001:
				{
					// SETPAL
					if (m_fifo.size())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_state = 3;
					}
				}
				break;
				case 0x00000002:
				{
					// VMODE
					if (m_fifo.size())
					{
						m_data = m_fifo.front();
						m_fifo.pop();
						m_state = 4;
					}
				}
				break;
				default:
					m_state = 5;
				break;
			}
		}
		break;

		case 2:
		{
			// SETVPAGE
			m_scanoutpointer = m_data;
			m_state = 0;
		}
		break;

		case 3:
		{
			// SETPAL
			uint32_t addrs = SelectBitRange(m_data, 31, 24);
			uint32_t color = SelectBitRange(m_data, 11, 0);
			m_vgapalette[addrs] = color; // TODO: boost to 32 bit color
			m_state = 0;
		}
		break;

		case 4:
		{
			// VMODE
			m_videoscanoutenable = m_data & 0x1;	// 0:video output disabled, 1:video output enabled
			m_indexedcolormode = m_data & 0x4;		// 0:8bit indexed, 1:16bit rgb
			m_scanwidth = m_data&0x2 ? 320 : 640;	// 0:320-wide, 1:640-wide
			m_state = 0;
		}
		break;

		default:
			// Unknown state / finalize
			m_state = 0;
		break;
	}
}

void CVPU::Read(uint32_t address, uint32_t& data)
{
	// TODO: Command FIFO reads should return device status (vsync in this case)
	data = 0;
}

void CVPU::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// Command FIFO write
	m_fifo.push(word);
}
