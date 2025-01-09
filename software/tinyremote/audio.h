#pragma once

#include "SDL_audio.h"

class AudioCapture
{
public:
	AudioCapture();
	~AudioCapture();

	void Initialize();
	void Terminate();

	uint32_t selectedrecordingdevice = 0xFFFFFFFF;
	uint32_t selectedplaybackdevice = 0xFFFFFFFF;
	int audioqueuecapacity = 1024;
	int alive = 1;
};
