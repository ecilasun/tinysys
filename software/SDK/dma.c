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

uint32_t DMAPending()
{
    return *DMAIO;
}
