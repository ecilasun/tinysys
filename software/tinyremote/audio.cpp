#include "audio.h"
#include "SDL_thread.h"
#include <stdio.h>
#include <string.h>

#if defined(CAT_LINUX) || defined(CAT_MACOS)
char audiocdevicename[512] = "Line In";
char audiopdevicename[512] = "Headphones";
#else // CAT_WINDOWS
char audiocdevicename[512] = "capture";
char audiopdevicename[512] = "Headphones";
#endif

const char* GetAudioCaptureDeviceName()
{
	return audiocdevicename;
}

const char* GetAudioPlaybackDeviceName()
{
	return audiopdevicename;
}

void SetAudioCaptureDeviceName(const char* name)
{
	strcpy(audiocdevicename, name);
}

void SetAudioPlaybackDeviceName(const char* name)
{
	strcpy(audiopdevicename, name);
}

void audioCaptureCallback(void *userdata, Uint8 *stream, int len)
{
	AudioCapture* audio = (AudioCapture*)userdata;

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
	{
		uint32_t capdev = 0xFFFFFFFF;
		char capname[256] = {0};
		for (int i = 0; i < numCap; i++)
		{
			const char* name = SDL_GetAudioDeviceName(i, 1);
			if (!name)
				continue;
			if (strstr(name, audiocdevicename) != nullptr)
			{
				fprintf(stderr, "Using audio capture device(%d): %s\n", i, name);
				strcpy(capname, name);
				capdev = i;
				break;
			}
			else
				fprintf(stderr, "Found audio capture device(%d): %s\n", i, name);
		}

		if (capdev == 0xFFFFFFFF)
		{
			fprintf(stderr, "No line in device found\n");
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
			audioSpecDesired.samples = audiocaptureframes;
			audioSpecDesired.callback = audioCaptureCallback;
			audioSpecDesired.userdata = this;

			selectedrecordingdevice = SDL_OpenAudioDevice(capname, 1, &audioSpecDesired, &audioSpecObtained, 0/*SDL_AUDIO_ALLOW_FORMAT_CHANGE*/);
			if (selectedrecordingdevice)
			{
				audiocaptureframes = audioSpecObtained.samples;
				fprintf(stderr, "Audio capture device opened: %d, capacity: %d\n", capdev, audiocaptureframes);
			}
		}
	}

	int numPlay = SDL_GetNumAudioDevices(0);

	{
		uint32_t playdev = 0xFFFFFFFF;
		char playname[256] = {0};
		for (int i = 0; i < numCap; i++)
		{
			const char* name = SDL_GetAudioDeviceName(i, 0);
			if (!name)
				continue;
			if (strstr(name, audiopdevicename) != nullptr)
			{
				fprintf(stderr, "Using audio playback device(%d): %s\n", i, name);
				strcpy(playname, name);
				playdev = i;
				break;
			}
			else
				fprintf(stderr, "Found audio playback device(%d): %s\n", i, name);
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

void AudioCapture::Update()
{
	if (buffer == nullptr)
		buffer = new short[32768];

	int len = SDL_DequeueAudio(selectedrecordingdevice, buffer, 65536);
	if (len)
		SDL_QueueAudio(selectedplaybackdevice, buffer, len);
}
