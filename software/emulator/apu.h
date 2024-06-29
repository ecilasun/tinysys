#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "rv32.h"
#include <queue>
#include "memmappeddevice.h"

#define APUCMD_BUFFERSIZE  0x00000000
#define APUCMD_START       0x00000001
#define APUCMD_NOOP1       0x00000002
#define APUCMD_NOOP2       0x00000003
#define APUCMD_SETRATE     0x00000004

class CBus;

class CAPU : public MemMappedDevice
{
public:
	CAPU();
	~CAPU();

	void Reset() override final;
	void Tick(CBus* bus) override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;

	void* GetPlaybackData() { return m_audioData[m_currentbuffer]; }
	void FlipBuffers() { m_currentbuffer ^= 1; }

	uint32_t m_rateselector{ 0 };
	uint32_t m_apuwordcount{ 0 };

private:
	uint32_t m_cmd{ 0 };
	uint32_t m_data{ 0 };
	uint32_t m_state{ 0 };
	uint32_t m_currentbuffer{ 0 };
	uint32_t m_sourceAddress{ 0 };
	uint32_t *m_audioData[2]{ nullptr };

	std::queue<uint32_t> m_fifo;
};
