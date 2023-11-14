#include "basesystem.h"
#include "dma.h"
#include "core.h"

// NOTE: Writes to this address will end up in the DMA command FIFO
// NOTE: Reads from this address will return DMA operation status
volatile uint32_t *DMAIO = (volatile uint32_t* ) DEVICE_DMAC;

// Simple DMA with aligned start and target addresses, does _16byteBlockCount x 128bit copies (no masking, no byte aligned copies)
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

void DMATag(const uint32_t _tag)
{
	*DMAIO = DMACMD_SETTAG;
	*DMAIO = _tag;
}

void DMAResolveTiles(const uint32_t _rpuTileBuffer16ByteAligned, const uint32_t _gpuWritePage16ByteAligned)
{
	// NOTE: 4x1 tile software version is a straigth memcpy
	__builtin_memcpy((void*)_gpuWritePage16ByteAligned, (void*)_rpuTileBuffer16ByteAligned, 240*320);

	// TODO: Let hardware handle this
	/*
	*DMAIO = DMACMD_SETSOURCE;
	*DMAIO = _rpuTileBuffer16ByteAligned;
	*DMAIO = DMACMD_SETTARGET;
	*DMAIO = _gpuWritePage16ByteAligned;
	*DMAIO = DMACMD_RESOLVETILES;*/

	// Software emulation
	/*for (uint32_t ty=0; ty<60; ++ty) // 240/4
	{
		uint32_t Y = _rpuTileBuffer16ByteAligned + ty*80*16;
		uint32_t U = _gpuWritePage16ByteAligned + ty*4*320;
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

uint32_t DMAPending()
{
	return *DMAIO;
}

void DMAWait()
{
	while (*DMAIO) { asm volatile("nop;"); }
}
