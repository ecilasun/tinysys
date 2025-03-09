#pragma once

#include "SDL_audio.h"

class AudioPlayback
{
public:
	AudioPlayback();
	~AudioPlayback();

	void Initialize();
	void Terminate();
	void Update();

	short *buffer = nullptr;
	uint32_t selectedplaybackdevice = 0xFFFFFFFF;
	int audioplaybackframes = 8192;
	int alive = 1;
};

const char* GetAudioPlaybackDeviceName();
void SetAudioPlaybackDeviceName(const char* name);
