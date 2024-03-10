#include <stdio.h>
#include "sdcard.h"

uint32_t CSDCard::Read(uint32_t address)
{
	printf("SDCard read 0x%.8X\n", address);
	return 0xFF;
}

void CSDCard::Write(uint32_t address, uint32_t word)
{
	// TODO:
	printf("SDCard write 0x%.8X, 0x%.8X\n", address, word);
}
