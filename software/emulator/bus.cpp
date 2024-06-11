#include <stdio.h>
#include "bus.h"

void CBus::Reset(uint32_t resetvector, uint8_t* rombin, uint32_t romsize)
{
	m_mem.Reset();
	m_csr[0].Reset();
	m_csr[1].Reset();
	m_sdcc.Reset();
	m_vpuc.Reset();

	m_mem.CopyROM(resetvector, rombin, romsize);
}

void CBus::UpdateVideoLink(uint32_t *pixels)
{
	m_vpuc.UpdateVideoLink(pixels, this);
}

uint32_t CBus::Tick(CClock& cpuclock, CRV32* cpu)
{
	// TODO: Update device interrupt state
	//Write(csrbase + (CSR_HWSTATE << 2), 0, 0xFFFFFFFF);

	uint32_t irq = 0;

	m_mem.Tick(cpuclock);
	irq |= m_csr[0].Tick(cpuclock, cpu);
	irq |= m_csr[1].Tick(cpuclock, nullptr); // TODO: This CSR has no CPU to talk to yet
	m_sdcc.Tick(cpuclock);
	m_vpuc.Tick(cpuclock);

	return irq;
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
				// DEVICE_LEDS
				//m_leds->Read(address, data);
				printf("<-LED\n");
				data = 0;
			}
			break;
			case 2:
			{
				// DEVICE_VPUC
				m_vpuc.Read(address, data);
				data = 0;
			}
			break;
			case 3:
			{
				// DEVICE_SDCC
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
				printf("<-USB-A\n");
				data = 0;
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
				// DEVICE_MAIL
				m_mail.Read(address, data);
				data = 0;
			}
			break;
			case 9:
			{
				// DEVICE_UART
				//m_uart->Read(address, data);
				printf("<-UART\n");
				data = 0;
			}
			break;
			case 0xA:
			{
				m_csr[0].Read(address, data);
			}
			break;
			case 0xB:
			{
				m_csr[1].Read(address, data);
			}
			break;
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
				// DEVICE_LEDS
				//m_leds->Write(address, data, wstrobe);
				printf("LEDS@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 2:
			{
				// DEVICE_VPUC
				m_vpuc.Write(address, data, wstrobe);
			}
			break;
			case 3:
			{
				// DEVICE_SDCC
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
				printf("USB-A@0x%.8X<-0x%.8x\n", address, data);
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
				// DEVICE_MAIL
				m_mail.Write(address, data, wstrobe);
			}
			break;
			case 9:
			{
				// DEVICE_UART
				//m_uart->Write(address, data, wstrobe);
				printf("UART@0x%.8X<-0x%.8x\n", address, data);
			}
			break;
			case 0xA:
			{
				m_csr[0].Write(address, data, wstrobe);
			}
			break;
			case 0xB:
			{
				m_csr[1].Write(address, data, wstrobe);
			}
			break;
		}
	}
	else
		m_mem.Write(address, data, wstrobe);
}
