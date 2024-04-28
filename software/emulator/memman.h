#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"
#include "sdcard.h"

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

#define CSR_MSTATUS			0x300
#define CSR_MISA			0x301
#define CSR_MIE		    	0x304
#define CSR_MTVEC			0x305
#define CSR_MEPC			0x341
#define CSR_MCAUSE			0x342
#define CSR_MTVAL			0x343
#define CSR_MIP				0x344
#define CSR_TIMECMPLO		0x800
#define CSR_TIMECMPHI		0x801
#define CSR_CYCLELO			0xC00
#define CSR_TIMELO			0xC01
#define CSR_RETILO			0xC02
#define CSR_CYCLEHI			0xC80
#define CSR_TIMEHI			0xC81
#define CSR_RETIHI			0xC82
#define CSR_MARCHID			0xF12
#define CSR_MIMPID			0xF13
#define CSR_MHARTID			0xF14
#define CSR_HWSTATE			0xFFF
#define CSR_REGISTERSHADOW	0x8A0
#define CSR_WATERMARK		0xFF0

class CMemMan
{
public:
	CMemMan();
	~CMemMan();

	void *m_devicemem;
	uint32_t *m_csrmem[2];

	void Tick(CClock& cpuclock);
	void CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size);
	uint32_t FetchInstruction(uint32_t address);
	uint32_t FetchDataWord(uint32_t address);
	void WriteDataWord(uint32_t address, uint32_t word, uint32_t wstrobe);

	// Memory mapped devices
	CSDCard m_sdcard;
};
