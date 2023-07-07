#pragma once

#include <inttypes.h>

#define APUCMD_BUFFERSIZE  0x00000000
#define APUCMD_START       0x00000001
#define APUCMD_STOP        0x00000002
#define APUCMD_SWAP        0x00000003
#define APUCMD_SETRATE     0x00000004

enum EAPUSampleRate
{
	ASR_44100_Hz = 0,	// 44.1 KHz
	ASR_22050_Hz = 1,	// 22.05 KHz
	ASR_11025_Hz = 2	// 11.025 KHz
};

extern volatile uint32_t *IO_AUDIOOUT;

// Utilities
uint8_t *APUAllocateBuffer(const uint32_t _size);

void APUSetBufferSize(uint32_t audioBufferSize);
void APUStartDMA(uint32_t audioBufferAddress16byteAligned);
void APUStop();
void APUSwapBuffers();
void APUSetSampleRate(enum EAPUSampleRate sampleRate);

inline uint32_t APUFrame() { return *IO_AUDIOOUT; }
