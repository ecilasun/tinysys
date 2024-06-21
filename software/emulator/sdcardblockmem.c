#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

static uint8_t* s_sdcardblockmem;

void SDInitBlockMem()
{
	s_sdcardblockmem = (uint8_t*)malloc(128 * 1024 * 1024);
	memset(s_sdcardblockmem, 0, 128 * 1024 * 1024);
}

int SDReadMultipleBlocks(uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress)
{
	//printf("R 0x%.8X blocks from 0x%.8X\n", numblocks, blockaddress);
	if (numblocks == 0)
		return -1;

	uint32_t cursor = 0;

	for (uint32_t b = 0; b < numblocks; ++b)
	{
		uint8_t* target = (uint8_t*)(datablock + cursor);
		uint8_t* source = s_sdcardblockmem + (b + blockaddress) * 512;
		memcpy(target, source, 512);
		cursor += 512;
	}

	return 0;
}

int SDWriteMultipleBlocks(const uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress)
{
	//printf("W 0x%.8X blocks at 0x%.8X\n", numblocks, blockaddress);
	if (numblocks == 0)
		return -1;

	uint32_t cursor = 0;

	for (uint32_t b = 0; b < numblocks; ++b)
	{
		uint8_t* source = (uint8_t*)(datablock + cursor);
		uint8_t* target = s_sdcardblockmem + (b + blockaddress) * 512;
		memcpy(target, source, 512);
		cursor += 512;
	}

	return 0;
}
