#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

static uint8_t* s_blockmem;

void SDInitBlockMem()
{
	s_blockmem = (uint8_t*)malloc(256 * 1024 * 1024);
}

int SDReadMultipleBlocks(uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress)
{
	if (numblocks == 0)
		return -1;

	uint32_t cursor = 0;

	//uint8_t checksum[2];

	for (uint32_t b = 0; b < numblocks; ++b)
	{
		uint8_t* target = (uint8_t*)(datablock + cursor);
		memcpy(target, s_blockmem + (b + blockaddress) * 512, 512);
		//SDReadSingleBlock(b + blockaddress, target, checksum);
		cursor += 512;
	}

	return 0;
}

int SDWriteMultipleBlocks(const uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress)
{
	if (numblocks == 0)
		return -1;

	uint32_t cursor = 0;

	for (uint32_t b = 0; b < numblocks; ++b)
	{
		uint8_t* source = (uint8_t*)(datablock + cursor);
		memcpy(s_blockmem + (b + blockaddress) * 512, source, 512);
		//SDWriteSingleBlock(b + blockaddress, source);
		cursor += 512;
	}

	return 0;
}
