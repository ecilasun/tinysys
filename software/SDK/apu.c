/**
 * @file apu.c
 * 
 * @brief Audio Processing Unit (APU) interface.
 *
 * This file provides functions for interacting with the Audio Processing Unit (APU).
 * It includes functions for allocating buffers, setting buffer size, starting audio DMA, and setting the sample rate.
 */

#include "basesystem.h"
#include "apu.h"
#include "core.h"
#include <stdlib.h>

volatile uint32_t *IO_AUDIOOUT = (volatile uint32_t* ) DEVICE_APUC;

/**
 * @brief Allocates a buffer for the APU.
 *
 * The buffer is allocated with an alignment of 64 bytes.
 *
 * @param _size The size of the buffer to allocate.
 * @return A pointer to the allocated buffer.
 */
uint8_t *APUAllocateBuffer(const uint32_t _size)
{
   void *buffer = (uint8_t*)malloc(_size + 64);
   return (uint8_t*)E32AlignUp((uint32_t)buffer, 64);
}

/**
 * @brief Sets the buffer size for the APU.
 *
 * The hardware requires the size to be in words, minus one.
 * For example, for a 512 byte buffer, length would be sent as 511 and burst count would be 127.
 *
 * @param audioBufferSize The size of the buffer.
 */
void APUSetBufferSize(uint32_t audioBufferSize)
{
    *IO_AUDIOOUT = APUCMD_BUFFERSIZE;
    *IO_AUDIOOUT = audioBufferSize-1;
}

/**
 * @brief Starts an audio DMA transfer to the APU.
 * 
 * The audio data in source buffer is written to the current write page of the APU buffer.
 * APU will be playing the audio data from the read page while this DMA transfer is in progress.
 * Once the APU reaches the end of the read page, it will switch the write with the reead page and continue playing.
 *
 * @param audioBufferAddress16byteAligned The address of the buffer, which must be 16-byte aligned.
 */
void APUStartDMA(uint32_t audioBufferAddress16byteAligned)
{
    *IO_AUDIOOUT = APUCMD_START;
    *IO_AUDIOOUT = audioBufferAddress16byteAligned;
}

/**
 * @brief Sets the sample rate for the APU.
 * 
 * Valid sample rates are defined in the EAPUSampleRate enum and include 44.1 KHz, 22.05 KHz, 11.025 KHz.
 * To stop audio playback, set the sample rate to ASR_Halt.
 *
 * @param sampleRate The sample rate to set.
 */
void APUSetSampleRate(enum EAPUSampleRate sampleRate)
{
    *IO_AUDIOOUT = APUCMD_SETRATE;
    *IO_AUDIOOUT = (uint32_t)sampleRate;
}
