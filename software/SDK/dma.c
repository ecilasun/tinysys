/**
 * @file dma.c
 * 
 * @brief DMA functions
 * 
 * This file provides functions for interacting with the DMA hardware.
 * It includes functions for copying data between memory locations and resolving 4x1 rasterizer tiles to 2x2 pixel blocks.
 */

#include "basesystem.h"
#include "dma.h"
#include "core.h"

// NOTE: Writes to this address will end up in the DMA command FIFO
// NOTE: Reads from this address will return DMA operation status
volatile uint32_t *DMAIO = (volatile uint32_t* ) DEVICE_DMAC;

/**
 * @brief Copy 4K of data from source to target
 * 
 * The function will copy 4K of data in 16 byte chunks (256 blocks) from source to target.
 * The source and target addresses must be 16 byte aligned.
 * 
 * @param _sourceAddress16ByteAligned 16 byte aligned source address
 * @param _targetAddress16ByteAligned 16 byte aligned target address
 */
void DMACopy4K(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned)
{
	*DMAIO = DMACMD_SETSOURCE;
	*DMAIO = _sourceAddress16ByteAligned;
	*DMAIO = DMACMD_SETTARGET;
	*DMAIO = _targetAddress16ByteAligned;
	*DMAIO = DMACMD_SETLENGHT;
	*DMAIO = 255;
	*DMAIO = DMACMD_ENQUEUE;
}

/**
 * @brief Copy data from source to target
 * 
 * The function will copy data in 16 byte chunks (256 blocks) from source to target.
 * The source and target addresses must be 16 byte aligned.
 * Any zero bytes in the data will be ignored and not written to the target.
 * 
 * @param _sourceAddress16ByteAligned 16 byte aligned source address
 * @param _targetAddress16ByteAligned 16 byte aligned target address
 */
void DMACopyAutoByteMask4K(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned)
{
	*DMAIO = DMACMD_SETSOURCE;
	*DMAIO = _sourceAddress16ByteAligned;
	*DMAIO = DMACMD_SETTARGET;
	*DMAIO = _targetAddress16ByteAligned;
	*DMAIO = DMACMD_SETLENGHT;
	*DMAIO = 255;
	*DMAIO = DMACMD_ENQUEUE_AUTOBYTEMASK;
}

/**
 * @brief Copy data from source to target
 * 
 * The function will copy data in given multiples of 16 byte chunks from source to target (up to 256 blocks).
 * The source and target addresses must be 16 byte aligned.
 * 
 * @param _sourceAddress16ByteAligned 16 byte aligned source address
 * @param _targetAddress16ByteAligned 16 byte aligned target address
 * @param _blockCountInMultiplesOf16bytes Number of 16 byte blocks to copy
 */
void DMACopy(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned, const uint8_t _blockCountInMultiplesOf16bytes)
{
	*DMAIO = DMACMD_SETSOURCE;
	*DMAIO = _sourceAddress16ByteAligned;
	*DMAIO = DMACMD_SETTARGET;
	*DMAIO = _targetAddress16ByteAligned;
	*DMAIO = DMACMD_SETLENGHT;
	*DMAIO = (_blockCountInMultiplesOf16bytes-1);
	*DMAIO = DMACMD_ENQUEUE;
}

/**
 * @brief Copy data from source to target
 * 
 * The function will copy data in given multiples of 16 byte chunks from source to target (up to 256 blocks).
 * The source and target addresses must be 16 byte aligned.
 * Any zero bytes in the data will be ignored and not written to the target.
 * 
 * @param _sourceAddress16ByteAligned 16 byte aligned source address
 * @param _targetAddress16ByteAligned 16 byte aligned target address
 * @param _blockCountInMultiplesOf16bytes Number of 16 byte blocks to copy
 */
