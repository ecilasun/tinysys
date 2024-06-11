#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"

class CMailMem
{
public:
	CMailMem();
	~CMailMem();

	uint32_t * m_mailmem;

	void Reset();
	void Tick(CClock& cpuclock);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
