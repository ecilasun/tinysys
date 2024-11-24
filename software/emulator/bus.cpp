#include <stdio.h>
#include "bus.h"

CBus::CBus(uint32_t resetvector)
{
	m_resetvector = resetvector;

	m_mem = new CSysMem();
	m_scratchpad = new CScratchpadMem();
	m_vpuc = new CVPU();
	m_apu = new CAPU();
	m_dmac = new CDMA();
	m_leds = new CLEDs();
	m_mail = new CMailMem();
	m_csr[0] = new CCSRMem(0);
	m_csr[1] = new CCSRMem(1);
	m_csr[2] = new CCSRMem(2);
	m_sdcc = new CSDCard();
	m_uart = new CUART();
	m_dummydevice = new CDummyDevice();

	// Device array
	m_devices[0] = m_scratchpad;
	m_devices[1] = m_leds;
	m_devices[2] = m_vpuc;
	m_devices[3] = m_sdcc;
	m_devices[4] = m_dummydevice; // XADC
	m_devices[5] = m_dmac;
	m_devices[6] = m_dummydevice; // USBA
	m_devices[7] = m_apu;
	m_devices[8] = m_mail;
	m_devices[9] = m_uart;
	m_devices[10] = m_csr[0];
	m_devices[11] = m_csr[1];
	m_devices[12] = m_csr[2];
	m_devices[13] = m_mem; // system memory
}

CBus::~CBus()
{
	if (m_scratchpad) delete m_scratchpad;
	if (m_csr[0]) delete m_csr[0];
	if (m_csr[1]) delete m_csr[1];
	if (m_csr[2]) delete m_csr[2];
	if (m_sdcc) delete m_sdcc;
	if (m_mail) delete m_mail;
	if (m_uart) delete m_uart;
	if (m_leds) delete m_leds;
	if (m_dmac) delete m_dmac;
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
	m_dmac->Reset();
	m_uart->Reset();
	m_leds->Reset();
	m_mail->Reset();
	m_dummydevice->Reset();
	m_apu->Reset();
	m_scratchpad->Reset();

	m_csr[0]->Reset();
	m_csr[1]->Reset();
	m_csr[2]->Reset();
}

void CBus::UpdateVideoLink(uint32_t *pixels, int pitch, int scanline)
{
	m_vpuc->UpdateVideoLink(pixels, pitch, scanline, this);
}

void CBus::QueueByte(uint8_t byte)
{
	m_uart->QueueByte(byte);
}

bool CBus::Tick()
{
	m_dmac->Tick(m_mem);
	m_sdcc->Tick(this);
	m_uart->Tick(this);
	m_vpuc->Tick(this);
	m_apu->Tick(this);
	// NOTE: CPU will tick the CSR
	return true;
}

uint32_t* CBus::GetHostAddress(uint32_t address)
{
	// Convert to emulator host address from emulated device memory address
	if (address & 0x80000000)
		return nullptr;
	else
		return m_mem->GetHostAddress(address);
}

void CBus::Read(uint32_t address, uint32_t& data)
{
	uint32_t dev = address & 0x80000000 ? ((address & 0xF0000) >> 16) : 13;
	m_devices[dev]->Read(address, data);
}

void CBus::Write(uint32_t address, uint32_t data, uint32_t wstrobe)
{
	uint32_t dev = address & 0x80000000 ? ((address & 0xF0000) >> 16) : 13;
	m_devices[dev]->Write(address, data, wstrobe);
}
