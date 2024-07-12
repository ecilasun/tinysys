#pragma once

#include <stdint.h>
#include <queue>
#include <ff.h>
#include "memmappeddevice.h"

class CSDCard : public MemMappedDevice
{
public:
	CSDCard() {}
	~CSDCard();

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
	void Tick(CBus* bus);

private:
	void PopulateFileSystem();
	uint32_t SPIRead(uint8_t* buffer, uint32_t len);
	std::queue<uint8_t> m_spiinfifo;
	std::queue<uint8_t> m_spioutfifo;
	uint32_t m_spimode{ 0 };
	uint32_t m_havestarttoken{ 0 };
	uint32_t m_numdatabytes{ 0 };
	uint8_t m_databytes[8] = {};
	uint8_t m_cmdbyte{ 0 };
	uint32_t m_readblock{ 0 };
	uint32_t m_writeblock{ 0 };
	uint8_t m_datablock[512] = {};
	bool m_app_mode{ false };

	FATFS* m_fs{ nullptr };
	uint8_t* m_workbuf{ nullptr };
};
