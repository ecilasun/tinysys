#pragma once

#include "clock.h"
#include "rv32.h"
#include "sysmem.h"
#include "csrmem.h"
#include "sdcard.h"
#include "mailmem.h"

#define DEVICE_BASE 0x80000000
#define DEVICE_TRUC (DEVICE_BASE+0x0000)
#define DEVICE_LEDS (DEVICE_BASE+0x1000)
#define DEVICE_VPUC (DEVICE_BASE+0x2000)
#define DEVICE_SPIC (DEVICE_BASE+0x3000)
#define DEVICE_XADC (DEVICE_BASE+0x4000)
#define DEVICE_DMAC (DEVICE_BASE+0x5000)
#define DEVICE_USBC (DEVICE_BASE+0x6000)
#define DEVICE_APUC (DEVICE_BASE+0x7000)
#define DEVICE_USBA (DEVICE_BASE+0x8000)
#define DEVICE_CSR0 (DEVICE_BASE+0x9000)

class CBus
{
public:
	CBus() {}
	~CBus() {}

	void Reset(uint32_t resetvector, uint8_t* rombin, uint32_t romsize);
	void Tick(CClock& cpuclock, CRV32& cpu);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t data, uint32_t wstrobe);

private:
	CSysMem m_mem;
	CCSRMem m_csr[2];
	CMailMem m_mail;
	CSDCard m_sdcc;
};
