#pragma once

#include <stdlib.h>
#include <stdint.h>

class CSysMem
{
public:
	CSysMem();
	~CSysMem();

	void *m_devicemem;

	void Reset();
	void Tick();
	void CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size);
	uint32_t* GetHostAddress(uint32_t address);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
