#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"

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

class CCSRMem
{
public:
	CCSRMem();
	~CCSRMem();

	uint32_t *m_csrmem;

	void Reset();
	void Tick(CClock& cpuclock);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
