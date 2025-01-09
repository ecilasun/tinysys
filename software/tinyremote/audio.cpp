#include "audio.h"
#include "SDL_thread.h"
#include <stdio.h>
#include <string.h>

void audioCaptureCallback(void *userdata, Uint8 *stream, int len)
{
	AudioCapture* audio = (AudioCapture*)userdata;

	//int bytesRead = SDL_DequeueAudio(audio->selectedrecordingdevice, stream, len); 
	//if (bytesRead)
	if (len)
		SDL_QueueAudio(audio->selectedplaybackdevice, stream, len);
}

AudioCapture::AudioCapture()
{
}

AudioCapture::~AudioCapture()
{
}

void AudioCapture::Initialize()
{
	int numCap = SDL_GetNumAudioDevices(1);
	fprintf(stderr, "Found %d audio capture devices\n", numCap);
	{
		uint32_t capdev = 0xFFFFFFFF;
		char capname[256] = {0};
		for (int i = 0; i < numCap; i++)
		{
			const char* name = SDL_GetAudioDeviceName(i, 1);
			if (strstr(name, "Line In") != NULL)
			{
				fprintf(stderr, "Using audio capture device(%d): %s\n", i, name);
				strcpy(capname, name);
				capdev = i;
				break;
			}
		}

		if (capdev == 0xFFFFFFFF)
		{
			fprintf(stderr, "No line in device found\n");
			return;
		}

		SDL_AudioSpec audioSpecDesired, audioSpecObtained;
		SDL_zero(audioSpecDesired);
		SDL_zero(audioSpecObtained);
		audioSpecDesired.freq = 44100;
		audioSpecDesired.format = AUDIO_S16;
		audioSpecDesired.channels = 2;
		audioSpecDesired.samples = audioqueuecapacity;
		audioSpecDesired.callback = audioCaptureCallback;
		audioSpecDesired.userdata = this;

		selectedrecordingdevice = SDL_OpenAudioDevice(capname, 1, &audioSpecDesired, &audioSpecObtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
		if (selectedrecordingdevice)
		{
			audioqueuecapacity = audioSpecObtained.samples;
			fprintf(stderr, "Audio capture device opened: %d, capacity: %d\n", capdev, audioqueuecapacity);
		}
	}

	int numPlay = SDL_GetNumAudioDevices(0);
	fprintf(stderr, "Found %d audio playback devices\n", numPlay);

	{
		uint32_t playdev = 0xFFFFFFFF;
		char playname[256] = {0};
		for (int i = 0; i < numCap; i++)
		{
			const char* name = SDL_GetAudioDeviceName(i, 0);
			if (strstr(name, "Head") != NULL)
			{
				fprintf(stderr, "Using audio playback device(%d): %s\n", i, name);
				strcpy(playname, name);
				playdev = i;
				break;
			}
		}

		if (playdev == 0xFFFFFFFF)
		{
			fprintf(stderr, "No audio playback device found\n");
			return;
		}

		SDL_AudioSpec audioSpecDesired, audioSpecObtained;
		SDL_zero(audioSpecDesired);
		SDL_zero(audioSpecObtained);
		audioSpecDesired.freq = 44100;
		audioSpecDesired.format = AUDIO_S16;
		audioSpecDesired.channels = 2;
		audioSpecDesired.samples = audioqueuecapacity;
		audioSpecDesired.callback = nullptr;
		audioSpecDesired.userdata = this;

		selectedplaybackdevice = SDL_OpenAudioDevice(playname, 0, &audioSpecDesired, &audioSpecObtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
		if (selectedplaybackdevice)
		{
			fprintf(stderr, "Audio playback device opened: %d, capacity: %d\n", playdev, audioqueuecapacity);
		}
	}

	if (selectedplaybackdevice != 0xFFFFFFFF)
	{
		SDL_PauseAudioDevice(selectedplaybackdevice, 0);
		fprintf(stderr, "Audio playback device started\n");
	}
	if (selectedrecordingdevice != 0xFFFFFFFF)
	{
		SDL_PauseAudioDevice(selectedrecordingdevice, 0);
		fprintf(stderr, "Audio capture device started\n");
	}
}

void AudioCapture::Terminate()
{
	alive = 0;

	if (selectedrecordingdevice != 0xFFFFFFFF)
	{
		SDL_CloseAudioDevice(selectedrecordingdevice);
		selectedrecordingdevice = 0xFFFFFFFF;
	}

	if (selectedplaybackdevice != 0xFFFFFFFF)
	{
		SDL_CloseAudioDevice(selectedplaybackdevice);
		selectedplaybackdevice = 0xFFFFFFFF;
	}
}
