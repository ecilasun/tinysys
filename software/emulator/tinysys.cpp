#include <stdio.h>
#include "emulator.h"

#ifdef CAT_WINDOWS
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

struct EmulatorContext
{
	CEmulator* emulator;
	SDL_Window* window;
	SDL_Window* debugwindow;
	SDL_Surface* surface;
};

static bool s_alive = true;

int emulatorthread(void* data)
{
	CEmulator* emulator = (CEmulator*)data;
	bool kicking;
	do
	{
		kicking = emulator->Step();
	} while(s_alive && kicking);

	s_alive = false;
	return 0;
}

void audioCallback(void* userdata, Uint8* stream, int len)
{
	if (len == 0)
		return;

	EmulatorContext* ctx = (EmulatorContext*)userdata;
	CAPU *apu = ctx->emulator->m_bus->GetAPU();
	uint8_t* source = (uint8_t*)apu->GetPlaybackData();

	// NOTE: Seems like we're getting an 8Kbyte buffer to fill for far fewer samples, use custom code instead of 'mix'
	//SDL_MixAudioFormat(stream, (uint8_t*)source, AUDIO_S16, len, SDL_MIX_MAXVOLUME);

	apu->FlipBuffers();
}

uint32_t videoCallback(Uint32 interval, void* param)
{
	EmulatorContext* ctx = (EmulatorContext*)param;
	if (SDL_MUSTLOCK(ctx->surface))
		SDL_LockSurface(ctx->surface);
	uint32_t* pixels = (uint32_t*)ctx->surface->pixels;
	ctx->emulator->UpdateVideoLink(pixels, ctx->surface->pitch);
	if (SDL_MUSTLOCK(ctx->surface))
		SDL_UnlockSurface(ctx->surface);
	SDL_UpdateWindowSurface(ctx->window);
	return interval;
}

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	printf("tinysys emulator v0.4\n");

	EmulatorContext ectx;
	ectx.emulator = new CEmulator;
	bool success;

	if (argc<=1)
		success = ectx.emulator->Reset("rom.bin");
	else
		success = ectx.emulator->Reset(argv[1]);

	if (!success)
	{
		printf("Failed to load ROM\n");
		return -1;
	}

	const int WIDTH = 640;
	const int HEIGHT = 480;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}
	ectx.window = SDL_CreateWindow("tinysys emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

#if defined(MEM_DEBUG)
	ectx.debugwindow = SDL_CreateWindow("memdbg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 1024, SDL_WINDOW_SHOWN);
#endif

	ectx.surface = SDL_GetWindowSurface(ectx.window);
	if (!ectx.surface)
	{
		printf("Could not create window surface\n");
		return -1;
	}

#if defined(MEM_DEBUG)
	SDL_Surface* debugsurface = SDL_GetWindowSurface(ectx.debugwindow);
	if (!debugsurface)
	{
		printf("Could not create window surface\n");
		return -1;
	}
#endif

	SDL_Thread* thread = SDL_CreateThread(emulatorthread, "emulator", ectx.emulator);
	SDL_TimerID videoTimer = SDL_AddTimer(13, videoCallback, &ectx); // 60fps

	// Enumerate audio output devices
	/*for (int i = 0; i < SDL_GetNumAudioDevices(0); i++)
	{
		printf("Audio device %d : %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}*/

	SDL_AudioSpec audioSpecDesired, audioSpecObtained;
	SDL_zero(audioSpecDesired);
	SDL_zero(audioSpecObtained);
	audioSpecDesired.freq = 22050;
	audioSpecDesired.format = AUDIO_S16;
	audioSpecDesired.channels = 2;
	audioSpecDesired.samples = 1024;
	audioSpecDesired.callback = audioCallback;
	audioSpecDesired.userdata = &ectx;

	int dev = SDL_OpenAudioDevice(nullptr, 0, &audioSpecDesired, &audioSpecObtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (dev != 0)
		SDL_PauseAudioDevice(dev, 0);

	SDL_Event ev;
	do
	{
		if (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				s_alive = false;
			else if (ev.type == SDL_KEYUP)
			{
				const Uint8* state = SDL_GetKeyboardState(nullptr);
				if (state[SDL_SCANCODE_LCTRL] && ev.key.keysym.sym == 'c')
					ectx.emulator->QueueByte(3);
				else if (ev.key.keysym.sym != SDLK_LCTRL)
					ectx.emulator->QueueByte(ev.key.keysym.sym);
			}
		}

		// Memory debug view
#if defined(MEM_DEBUG)
		if ((ectx.emulator->m_steps % 16384) == 0)
		{
			if (SDL_MUSTLOCK(debugsurface))
				SDL_LockSurface(debugsurface);
			// Update memory bitmap
			uint32_t* pixels = (uint32_t*)debugsurface->pixels;
			ectx.emulator->FillMemBitmap(pixels);
			if (SDL_MUSTLOCK(debugsurface))
				SDL_UnlockSurface(debugsurface);
			SDL_UpdateWindowSurface(ectx.debugwindow);
		}
#endif
	} while(s_alive);

	SDL_RemoveTimer(videoTimer);
	SDL_WaitThread(thread, nullptr);
	SDL_FreeSurface(ectx.surface);
	SDL_DestroyWindow(ectx.window);
	SDL_CloseAudioDevice(dev);
	SDL_Quit();

	return 0;
}
