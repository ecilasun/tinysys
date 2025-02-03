#pragma once

#include "SDL_audio.h"

class AudioCapture
{
public:
	AudioCapture();
	~AudioCapture();

	void Initialize();
	void Terminate();
	void Update();

	short *buffer = nullptr;
	uint32_t selectedrecordingdevice = 0xFFFFFFFF;
	uint32_t selectedplaybackdevice = 0xFFFFFFFF;
	int audiocaptureframes = 8192;
	int audioplaybackframes = 8192;
	int alive = 1;
};

const char* GetAudioCaptureDeviceName();
const char* GetAudioPlaybackDeviceName();
void SetAudioCaptureDeviceName(const char* name);
void SetAudioPlaybackDeviceName(const char* name);
