#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"
#include "rv32.h"
#include <queue>

class CVPU
{
public:
	CVPU();
	~CVPU();

	void Reset();
	void Tick(CClock& cpuclock);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

private:
	uint32_t* m_vrampointer = nullptr;
	std::queue<uint32_t> m_fifo;
};
