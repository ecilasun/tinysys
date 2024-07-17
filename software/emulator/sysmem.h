#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "memmappeddevice.h"

class CSysMem : public MemMappedDevice
{
public:
	CSysMem();
	~CSysMem();

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;

	void Read128bits(uint32_t address, uint32_t* data);
	void Write128bits(uint32_t address, uint32_t* word);

	void Read512bits(uint32_t address, uint32_t* data);
	void Write512bits(uint32_t address, uint32_t* word);

	void* m_devicemem;
	void CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size);
	uint32_t* GetHostAddress(uint32_t address);
};
