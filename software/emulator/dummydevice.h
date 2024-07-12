#pragma once

#include <stdint.h>
#include "memmappeddevice.h"

class CDummyDevice : public MemMappedDevice
{
public:
	CDummyDevice() {}
	~CDummyDevice() {}

	void Reset() override final { }
	void Read(uint32_t address, uint32_t& data) override final { data = 0xFFFFFFFF; }
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final { }
	void Read16(uint32_t address, uint32_t* data) override final { data[0] = 0; }
	void Write16(uint32_t address, uint32_t* data) override final {}
};
