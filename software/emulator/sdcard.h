#pragma once

#include <stdint.h>
#include <queue>

#include <ff.h>

class CSDCard
{
public:
	CSDCard() {}
	~CSDCard() {}

	void Reset();
	void Tick();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

private:
	void ProcessSPI();
	uint32_t SPIRead(uint8_t* buffer, uint32_t len);
	std::queue<uint8_t> m_spiinfifo;
	std::queue<uint8_t> m_spioutfifo;
	uint32_t m_spimode{ 0 };
	uint32_t m_numdatabytes{ 0 };
	uint8_t m_databytes[8];
	uint8_t m_cmdbyte;
	bool m_app_mode{ false };

	FATFS* m_fs;
};
