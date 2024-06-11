#include <stdio.h>
#include "sdcard.h"

void CSDCard::Reset()
{
}

void CSDCard::Read(uint32_t address, uint32_t& data)
{
	// TODO: Implement SD card SPI read

	// Return illegal data so OS detects that the sdcard reader is not working
	data = 0xFF;
}

void CSDCard::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// TODO: Implement SD card SPI write
}
