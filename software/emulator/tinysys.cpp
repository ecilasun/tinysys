#include <stdio.h>
#include "emulator.h"

#include "SDL.h"

#ifdef CAT_WINDOWS
// 
#else
#include "SDL_ttf.h"
#endif

static int AudioQueueCapacity = 1024;	// Size of the audio queue in samples
const int QueueSampleCount = 64;		// Push this many samples to the queue per iteration
static int AudioQueueCapacityInBytes = AudioQueueCapacity*sizeof(uint16_t)*2;
const int QueueSampleByteSize = QueueSampleCount*sizeof(int16_t)*2;

static TTF_Font* s_debugfont = nullptr;
static SDL_Surface* s_textSurface = nullptr;
static uint32_t s_logotime = 0;

struct EmulatorContext
{
	CEmulator* emulator;
	SDL_Window* window;
	SDL_Surface* surface;
};

static bool s_alive = true;

int emulatorthread(void* data)
{
	CEmulator* emulator = (CEmulator*)data;
	do
	{
		emulator->Step();
	} while(s_alive);

	s_alive = false;
	return 0;
}

int audiothread(void* data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);

	CEmulator* emulator = (CEmulator*)data;
	uint32_t pastSelector = 0xFF;
	CAPU *apu = emulator->m_bus->GetAPU();
	int queueReadCursor = 0;
	uint16_t *tmpbuf = new uint16_t[QueueSampleCount*8];
	int spaceLeft = AudioQueueCapacityInBytes;
	do
	{
		if (apu->m_rateselector != pastSelector)
		{
			pastSelector = apu->m_rateselector;
			SDL_PauseAudioDevice(emulator->m_audioDevice, pastSelector ? 0 : 1);
		}

		// Check if we've got enough space in the audio queue to fit our 128 samples
		if (pastSelector && (spaceLeft >= QueueSampleByteSize))
		{
			uint8_t* source = ((uint8_t*)apu->GetPlaybackData()) + queueReadCursor;

			// Queue more samples if we found some room
			if (pastSelector == 4)			// 44.1kHz
			{
				SDL_QueueAudio(emulator->m_audioDevice, source, QueueSampleByteSize);
				queueReadCursor += QueueSampleByteSize;
			}
			else if (pastSelector == 2)		// 22.05kHz
			{
				uint16_t* p = (uint16_t*)source;
				for (int i = 0; i < QueueSampleCount/2; i++)
				{
					uint16_t L = p[i * 2 + 0];
					uint16_t R = p[i * 2 + 1];
					tmpbuf[i * 4 + 0] = L;
					tmpbuf[i * 4 + 1] = R;
					tmpbuf[i * 4 + 2] = L;
					tmpbuf[i * 4 + 3] = R;
				}
				SDL_QueueAudio(emulator->m_audioDevice, tmpbuf, QueueSampleByteSize);
				queueReadCursor += QueueSampleByteSize/2;
			}
			else if (pastSelector == 1)		// 11.025kHz
			{
				uint16_t* p = (uint16_t*)source;
				for (int i = 0; i < QueueSampleCount/4; i++)
				{
					uint16_t L = p[i * 2 + 0];
					uint16_t R = p[i * 2 + 1];
					tmpbuf[i * 8 + 0] = L;
					tmpbuf[i * 8 + 1] = R;
					tmpbuf[i * 8 + 2] = L;
					tmpbuf[i * 8 + 3] = R;
					tmpbuf[i * 8 + 4] = L;
					tmpbuf[i * 8 + 5] = R;
					tmpbuf[i * 8 + 6] = L;
					tmpbuf[i * 8 + 7] = R;
				}
				SDL_QueueAudio(emulator->m_audioDevice, tmpbuf, QueueSampleByteSize);
				queueReadCursor += QueueSampleByteSize/4;
			}
			// else if (pastSelector == 0) {} // audio is paused
		}

		// Bytes queued reached APU word count, swap
		if (queueReadCursor >= apu->m_apuwordcount*sizeof(int32_t))
		{
			queueReadCursor = 0;
			apu->FlipBuffers();
		}

		if (pastSelector)
		{
			int bytesQueued = SDL_GetQueuedAudioSize(emulator->m_audioDevice);
			spaceLeft = AudioQueueCapacityInBytes-bytesQueued;
		}
		else
			spaceLeft = AudioQueueCapacityInBytes;

	} while(s_alive);

	delete [] tmpbuf;

	return 0;
}

