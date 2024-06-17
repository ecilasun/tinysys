#pragma once

#include "rv32.h"
#include "sysmem.h"
#include "csrmem.h"
#include "sdcard.h"
#include "mailmem.h"
#include "vpu.h"
#include "leds.h"
#include "uart.h"

#define DEVICE_BASE 0x80000000

#define DEVICE_GPIO (DEVICE_BASE+0x00000)
#define DEVICE_LEDS (DEVICE_BASE+0x10000)
#define DEVICE_VPUC (DEVICE_BASE+0x20000)
#define DEVICE_SPIC (DEVICE_BASE+0x30000)
#define DEVICE_XADC (DEVICE_BASE+0x40000)
#define DEVICE_DMAC (DEVICE_BASE+0x50000)
#define DEVICE_USBA (DEVICE_BASE+0x60000)
#define DEVICE_APUC (DEVICE_BASE+0x70000)
#define DEVICE_MAIL (DEVICE_BASE+0x80000)
#define DEVICE_UART (DEVICE_BASE+0x90000)
#define DEVICE_CSR0 (DEVICE_BASE+0xA0000)
#define DEVICE_CSR1 (DEVICE_BASE+0xB0000)
//#define DEVICE_DEV0 (DEVICE_BASE+0xC0000)
//#define DEVICE_DEV1 (DEVICE_BASE+0xD0000)
//#define DEVICE_DEV2 (DEVICE_BASE+0xE0000)
//#define DEVICE_DEV3 (DEVICE_BASE+0xF0000)

class CBus
{
public:
	CBus() {}
	~CBus() {}

	void Reset(uint32_t resetvector, uint8_t* rombin, uint32_t romsize);
	uint32_t Tick(CRV32* cpu0, uint32_t* sie0);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t data, uint32_t wstrobe);
	uint32_t* GetHostAddress(uint32_t address);

	void UpdateVideoLink(uint32_t* pixels, int pitch);
	void QueueByte(uint8_t byte);

	bool IsVideoDirty() { return m_vpuc.IsVideoDirty(); }
	void ClearVideoDirty() { m_vpuc.ClearVideoDirty(); }

private:
	CSysMem m_mem;
	CCSRMem m_csr[2];
	CMailMem m_mail;
	CSDCard m_sdcc;
	CVPU m_vpuc;
	CLEDs m_leds;
	CUART m_uart;
};
