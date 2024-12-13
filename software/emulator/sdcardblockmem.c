#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <ff.h>
#include <diskio.h>

static intptr_t* s_alloctable;
static uint32_t s_blockcount;

// 1 GByte virtual disk
#define VIRTUALDISKSIZE (1024*1024*1024)

void SDInitBlockMem()
{
	// One entry per mbyte we might be using
	// This will ensure we use only as much memory as we need, in 1Mbyte increments
	s_alloctable = (intptr_t*)malloc(512*sizeof(intptr_t));
	s_blockcount = 0;
	memset(s_alloctable, 0, 512*sizeof(intptr_t));
}

void SDReportMemoryUsage()
{
	printf("Virtual sdcard using %d MBytes of RAM out of %d Mbytes disk capacity\n", s_blockcount, VIRTUALDISKSIZE/(1024*1024));
}

void SDFreeBlockMem()
{
	for (int i = 0; i < 512; ++i)
	{
		if (s_alloctable[i])
			free((void*)s_alloctable[i]);
	}
	free(s_alloctable);
}

int SDCardStartup()
{
	return 0;
}

uint8_t* SDCardGetPhysicalBlock(uint32_t blockaddress)
{
	// We can fit 2048 blocks per mbyte
	uint32_t mbyteblock = blockaddress / 2048;
	uint32_t blockoffset = blockaddress % 2048;
	uint32_t byteoffset = blockoffset * 512; // block address is divided by 512

	if (!s_alloctable[mbyteblock])
	{
		++s_blockcount;
		uint8_t *ptr = (uint8_t*)malloc(1024 * 1024);
		s_alloctable[mbyteblock] = (intptr_t)ptr;
	}

	//printf("Total memory used:%d MBytes (block#:%08X, offset:%08X ptr:%llx)\n", s_blockcount, mbyteblock, byteoffset, (size_t)s_alloctable[mbyteblock]);

	intptr_t physical = s_alloctable[mbyteblock] + byteoffset;

	return (uint8_t*)physical;
}

int SDIOControl(const uint8_t cmd, void* buffer)
{
	if (cmd == GET_BLOCK_SIZE)
		*(DWORD*)buffer = 0x200; // 512
	else if (cmd == GET_SECTOR_COUNT)
		*(DWORD*)buffer = VIRTUALDISKSIZE/512;
	else if (cmd == CTRL_SYNC) // we never have unfinished writes in flight
		;
	else
		;// __debugbreak();

	return RES_OK;
}

int SDReadBlock(uint32_t blockaddress, uint8_t* datablock)
{
	if (blockaddress >= 262144)
	{
		printf("Error: out of bounds when reading SDCard block\n");
		return -1;
	}
	uint8_t* source = SDCardGetPhysicalBlock(blockaddress);
	memcpy(datablock, source, 512);
	return 0;
}

int SDWriteBlock(uint32_t blockaddress, const uint8_t* datablock)
{
	if (blockaddress >= 262144)
	{
		printf("Error: out of bounds when writing SDCard block\n");
		return -1;
	}
	uint8_t* target = SDCardGetPhysicalBlock(blockaddress);
	memcpy(target, datablock, 512);
	return 0;
}

int SDReadMultipleBlocks(uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress)
{
	if (numblocks == 0)
	{
		printf("Error: reading zero SDCard blocks\n");
		return -1;
	}

	uint32_t cursor = 0;

	for (uint32_t b = 0; b < numblocks; ++b)
	{
		uint8_t* target = (uint8_t*)(datablock + cursor);
		if (SDReadBlock(b + blockaddress, target) < 0)
			return -1;
		cursor += 512;
	}

	return 0;
}

int SDWriteMultipleBlocks(const uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress)
{
	if (numblocks == 0)
	{
		printf("Error: writing zero SDCard blocks\n");
		return -1;
	}

	uint32_t cursor = 0;

	for (uint32_t b = 0; b < numblocks; ++b)
	{
		uint8_t* source = (uint8_t*)(datablock + cursor);
		if (SDWriteBlock(b + blockaddress, source)<0)
			return -1;
		cursor += 512;
	}

	return 0;
}
