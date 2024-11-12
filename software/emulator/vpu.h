#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "rv32.h"
#include <queue>
#include "memmappeddevice.h"

class CBus;

class CVPU : public MemMappedDevice
{
public:
	CVPU();
	~CVPU();

	void Reset() override final;
	void Read(uint32_t address, uint32_t& data) override final;
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe) override final;
	void Tick(CBus* bus);

	void UpdateVideoLink(uint32_t* pixels, int pitch, int scanline, CBus* bus);

	uint32_t m_hirq{ 0 };

private:
	uint32_t m_cmd{ 0 };
	uint32_t m_data{ 0 };
	uint32_t m_state{ 0 };
	uint32_t m_videoscanoutenable{ 0 };
	uint32_t m_12bppmode{ 0 };
	uint32_t m_scanoutpointer{ 0 };
	uint32_t m_scanwidth{ 320 };
	uint32_t m_scanheight{ 240 };
	uint32_t m_scanlength{ 320 * 240 };
	uint32_t m_count{ 0 };
	uint32_t m_vgapalette[256];
	uint32_t m_fakevsync{ 0 };
	uint32_t m_ctlreg{ 0 };
	uint32_t m_regA{ 0 };
	uint32_t m_regB{ 0xFFFFFFFF };
	std::queue<uint32_t> m_fifo;
};
