#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "memmappeddevice.h"

class CMailMem : public MemMappedDevice
{
public:
	CMailMem();
	~CMailMem();

	uint32_t* m_mailmem{ nullptr };

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
	void Read16(uint32_t address, uint32_t* data) override final { data[0] = 0; }
	void Write16(uint32_t address, uint32_t* data) override final {}
};
