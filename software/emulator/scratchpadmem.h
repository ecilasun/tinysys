#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "memmappeddevice.h"

class CScratchpadMem : public MemMappedDevice
{
public:
	CScratchpadMem();
	~CScratchpadMem();

	uint32_t* m_scratchmem{ nullptr };

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
};
