#pragma once

#include <stdint.h>

class CSDCard
{
public:
	CSDCard() {}
	~CSDCard() {}

	void Reset();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);
};
