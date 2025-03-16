#include "audio.h"
#include "SDL_thread.h"
#include <stdio.h>
#include <string.h>

#if defined(CAT_LINUX) || defined(CAT_MACOS)
char audiopdevicename[512] = "Headphones";
#else // CAT_WINDOWS
char audiopdevicename[512] = "Headphones";
#endif

const char* GetAudioPlaybackDeviceName()
{
	return audiopdevicename;
}

void SetAudioPlaybackDeviceName(const char* name)
{
	strcpy(audiopdevicename, name);
}

AudioPlayback::AudioPlayback()
{
}

AudioPlayback::~AudioPlayback()
{
}

void AudioPlayback::Initialize()
{
	int numPlay = SDL_GetNumAudioDevices(0);

	{
		uint32_t playdev = 0xFFFFFFFF;
		char playname[256] = {0};
		for (int i = 0; i < numPlay; i++)
		{
			const char* name = SDL_GetAudioDeviceName(i, 0);
			if (!name)
				continue;
			if (strstr(name, audiopdevicename) != nullptr)
			{
				fprintf(stderr, ">Using audio playback device(%d): %s\n", i, name);
				strcpy(playname, name);
				playdev = i;
				break;
			}
			else
				fprintf(stderr, " Found audio playback device(%d): %s\n", i, name);
		}

		if (playdev == 0xFFFFFFFF)
		{
			fprintf(stderr, "No audio playback device found\n");
			return;
		}
		else
		{
			SDL_AudioSpec audioSpecDesired, audioSpecObtained;
			SDL_zero(audioSpecDesired);
			SDL_zero(audioSpecObtained);
			audioSpecDesired.freq = 48000;
			audioSpecDesired.format = AUDIO_S16;
			audioSpecDesired.channels = 2;
			audioSpecDesired.samples = audioplaybackframes;
			audioSpecDesired.callback = nullptr;
			audioSpecDesired.userdata = this;

			selectedplaybackdevice = SDL_OpenAudioDevice(playname, 0, &audioSpecDesired, &audioSpecObtained, 0/*SDL_AUDIO_ALLOW_FORMAT_CHANGE*/);
			if (selectedplaybackdevice)
			{
				audioplaybackframes = audioSpecObtained.samples;
				fprintf(stderr, "Audio playback device opened: %d, capacity: %d\n", playdev, audioplaybackframes);
			}
		}
	}

	if (selectedplaybackdevice != 0xFFFFFFFF)
	{
		SDL_PauseAudioDevice(selectedplaybackdevice, 0);
		fprintf(stderr, "Audio playback device started\n");
	}
}

void AudioPlayback::Terminate()
{
	alive = 0;

	if (selectedplaybackdevice != 0xFFFFFFFF)
	{
		SDL_CloseAudioDevice(selectedplaybackdevice);
		selectedplaybackdevice = 0xFFFFFFFF;
	}
}

void AudioPlayback::Update()
{
	/*if (buffer == nullptr)
		buffer = new short[32768];*/

	//int len = SDL_DequeueAudio(selectedrecordingdevice, buffer, 65536);
	/*if (len)
		SDL_QueueAudio(selectedplaybackdevice, buffer, len);*/
}
