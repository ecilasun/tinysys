#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "config.h"
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

	void UpdateVideoLink(uint32_t* pixels, int pitch, CBus* bus);

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
	uint32_t m_vsyncCount{ 0 };
	uint32_t m_vgapalette[256];
	uint32_t m_fakevsync{ 0 };
	uint32_t m_ctlreg{ 0 };
	int32_t m_regA{ 0 };
	int32_t m_regB{ 65536 };
	int32_t m_regC{ 0 };
	std::queue<uint32_t> m_fifo;
};
