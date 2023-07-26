#pragma once

#include <inttypes.h>

#define DMACMD_SETSOURCE      0x00000000
#define DMACMD_SETTARGET      0x00000001
#define DMACMD_SETLENGHT      0x00000002
#define DMACMD_ENQUEUE        0x00000003
#define DMACMD_SETTAG         0x00000004

// DMA read/write operations are in increments of 16 byte blocks
#define DMA_BLOCK_SIZE 16

void DMACopy4K(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned);
void DMACopy(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned, const uint8_t _blockCountInMultiplesOf16bytes);
void DMATag(const uint32_t _tag);
uint32_t DMAPending();
void DMAResolveTiles(const uint32_t _rpuTileBuffer16ByteAligned, const uint32_t _gpuWritePage16ByteAligned)