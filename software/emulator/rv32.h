#pragma once

#include "clock.h"
#include "memman.h"

enum CPUState{
	ECPUReset,
	ECPUFetch,
	ECPUDecode,
	ECPUExecute,
	ECPURetire
};

class CRV32
{
public:
	CRV32();
	~CRV32();

	uint32_t PC;
	uint32_t GPR[32];

	uint32_t PC_next;
	uint32_t GPR_next[32];

	CPUState m_state = ECPUReset;
	CPUState m_state_next;

	CMemMan m_memory;

	void Tick(CClock& cpuclock);
};
