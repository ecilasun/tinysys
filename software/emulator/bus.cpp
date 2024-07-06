#include <stdio.h>
#include "bus.h"

CBus::CBus(uint32_t resetvector)
{
	m_resetvector = resetvector;

	m_mem = new CSysMem();
	m_vpuc = new CVPU();
	m_apu = new CAPU();
	m_dmac = new CDMA();
	m_leds = new CLEDs();
	m_mail = new CMailMem();
	m_csr[0] = new CCSRMem(0);
	m_csr[1] = new CCSRMem(1);
	m_cpu[0] = new CRV32(0, resetvector);
	m_cpu[1] = new CRV32(1, resetvector);
	m_sdcc = new CSDCard();
	m_uart = new CUART();
	m_dummydevice = new CDummyDevice();

	// Device array
	m_devices[0] = m_dummydevice; // GPIO
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
	m_devices[12] = m_mem; // system memory
}

CBus::~CBus()
{
	if (m_csr[0]) delete m_csr[0];
	if (m_csr[1]) delete m_csr[1];
	if (m_sdcc) delete m_sdcc;
	if (m_cpu[0]) delete m_cpu[0];
	if (m_cpu[1]) delete m_cpu[1];
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

	m_csr[0]->Reset();
	if (m_csr[1]) m_csr[1]->Reset();

	m_cpu[0]->Reset();
	if (m_cpu[1]) m_cpu[1]->Reset();
}

void CBus::UpdateVideoLink(uint32_t *pixels, int pitch)
{
	m_vpuc->UpdateVideoLink(pixels, pitch, this);
}

#if defined(MEM_DEBUG)
void CBus::FillMemBitmap(uint32_t* pixels)
{
	uint32_t* source = m_mem->GetHostAddress(0);

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
	m_uart->QueueByte(byte);
}

bool CBus::Tick()
{
	bool ret0 = m_cpu[0]->Tick(this);

	bool ret1 = true;
	if (m_cpu[1])
		ret1 = m_cpu[1]->Tick(this);

	for (int i = 0; i < 13; ++i)
		m_devices[i]->Tick(this);

	return ret0 && ret1;
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
	uint32_t dev = address & 0x80000000 ? ((address & 0xF0000) >> 16) : 12;
	m_devices[dev]->Read(address, data);
}

void CBus::Write(uint32_t address, uint32_t data, uint32_t wstrobe)
{
	uint32_t dev = address & 0x80000000 ? ((address & 0xF0000) >> 16) : 12;
	m_devices[dev]->Write(address, data, wstrobe);

#if defined(MEM_DEBUG)
	if (dev == 12)
	{
		uint32_t addrkb = address / 1024;
		m_busactivitystart = addrkb < m_busactivitystart ? addrkb : m_busactivitystart;
		m_busactivityend = addrkb > m_busactivityend ? addrkb : m_busactivityend;
	}
#endif
}

void CBus::Read16(uint32_t address, uint32_t* data)
{
	uint32_t dev = address & 0x80000000 ? ((address & 0xF0000) >> 16) : 12;
	m_devices[dev]->Read16(address, data);
}

void CBus::Write16(uint32_t address, uint32_t* data)
{
	uint32_t dev = address & 0x80000000 ? ((address & 0xF0000) >> 16) : 12;
	m_devices[dev]->Write16(address, data);
}