void DMACopyAutoByteMask(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned, const uint8_t _blockCountInMultiplesOf16bytes)
{
	*DMAIO = DMACMD_SETSOURCE;
	*DMAIO = _sourceAddress16ByteAligned;
	*DMAIO = DMACMD_SETTARGET;
	*DMAIO = _targetAddress16ByteAligned;
	*DMAIO = DMACMD_SETLENGHT;
	*DMAIO = (_blockCountInMultiplesOf16bytes-1);
	*DMAIO = DMACMD_ENQUEUE_AUTOBYTEMASK;
}

/**
 * @brief Set DMA tag
 * 
 * The function will set a tag value that can be used to identify a DMA operation.
 * @note The tag value is not used by the DMA hardware at this point in time, but can still be used to synchronize DMA with software.
 * 
 * @param _tag Tag value
 */
void DMATag(const uint32_t _tag)
{
	*DMAIO = DMACMD_SETTAG;
	*DMAIO = _tag;
}

/**
 * @brief Resolve 4x1 rasterizer tiles
 * 
 * The function will resolve 4x1 rasterizer tiles from source to target and change their layout to 2x2 pixel blocks.
 * The source and target addresses must be 16 byte aligned.
 * 
 * @note This function is a software emulation of the hardware tile resolver, and isn't used just yet.
 * 
 * @param _rpuTileBuffer16ByteAligned 16 byte aligned source address
 * @param _vpuWritePage16ByteAligned 16 byte aligned target address
 */
void DMAResolveTiles(const uint32_t _rpuTileBuffer16ByteAligned, const uint32_t _vpuWritePage16ByteAligned)
{
	// NOTE: 4x1 tile software version is a straigth memcpy
	__builtin_memcpy((void*)_vpuWritePage16ByteAligned, (void*)_rpuTileBuffer16ByteAligned, 240*320);

	// TODO: Let hardware handle this
	/*
	*DMAIO = DMACMD_SETSOURCE;
	*DMAIO = _rpuTileBuffer16ByteAligned;
	*DMAIO = DMACMD_SETTARGET;
	*DMAIO = _vpuWritePage16ByteAligned;
	*DMAIO = DMACMD_RESOLVETILES;*/

	// Software emulation
	/*for (uint32_t ty=0; ty<60; ++ty) // 240/4
	{
		uint32_t Y = _rpuTileBuffer16ByteAligned + ty*80*16;
		uint32_t U = _vpuWritePage16ByteAligned + ty*4*320;
		for (uint32_t tx=0;tx<80;++tx) // 320/4
		{
			// Read 16 byte source
			uint32_t *tilebuffer = (uint32_t*)(Y+tx*16);
			uint32_t T0 = tilebuffer[0];
			uint32_t T1 = tilebuffer[1];
			uint32_t T2 = tilebuffer[2];
			uint32_t T3 = tilebuffer[3];

			// Expand onto target
			uint32_t *writepageasword = (uint32_t*)(U+tx*4);
			writepageasword[0] = T0;
			writepageasword[80] = T1;
			writepageasword[160] = T2;
			writepageasword[240] = T3;
		}
	}*/
	CFLUSH_D_L1;
}

/**
 * @brief Check if there are pending DMA operations
 * 
 * The function will return non-zero if there are pending DMA operations.
 * To block and wait for all DMA operations to complete, use DMAWait.
 * 
 * @return Number of pending DMA operations
 */
uint32_t DMAPending()
{
	return *DMAIO;
}

/**
 * @brief Wait for all DMA operations to complete
 * 
 * The function will wait for all DMA operations to complete before returning.
 * The coherence mode can be set to CPUCoherent to make sure the CPU sees all DMA writes.
 * 
 * @param _cpucoherency CPU coherency mode
 */
void DMAWait(enum ECPUCoherency _cpucoherency)
{
	while (*DMAIO) { asm volatile("nop;"); }

	if (_cpucoherency == CPUCoherent)
	{
		// Make sure we flush the cache to memory to make sure we don't lose anything.
		CFLUSH_D_L1;
		// Then we discard cache contents.
		// This allows the CPU to reload cache lines so it can
		// see the DMA writes
		CDISCARD_D_L1;
	}
}
