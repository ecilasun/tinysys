#include <stdio.h>
#include <random>
#include "sdcard.h"

void CSDCard::Reset()
{
}

void CSDCard::Tick()
{
	// Run the SPI bus
}

void CSDCard::Read(uint32_t address, uint32_t& data)
{
	// TODO: Implement SD card SPI read

	data = 0x1F; // SPI access should return FF for no device present
}

void CSDCard::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// TODO: Implement SD card SPI write
}
