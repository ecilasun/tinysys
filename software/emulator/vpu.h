#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "rv32.h"
#include <queue>

class CBus;

class CVPU
{
public:
	CVPU();
	~CVPU();

	void Reset();
	void Tick();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

	void UpdateVideoLink(uint32_t* pixels, int pitch, CBus* bus);
	bool IsVideoDirty() { return m_videodirty; }
	void ClearVideoDirty() { m_videodirty = 0; }
	void DirtyInVideoScanoutRegion(uint32_t address) { if (address>=m_scanoutpointer && address<=m_scanoutpointer+m_scanlength) m_videodirty = 1; }

private:
	uint32_t m_cmd{ 0 };
	uint32_t m_data{ 0 };
	uint32_t m_state{ 0 };
	uint32_t m_videoscanoutenable{ 0 };
	uint32_t m_indexedcolormode{ 0 };
	uint32_t m_scanoutpointer{ 0 };
	uint32_t m_scanwidth{ 320 };
	uint32_t m_scanheight{ 240 };
	uint32_t m_scanlength{ 320 * 240 };
	uint32_t m_count{ 0 };
	uint32_t m_vgapalette[256];
	uint32_t m_videodirty{ 1 };
	std::queue<uint32_t> m_fifo;
};
