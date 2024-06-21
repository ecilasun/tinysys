#include <stdio.h>
#include "bus.h"

CBus::CBus(uint32_t resetvector)
{
	m_resetvector = resetvector;

	m_csr[0] = new CCSRMem(0);
	m_csr[1] = new CCSRMem(1);

	m_cpu[0] = new CRV32(0, resetvector);
	m_cpu[1] = new CRV32(1, resetvector);
}

CBus::~CBus()
{
	delete m_csr[0];
	delete m_csr[1];
	if (m_cpu[0]) delete m_cpu[0];
	if (m_cpu[1]) delete m_cpu[1];
}

void CBus::Reset(uint8_t* rombin, uint32_t romsize)
{
	m_mem.Reset();
	m_mem.CopyROM(m_resetvector, rombin, romsize);

	m_sdcc.Reset();
	m_vpuc.Reset();
	m_uart.Reset();

	m_csr[0]->Reset();
	m_csr[1]->Reset();

	if (m_cpu[0]) m_cpu[0]->Reset();
	if (m_cpu[1]) m_cpu[1]->Reset();
}

void CBus::UpdateVideoLink(uint32_t *pixels, int pitch)
{
	m_vpuc.UpdateVideoLink(pixels, pitch, this);
}

#if defined(MEM_DEBUG)
void CBus::FillMemBitmap(uint32_t* pixels)
{
	uint32_t* source = m_mem.GetHostAddress(0);

	// System memory, 1 pixel per 1Kbytes of memory
	for (uint32_t i = m_busactivitystart; i < m_busactivityend; ++i)
	{
		uint32_t mix = 0;
		for (uint32_t j = 0; j < 256; ++j)
			mix = (mix << 8) ^ source[i * 256 + j];
		pixels[i] = mix | 0xFF000000;
	}

	m_busactivitystart = 0xFFFFFFFF;
	m_busactivityend = 0x00000000;
}
#endif

void CBus::QueueByte(uint8_t byte)
{
	m_uart.QueueByte(byte);
}

bool CBus::Tick()
{
	bool ret0 = true;
	if (m_cpu[0])
	{
		ret0 = m_cpu[0]->Tick(this);
		m_csr[0]->Tick(m_cpu[0], &m_uart);
	}

	bool ret1 = true;
	if (m_cpu[1])
	{
		ret1 = m_cpu[1]->Tick(this);
		m_csr[1]->Tick(m_cpu[1], &m_uart);
	}

	m_mem.Tick();
	m_vpuc.Tick();
	m_uart.Tick();
	m_sdcc.Tick();

	return ret0 && ret1;
}

uint32_t* CBus::GetHostAddress(uint32_t address)
{
	// Convert to emulator host address from emulated device memory address
	if (address & 0x80000000)
		return nullptr;
	else
		return m_mem.GetHostAddress(address);
}

void CBus::Read(uint32_t address, uint32_t& data)
{
	uint32_t dev = (address & 0xF0000) >> 16;

	if (address & 0x80000000)
	{
		switch (dev)
		{
			case 0:
			{
				// DEVICE_GPIO
				//m_gpio->Read(address, data);
				printf("<-GPIO\n");
				data = 0;
			}
			break;
			case 1:
			{
				m_leds.Read(address, data);
			}
			break;
			case 2:
			{
				m_vpuc.Read(address, data);
			}
			break;
			case 3:
			{
				m_sdcc.Read(address, data);
			}
			break;
			case 4:
			{
				// DEVICE_XADC
				//m_xadc->Read(address, data);
				printf("<-ADC\n");
				data = 0;
			}
			break;
			case 5:
			{
				// DEVICE_DMAC
				//m_dmac->Read(address, data);
				printf("<-DMA\n");
				data = 0;
			}
			break;
			case 6:
			{
				// DEVICE_USBA
				//m_usba->Read(address, data);
				//printf("<-USB-A\n");
				data = 0xFF; // SPI access should return FF for no device present
			}
			break;
			case 7:
			{
				// DEVICE_APUC
				//m_apuc->Read(address, data);
				printf("<-APU\n");
				data = 0;
			}
			break;
			case 8:
			{
				m_mail.Read(address, data);
			}
			break;
			case 9:
			{
				m_uart.Read(address, data);
			}
			break;
			case 0xA:
			{
				m_csr[0]->Read(address, data);
			}
			break;
			case 0xB:
			{
				m_csr[1]->Read(address, data);
			}
			break;
			default:
			{
				printf("Unknown device read @0x%.8X\n", address);
				data = 0;
			}
		}
	}
	else
		m_mem.Read(address, data);
}

void CBus::Write(uint32_t address, uint32_t data, uint32_t wstrobe)
{
	if (address & 0x80000000)
	{
		uint32_t dev = (address & 0xF0000) >> 16;
		switch (dev)
		{
			case 0:
			{
				// DEVICE_GPIO
				//m_gpio->Write(address, data, wstrobe);
				printf("GPIO@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 1:
			{
				m_leds.Write(address, data, wstrobe);
			}
			break;
			case 2:
			{
				m_vpuc.Write(address, data, wstrobe);
			}
			break;
			case 3:
			{
				m_sdcc.Write(address, data, wstrobe);
			}
			break;
			case 4:
			{
				// DEVICE_XADC
				//m_xadc->Write(address, data, wstrobe);
				printf("XADC@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 5:
			{
				// DEVICE_DMAC
				//m_dmac->Write(address, data, wstrobe);
				printf("DMAC@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 6:
			{
				// DEVICE_USBA
				//m_usba->Write(address, data, wstrobe);
				//printf("USB-A@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 7:
			{
				// DEVICE_APUC
				//m_apuc->Write(address, data, wstrobe);
				printf("APUC@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 8:
			{
				m_mail.Write(address, data, wstrobe);
			}
			break;
			case 9:
			{
				m_uart.Write(address, data, wstrobe);
			}
			break;
			case 0xA:
			{
				m_csr[0]->Write(address, data, wstrobe);
			}
			break;
			case 0xB:
			{
				m_csr[1]->Write(address, data, wstrobe);
			}
			break;
			default:
			{
				printf("Unknown device write @0x%.8X\n", address);
			}
		}
	}
	else
	{
#if defined(MEM_DEBUG)
		uint32_t addrkb = address / 1024;
		m_busactivitystart = addrkb < m_busactivitystart ? addrkb : m_busactivitystart;
		m_busactivityend = addrkb > m_busactivityend ? addrkb : m_busactivityend;
#endif

		m_vpuc.DirtyInVideoScanoutRegion(address);
		m_mem.Write(address, data, wstrobe);
	}
}
