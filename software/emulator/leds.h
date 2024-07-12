#pragma once

#include <stdint.h>
#include "memmappeddevice.h"

class CLEDs : public MemMappedDevice
{
public:
	CLEDs() {}
	~CLEDs() {}

	uint32_t m_ledstate = 0;

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
	void Read16(uint32_t address, uint32_t* data) override final { data[0] = 0; }
	void Write16(uint32_t address, uint32_t* data) override final {}
};
