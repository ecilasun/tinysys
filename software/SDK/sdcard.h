#pragma once

#include <inttypes.h>

extern volatile uint8_t *IO_SPIRXTX;
extern volatile uint8_t *IO_CARDDETECT;

#define SD_READY 0x00
#define SD_START_TOKEN 0xFE

int SDCardStartup();
int SDIOControl(const uint8_t cmd, void *buffer);
int SDReadMultipleBlocks(uint8_t *datablock, uint32_t numblocks, uint32_t blockaddress);
int SDWriteMultipleBlocks(const uint8_t *datablock, uint32_t numblocks, uint32_t blockaddress);