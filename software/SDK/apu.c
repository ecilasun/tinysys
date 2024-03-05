#include "basesystem.h"
#include "apu.h"
#include "core.h"
#include <stdlib.h>

volatile uint32_t *IO_AUDIOOUT = (volatile uint32_t* ) DEVICE_APUC;

// APU buffers are allocated aligned to 64byte boundaries
uint8_t *APUAllocateBuffer(const uint32_t _size)
{
   void *buffer = (uint8_t*)malloc(_size + 64);
   return (uint8_t*)E32AlignUp((uint32_t)buffer, 64);
}

void APUSetBufferSize(uint32_t audioBufferSize)
{
    *IO_AUDIOOUT = APUCMD_BUFFERSIZE;
    // NOTE: Hardware requires this to be in words, minus one
    // 16byte DMA count is therefore (audioBufferSize-1)>>2
    // e.g. for a 512 byte buffer, length would be sent as 511 and burst count would be 127
    *IO_AUDIOOUT = audioBufferSize-1;
}

void APUStartDMA(uint32_t audioBufferAddress16byteAligned)
{
    *IO_AUDIOOUT = APUCMD_START;
    *IO_AUDIOOUT = audioBufferAddress16byteAligned;
}

void APUSetSampleRate(enum EAPUSampleRate sampleRate)
{
    *IO_AUDIOOUT = APUCMD_SETRATE;
    *IO_AUDIOOUT = (uint32_t)sampleRate;
}
