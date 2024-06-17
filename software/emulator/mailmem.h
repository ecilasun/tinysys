#pragma once

#include <stdlib.h>
#include <stdint.h>

class CMailMem
{
public:
	CMailMem();
	~CMailMem();

	uint32_t* m_mailmem{ nullptr };

	void Reset();
	void Tick();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
