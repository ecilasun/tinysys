#pragma once

#include "rv32.h"
#include "sysmem.h"
#include "csrmem.h"
#include "sdcard.h"
#include "mailmem.h"
#include "scratchpadmem.h"
#include "vpu.h"
#include "apu.h"
#include "leds.h"
#include "uart.h"
#include "dummydevice.h"
#include "memmappeddevice.h"

#define DEVICE_BASE 0x80000000

#define DEVICE_SPAD (DEVICE_BASE+0x00000)
#define DEVICE_LEDS (DEVICE_BASE+0x10000)
#define DEVICE_VPUC (DEVICE_BASE+0x20000)
#define DEVICE_SPIC (DEVICE_BASE+0x30000)
#define DEVICE_DEV0 (DEVICE_BASE+0x40000)
#define DEVICE_USBA (DEVICE_BASE+0x50000)
#define DEVICE_APUC (DEVICE_BASE+0x60000)
#define DEVICE_MAIL (DEVICE_BASE+0x70000)
#define DEVICE_UART (DEVICE_BASE+0x80000)
#define DEVICE_CSR0 (DEVICE_BASE+0x90000)
#define DEVICE_CSR1 (DEVICE_BASE+0xA0000)
//#define DEVICE_DEV1 (DEVICE_BASE+0xB0000)
//#define DEVICE_DEV2 (DEVICE_BASE+0xC0000)
//#define DEVICE_DEV3 (DEVICE_BASE+0xD0000)
//#define DEVICE_DEV4 (DEVICE_BASE+0xE0000)
//#define DEVICE_DEV5 (DEVICE_BASE+0xF0000)

class CBus
{
public:
	explicit CBus(uint32_t resetvector);
	~CBus();

	void Reset(uint8_t* rombin, uint32_t romsize);
	bool Tick();
	void Read(uint32_t address, uint32_t& data);	// Write 32 bits
	void Write(uint32_t address, uint32_t data, uint32_t wstrobe);	// Write 32 bits
	uint32_t* GetHostAddress(uint32_t address);

	CAPU* GetAPU() { return m_apu; }
	CUART* GetUART() { return m_uart; }
	CVPU* GetVPU() { return m_vpuc; }
	CLEDs* GetLEDs() { return m_leds; }
	CCSRMem* GetCSR(uint32_t hartid) { return m_csr[hartid]; }

	void UpdateVideoLink(uint32_t* pixels, int pitch);
	void QueueByte(uint8_t byte);

	CSysMem* m_mem{ nullptr };

private:
	CCSRMem* m_csr[2]{ nullptr, nullptr};
	CMailMem* m_mail{ nullptr };
	CScratchpadMem* m_spad{ nullptr };
	CSDCard* m_sdcc{ nullptr };
	CVPU* m_vpuc{ nullptr };
	CLEDs* m_leds{ nullptr };
	CUART* m_uart{ nullptr };
	CAPU* m_apu{ nullptr };
	CDummyDevice* m_null{ nullptr };

	MemMappedDevice* m_devices[12]{ nullptr };

	uint32_t m_resetvector{ 0 };
};
