#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "rv32.h"
#include <queue>

class CBus;

class CDMA
{
public:
	CDMA();
	~CDMA();

	void Reset();
	void Tick(CBus* bus);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

private:
	uint32_t m_op{ 0 };
	uint32_t m_cmd{ 0 };
	uint32_t m_data{ 0 };
	uint32_t m_state{ 0 };

	uint32_t m_dmasourceaddr{ 0 };
	uint32_t m_dmatargetaddr{ 0 };
	uint32_t m_dmaburstcount{ 0 };
	uint32_t m_dmamaskmode{ 0 };

	std::queue<uint32_t> m_fifo;
};
