#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <queue>
#include "rv32.h"
#include "memmappeddevice.h"

class CSysMem;

class CDMA : public MemMappedDevice
{
public:
	CDMA();
	~CDMA();

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
	void Tick(CSysMem* mem);

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
