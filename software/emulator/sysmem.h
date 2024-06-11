#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"

class CSysMem
{
public:
	CSysMem();
	~CSysMem();

	void *m_devicemem;

	void Reset();
	void Tick(CClock& cpuclock);
	void CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
