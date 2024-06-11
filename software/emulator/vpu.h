#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "clock.h"
#include "rv32.h"
#include <queue>

class CBus;

class CVPU
{
public:
	CVPU();
	~CVPU();

	void Reset();
	void Tick(CClock& cpuclock);
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

	void UpdateVideoLink(uint32_t* pixels, CBus* bus);

private:
	uint32_t m_cmd{ 0 };
	uint32_t m_data{ 0 };
	uint32_t m_state{ 0 };
	uint32_t m_videoscanoutenable{ 0 };
	uint32_t m_indexedcolormode{ 0 };
	uint32_t m_scanoutpointer{ 0 };
	uint32_t m_scanwidth{ 320 };
	uint32_t m_vgapalette[256];
	std::queue<uint32_t> m_fifo;
};