uint32_t videoCallback(Uint32 interval, void* param)
{
	EmulatorContext* ctx = (EmulatorContext*)param;
	if (SDL_MUSTLOCK(ctx->surface))
		SDL_LockSurface(ctx->surface);

	uint32_t* pixels = (uint32_t*)ctx->surface->pixels;
	ctx->emulator->UpdateVideoLink(pixels, ctx->surface->pitch);

	uint32_t W = ctx->surface->w;
	uint32_t H = ctx->surface->h-8;
	uint32_t S = ctx->emulator->m_bus->GetLEDs()->m_ledstate;

	// TODO: LED image instead of flat colors
	{
		uint32_t L1 = S&0x1 ?  0xFFFF7F00 : 0xFF201000;
		uint32_t L2 = S&0x2 ?  0xFFFF7F00 : 0xFF201000;
		uint32_t L3 = S&0x4 ?  0xFFFF7F00 : 0xFF201000;
		uint32_t L4 = S&0x8 ?  0xFFFF7F00 : 0xFF201000;
		
		for (uint32_t j = H; j < H+8; j++)
		{
			for (uint32_t i = 8; i < 16; i++)
			{
				pixels[W*j+i] = L1;
				pixels[W*j+i+9] = L2;
				pixels[W*j+i+18] = L3;
				pixels[W*j+i+27] = L4;
			}
		}
	}

	if (s_logotime < 120)
		SDL_BlitSurface(s_textSurface, nullptr, ctx->surface, nullptr);
	++s_logotime;

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
	fprintf(stderr, "tinysys emulator v0.5\n");

	EmulatorContext ectx;
	ectx.emulator = new CEmulator;
	bool success;

	const uint32_t resetvector = 0x0FFE0000;
	if (argc<=1)
		success = ectx.emulator->Reset("rom.bin", resetvector);
	else
		success = ectx.emulator->Reset(argv[1], resetvector);

	if (!success)
	{
		fprintf(stderr, "Failed to load ROM\n");
		return -1;
	}

	const int WIDTH = 640;
	const int HEIGHT = 480;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}
	ectx.window = SDL_CreateWindow("tinysys emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT+8, SDL_WINDOW_SHOWN);

	if (TTF_Init() != 0)
	{
		fprintf(stderr, "Error initializing SDL2_ttf: %s\n", TTF_GetError());
		return -1;
	}
	s_debugfont = TTF_OpenFont("DejaVuSansMono.ttf", 16);
	if (!s_debugfont)
	{
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return -1;
	}

	ectx.surface = SDL_GetWindowSurface(ectx.window);
	if (!ectx.surface)
	{
		fprintf(stderr, "Could not create window surface\n");
		return -1;
	}

	SDL_AudioSpec audioSpecDesired, audioSpecObtained;
	SDL_zero(audioSpecDesired);
	SDL_zero(audioSpecObtained);
	audioSpecDesired.freq = 44100;
	audioSpecDesired.format = AUDIO_S16;
	audioSpecDesired.channels = 2;
	audioSpecDesired.samples = AudioQueueCapacity;
	audioSpecDesired.callback = nullptr;
	audioSpecDesired.userdata = &ectx;

	int dev = SDL_OpenAudioDevice(nullptr, 0, &audioSpecDesired, &audioSpecObtained, 0/*SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE*/);
	ectx.emulator->m_audioDevice = dev;
	if (dev)
		AudioQueueCapacity = audioSpecObtained.samples;

	// Enumerate audio output devices
	fprintf(stderr, "Enumerating audio devices:\n");
	for (int i = 0; i < SDL_GetNumAudioDevices(0); i++)
	{
		fprintf(stderr, "  #%d : %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}
	fprintf(stderr, "Using device #%d with audio queue size of %d samples\n", dev, AudioQueueCapacity);

	SDL_Thread* emulatorthreadID = SDL_CreateThread(emulatorthread, "emulator", ectx.emulator);
	SDL_Thread* audiothreadID = SDL_CreateThread(audiothread, "audio", ectx.emulator);
	SDL_TimerID videoTimer = SDL_AddTimer(16, videoCallback, &ectx); // 60fps

	s_textSurface = TTF_RenderText_Blended(s_debugfont, "tinysys emulator", {255,255,255});

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

		// Print CPU stats
#if defined(CPU_STATS)
		if ((ectx.emulator->m_steps % 16384) == 0)
		{
			CRV32 *cpu0 = ectx.emulator->m_cpu[0];
			printf("I$  read hits / misses: %d %d\n", cpu0->m_icache.m_hits, cpu0->m_icache.m_misses);
			printf("D$  read hits / misses: %d %d\n", cpu0->m_dcache.m_readhits, cpu0->m_dcache.m_readmisses);
			printf("   write hits / misses: %d %d\n", cpu0->m_dcache.m_writehits, cpu0->m_dcache.m_writemisses);
			printf("EX retired instructions: %lld\n", cpu0->m_retired);
			printf("EX conditional branches taken / not taken: %d / %d\n", cpu0->m_btaken, cpu0->m_bntaken);
			printf("EX unconditional branches taken: %d\n", cpu0->m_ucbtaken);
		}
#endif
	} while(s_alive);

	TTF_CloseFont(s_debugfont);
	TTF_Quit();
	SDL_RemoveTimer(videoTimer);
	SDL_WaitThread(emulatorthreadID, nullptr);
	SDL_WaitThread(audiothreadID, nullptr);
	SDL_ClearQueuedAudio(ectx.emulator->m_audioDevice);
	SDL_FreeSurface(ectx.surface);
	SDL_DestroyWindow(ectx.window);
	SDL_CloseAudioDevice(ectx.emulator->m_audioDevice);
	SDL_Quit();

	return 0;
}
