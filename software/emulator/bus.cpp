#include <stdio.h>
#include "bus.h"

CBus::CBus(uint32_t resetvector)
{
	m_resetvector = resetvector;

	m_mem = new CSysMem();
	m_spad = new CScratchpadMem();
	m_vpuc = new CVPU();
	m_apu = new CAPU();
	m_leds = new CLEDs();
	m_mail = new CMailMem();
	m_csr[0] = new CCSRMem(0);
	m_csr[1] = new CCSRMem(1);
	m_sdcc = new CSDCard();
	m_uart = new CUART();
	m_null = new CDummyDevice();

	// Device array
	m_devices[0] = m_spad;
	m_devices[1] = m_leds;
	m_devices[2] = m_vpuc;
	m_devices[3] = m_sdcc;
	m_devices[4] = m_null;
	m_devices[5] = m_null;
	m_devices[6] = m_apu;
	m_devices[7] = m_mail;
	m_devices[8] = m_uart;
	m_devices[9] = m_csr[0];
	m_devices[10] = m_csr[1];
	m_devices[11] = m_mem; // system memory
}

CBus::~CBus()
{
	if (m_spad) delete m_spad;
	if (m_csr[0]) delete m_csr[0];
	if (m_csr[1]) delete m_csr[1];
	if (m_sdcc) delete m_sdcc;
	if (m_mail) delete m_mail;
	if (m_uart) delete m_uart;
	if (m_null) delete m_null;
	if (m_leds) delete m_leds;
	if (m_vpuc) delete m_vpuc;
	if (m_apu) delete m_apu;
	if (m_mem) delete m_mem;
}

void CBus::Reset(uint8_t* rombin, uint32_t romsize)
{
	m_mem->Reset();
	m_mem->CopyROM(m_resetvector, rombin, romsize);

	m_sdcc->Reset();
	m_vpuc->Reset();
	m_uart->Reset();
	m_leds->Reset();
	m_mail->Reset();
	m_null->Reset();
	m_apu->Reset();
	m_spad->Reset();

	m_csr[0]->Reset();
	m_csr[1]->Reset();
}

void CBus::UpdateVideoLink(uint32_t *pixels, int pitch)
{
	m_vpuc->UpdateVideoLink(pixels, pitch, this);
}

void CBus::QueueByte(uint8_t byte)
{
	m_uart->QueueByte(byte);
}

bool CBus::Tick(uint32_t _hartid)
{
	if(_hartid == 0)
	{
		m_sdcc->Tick(this);
		m_uart->Tick(this);
		m_vpuc->Tick(this);
		m_apu->Tick(this);
	}
	m_csr[_hartid]->Tick(this);
	return true;
}

uint32_t* CBus::GetHostAddress(uint32_t address)
{
	// Convert to emulator host address from emulated device memory address
	if (address & 0x80000000)
	{
		uint32_t dev = (address & 0x80000000) ? ((address & 0xF0000) >> 16) : 11;

		switch(dev)
		{
			case 0: // SPAD
				return m_spad->m_scratchmem;
			break;
			case 1: // LEDS
				return nullptr;
			break;
			case 2: // VPUC
				return nullptr;
			break;
			case 3: // SDCC
				return nullptr;
			break;
			case 4: // NULL
			case 5: // NULL
				return nullptr;
			break;
			case 6: // APU
				return nullptr;
			break;
			case 7: // MAIL
				return m_mail->m_mailmem;
			break;
			case 8: // UART
				return nullptr;
			break;
			case 9: // CSR0
				return m_csr[0]->GetCSRMem();
			break;
			case 10: // CSR1
				return m_csr[1]->GetCSRMem();
			break;
			default:
				return nullptr;
		}
	}
	else
		return m_mem->GetHostAddress(address);
}

void CBus::Read(uint32_t address, uint32_t& data)
{
	uint32_t dev = (address & 0x80000000) ? ((address & 0xF0000) >> 16) : 11;
	if (dev>11)
	{
		//fprintf(stderr, "CBus::Read - invalid device %d\n", dev);
		data = 0;
		return;
	}
	m_devices[dev]->Read(address, data);
}
 
void CBus::Write(uint32_t address, uint32_t data, uint32_t wstrobe)
{
	uint32_t dev = (address & 0x80000000) ? ((address & 0xF0000) >> 16) : 11;
	m_devices[dev]->Write(address, data, wstrobe);
}
