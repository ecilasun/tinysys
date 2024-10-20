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
	m_count = 0;
}

void CVPU::UpdateVideoLink(uint32_t* pixels, int pitch, CBus* bus)
{
	if (m_scanoutpointer)
	{
		uint32_t* devicemem = bus->GetHostAddress(m_scanoutpointer);
		if (m_videoscanoutenable && devicemem)
		{
			// Copy vram scan out pointer contents to SDL surface
			if (m_12bppmode)
			{
				if (m_scanwidth == 320)
				{
					// 16bpp
					uint16_t* devicememas12bpp = (uint16_t*)devicemem;
					const int W = pitch / 4;
					for (uint32_t y = 0; y < m_scanheight; y++)
					{
						const int linetop0 = 2 * W * y;
						const int linetop1 = 2 * W * y + W;
						uint16_t* sourceRow = &devicememas12bpp[m_scanwidth * y];
						for (uint32_t x = 0; x < m_scanwidth; ++x)
						{
							uint16_t color = sourceRow[x];
							uint32_t G = SelectBitRange(color, 3, 0);
							uint32_t B = SelectBitRange(color, 7, 4);
							uint32_t R = SelectBitRange(color, 11, 8);
							uint32_t expandedcolor = 0xFF000000 | (R << 20) | (G << 12) | (B << 4);
							pixels[linetop0 + x*2+0] = expandedcolor;
							pixels[linetop1 + x*2+0] = expandedcolor;
							pixels[linetop0 + x*2+1] = expandedcolor;
							pixels[linetop1 + x*2+1] = expandedcolor;
						}
					}
				}
				else
				{
					// 16bpp
					uint16_t* devicememas12bpp = (uint16_t*)devicemem;
					const int W = pitch / 4;
					for (uint32_t y = 0; y < m_scanheight; y++)
					{
						const int linetop = W * y;
						uint16_t* sourceRow = &devicememas12bpp[m_scanwidth * y];
						for (uint32_t x = 0; x < m_scanwidth; ++x)
						{
							uint16_t color = sourceRow[x];
							uint32_t G = SelectBitRange(color, 3, 0);
							uint32_t B = SelectBitRange(color, 7, 4);
							uint32_t R = SelectBitRange(color, 11, 8);
							uint32_t expandedcolor = 0xFF000000 | (R << 20) | (G << 12) | (B << 4);
							pixels[linetop + x] = expandedcolor;
						}
					}
				}
			}
			else
			{
				if (m_scanwidth == 320)
				{
					// 8bpp
					uint8_t* devicememas8bpp = (uint8_t*)devicemem;
					const int W = pitch / 4;
					for (uint32_t y = 0; y < m_scanheight; y++)
					{
						uint32_t* pixelRow0 = &pixels[2*W*y];
						uint32_t* pixelRow1 = pixelRow0 + W;
						uint8_t* sourceRow = &devicememas8bpp[m_scanwidth * y];
						for (uint32_t x = 0; x < m_scanwidth; ++x)
						{
							uint32_t color = m_vgapalette[sourceRow[x]];
							uint32_t *pixelPos0 = pixelRow0+(x<<1);
							uint32_t *pixelPos1 = pixelRow1+(x<<1);
							pixelPos0[0] = color;
							pixelPos0[1] = color;
							pixelPos1[0] = color;
							pixelPos1[1] = color;
						}
					}
				}
				else
				{
					// 8bpp
					uint8_t* devicememas8bpp = (uint8_t*)devicemem;
					const int W = pitch / 4;
					for (uint32_t y = 0; y < m_scanheight; y++)
					{
						uint32_t* pixelRow = &pixels[W*y];
						uint8_t* sourceRow = &devicememas8bpp[m_scanwidth * y];
						for (uint32_t x = 0; x < m_scanwidth; ++x)
						{
							uint32_t color = m_vgapalette[sourceRow[x]];
							pixelRow[x] = color;
						}
					}
				}
			}
		}
		else
		{
			// This is going to reset evrything including the 8 pixel status bar
			for (uint32_t i = 0; i < 640 * 488; i++)
				pixels[i] = (i%2) ? 0xFF201515 : 0xFF001515;
		}
	}

	if (m_scanoutpointer == 0x0 || m_videoscanoutenable == 0x0)
	{
		// This is going to reset evrything including the 8 pixel status bar
		for (uint32_t i = 0; i < 640 * 488; i++)
			pixels[i] = ((i/640)%2) ? 0xFF151515 : 0xFF000015;
	}

	// This function is invoked at 13ms intervals, so this counter acts as our vsync
	++m_count;
}

void CVPU::Tick(CBus* bus)
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
			//printf("scanout @%.8X\n", m_scanoutpointer);
			m_state = 0;
		}
		break;

		case 3:
		{
			// SETPAL
			uint32_t addrs = SelectBitRange(m_data, 31, 24);
			uint32_t color = SelectBitRange(m_data, 11, 0);
			// Convert 12bpp color to 32bpp ARGB
			uint32_t G = SelectBitRange(color, 3, 0) << 4;
			uint32_t B = SelectBitRange(color, 7, 4) << 4;
			uint32_t R = SelectBitRange(color, 11, 8) << 4;
			m_vgapalette[addrs] = 0xFF000000 | (R << 16) | (G << 8) | (B);
			m_state = 0;
		}
		break;

		case 4:
		{
			// VMODE
			m_videoscanoutenable = m_data & 0x1 ? 1 : 0;	// 0:video output disabled, 1:video output enabled
			m_12bppmode = m_data & 0x4 ? 1 : 0;				// 0:8bit indexed, 1:16bit rgb
			m_scanwidth = m_data&0x2 ? 640 : 320;			// 0:320-wide, 1:640-wide
			m_scanheight = m_data&0x2 ? 480 : 240;			// 0:240-tall, 1:480-tall
			m_scanlength = m_scanwidth * m_scanheight;
			if (m_12bppmode)
				m_scanlength *= 2;
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
	// Every time we call UpdateVideoLink we increment m_count, so we can use this as a vsync signal
	data = m_count%2;
}

void CVPU::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// Command FIFO writes dirty the video output
	m_fifo.push(word);
}
