#include "tinysys.h"

static socket_t newsockfd;

static const char* emulatorVersionString = "tinysys emulator v0.7";

static int AudioQueueCapacity = 1024;	// Size of the audio queue in samples
const int QueueSampleCount = 64;		// Push this many samples to the queue per iteration
static int AudioQueueCapacityInBytes = AudioQueueCapacity*sizeof(uint16_t)*2;
const int QueueSampleByteSize = QueueSampleCount*sizeof(int16_t)*2;

static TTF_Font* s_debugfont = nullptr;
static SDL_Surface* s_textSurface = nullptr;
static uint32_t s_logotime = 0;

#if defined(CPU_STATS)
static SDL_Surface* s_statSurface = nullptr;
#endif

const int WIDTH = 640;
const int HEIGHT = 480;

struct EmulatorContext
{
	CEmulator* emulator{ nullptr };
	SDL_Window* window{ nullptr };
	SDL_Surface* compositesurface{ nullptr };
	SDL_GameController* gamecontroller{ nullptr };
};

static bool s_alive = true;
static uint64_t s_wallclock = 0;

#if defined(GALLIFREY)
int emulatorthreadcpu0(void* data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);

	EmulatorContext* ctx = (EmulatorContext*)data;
	int spins = 0;
	do
	{
		CEmulator *emulator = ctx->emulator;
		if (emulator->m_debugStop)
		{
			// Stop for debugger
			emulator->m_debugAck = 1;
		}
		else
		{
			emulator->m_debugAck = 0;
			emulator->Step(s_wallclock, 0);
		}

		if (spins % 1048576 == 0)
		{
			// Handle hardware switch etc changes
			emulator->m_bus->GetSDCard()->UpdateSDCardSwitch();
		}
		++spins;
	} while(s_alive);

	return 0;
}

int emulatorthreadcpu1(void* data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);

	EmulatorContext* ctx = (EmulatorContext*)data;
	do
	{
		CEmulator *emulator = ctx->emulator;
		emulator->Step(s_wallclock, 1);
	} while(s_alive);

	return 0;
}
#else
int emulatorthread(void* data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);

	EmulatorContext* ctx = (EmulatorContext*)data;
	int spins = 0;
	do
	{
		CEmulator *emulator = ctx->emulator;
		if (emulator->m_debugStop)
		{
			// Stop for debugger
			emulator->m_debugAck = 1;
		}
		else
		{
			emulator->m_debugAck = 0;
			emulator->Step(s_wallclock, 0);
			emulator->Step(s_wallclock, 1);
		}

		if (spins % 1048576 == 0)
		{
			// Handle hardware switch etc changes
			emulator->m_bus->GetSDCard()->UpdateSDCardSwitch();
		}
		++spins;
	} while(s_alive);

	return 0;
}
#endif

int audiothread(void* data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);

	CEmulator* emulator = (CEmulator*)data;
	uint32_t pastSelector = 0xFF;
	CAPU *apu = emulator->m_bus->GetAPU();
	int queueReadCursor = 0;
	uint32_t *tmpbuf = new uint32_t[QueueSampleCount*8];
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
					tmpbuf[i * 2 + 0] = R<<16 | L;
					tmpbuf[i * 2 + 1] = R<<16 | L;
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
					tmpbuf[i * 4 + 0] = R<<16 | L;
					tmpbuf[i * 4 + 1] = R<<16 | L;
					tmpbuf[i * 4 + 2] = R<<16 | L;
					tmpbuf[i * 4 + 3] = R<<16 | L;
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

float Clamp(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void ClampAnalogInput(Axis6& input, float innerDeadzone, float outerDeadzone)
{
	float magleft = input.leftx*input.leftx + input.lefty*input.lefty;
	float magright = input.rightx*input.rightx + input.righty*input.righty;

	if (magleft == 0.f)
	{
		input.leftx = 0;
		input.lefty = 0;
	}
	else
	{
		magleft = sqrtf(magleft);
		float scale = (magleft - innerDeadzone) / (outerDeadzone - innerDeadzone);
		input.leftx = input.leftx * Clamp(scale, 0, 1) / magleft;
		input.lefty = input.lefty * Clamp(scale, 0, 1) / magleft;
	}

	if (magright == 0.f)
	{
		input.rightx = 0;
		input.righty = 0;
	}
	else
	{
		magright = sqrtf(magright);
		float scale = (magright - innerDeadzone) / (outerDeadzone - innerDeadzone);
		input.rightx = input.rightx * Clamp(scale, 0, 1) / magright;
		input.righty = input.righty * Clamp(scale, 0, 1) / magright;
	}

	input.lefttrigger = Clamp(input.lefttrigger, 0, 1);
	input.righttrigger = Clamp(input.righttrigger, 0, 1);
}

void ControllerAdd(EmulatorContext& ctx)
{
	int numJoy = SDL_NumJoysticks();
	if (numJoy <= 0)
		fprintf(stderr, "No joysticks: %d\n", numJoy);
	else
		fprintf(stderr, "Found %d joysticks\n", numJoy);

	for (int i = 0; i < numJoy; ++i)
	{
		if (SDL_IsGameController(i))
		{
			ctx.gamecontroller = SDL_GameControllerOpen(i);
			if (ctx.gamecontroller)
			{
				if (SDL_GameControllerGetAttached(ctx.gamecontroller))
				{
					fprintf(stderr, "Using game controller #%d: '%s'\n", i, SDL_GameControllerName(ctx.gamecontroller));
					// Read button mappings
					for (int j = 0; j < SDL_CONTROLLER_BUTTON_MAX; ++j)
					{
						if (SDL_GameControllerHasButton(ctx.gamecontroller, (SDL_GameControllerButton)j))
						{
							SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(ctx.gamecontroller, (SDL_GameControllerButton)j);
							switch (bind.bindType)
							{
								case SDL_CONTROLLER_BINDTYPE_BUTTON:
									fprintf(stderr, "Button %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								break;
								case SDL_CONTROLLER_BINDTYPE_AXIS:
									fprintf(stderr, "Axis %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								break;
								case SDL_CONTROLLER_BINDTYPE_HAT:
									fprintf(stderr, "Hat %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								break;
								default:
								{
									fprintf(stderr, "Unknown %d: %s\n", j, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)j));
								}
							}
						}
					}
					break;
				}
			}
		}
	}
}

void ControllerRemove(EmulatorContext& ctx)
{
	if (ctx.gamecontroller)
	{
		SDL_GameControllerClose(ctx.gamecontroller);
		ctx.gamecontroller = nullptr;
	}
}

uint32_t videoCallback(Uint32 interval, void* param)
{
	EmulatorContext* ctx = (EmulatorContext*)param;

	if (SDL_MUSTLOCK(ctx->compositesurface))
		SDL_LockSurface(ctx->compositesurface);

	uint32_t* pixels = (uint32_t*)ctx->compositesurface->pixels;

	ctx->emulator->UpdateVideoLink(pixels, ctx->compositesurface->pitch);

	uint32_t W = ctx->compositesurface->w;
	uint32_t H = ctx->compositesurface->h-8;
	uint32_t S = ctx->emulator->m_bus->GetLEDs()->m_ledstate;

	// TODO: LED image instead of flat colors
	{
		uint32_t L1 = S&0x1 ?  0xFFFF0000 : 0xFF200000; // status RED
		uint32_t L2 = S&0x2 ?  0xFF00FF00 : 0xFF002000; // status GREEN
		uint32_t L1_2 = L1 | L2;

		uint32_t L3 = S&0x4 ?  0xFFFF7F00 : 0xFF201000; // Debug EMBER
		uint32_t L4 = S&0x8 ?  0xFFFF7F00 : 0xFF201000;
		uint32_t L5 = S&0x10 ?  0xFFFF7F00 : 0xFF201000;
		uint32_t L6 = S&0x20 ?  0xFFFF7F00 : 0xFF201000;
		
		for (uint32_t j = H; j < H+8; j++)
		{
			for (uint32_t i = 8; i < 16; i++)
			{
				pixels[W*j+i] = L1_2;
				pixels[W*j+i+9] = L3;
				pixels[W*j+i+18] = L4;
				pixels[W*j+i+27] = L5;
				pixels[W*j+i+36] = L6;
			}
		}
	}

	// Center the splash image
	SDL_Rect splashRect = s_textSurface ? s_textSurface->clip_rect : SDL_Rect();
	splashRect.x = (W-splashRect.w)/2;
	splashRect.y = (H-splashRect.h)/2;

	// Stay up for 2 seconds
	if (s_logotime < 120)
	{
		int m = 0;
		int top = splashRect.y-4;
		int bottom = splashRect.y+splashRect.h+4;
		int d = bottom-top;
		for (int j = top; j < bottom; j++)
		{
			m = 255*(j-top)/d;
			for (uint32_t i = 0; i < W; i++)
				pixels[W*j+i] = 0xFF000000 | (m);
		}
	}

	if (SDL_MUSTLOCK(ctx->compositesurface))
		SDL_UnlockSurface(ctx->compositesurface);

	if (s_logotime < 120 && s_textSurface)
		SDL_BlitSurface(s_textSurface, nullptr, ctx->compositesurface, &splashRect);

	++s_logotime;

#if defined(CPU_STATS)
	if (s_statSurface)
	{
		SDL_Rect statRect = s_statSurface->clip_rect;
		statRect.y = H-statRect.h;

		SDL_BlitSurface(s_statSurface, nullptr, ctx->compositesurface, &statRect);
	}
#endif

	// Update window surface
	SDL_BlitScaled(ctx->compositesurface, nullptr, SDL_GetWindowSurface(ctx->window), nullptr);
	SDL_UpdateWindowSurface(ctx->window);

	return interval;
}

// Print CPU stats
#if defined(CPU_STATS)
uint32_t statsCallback(Uint32 interval, void* param)
{
	EmulatorContext* ctx = (EmulatorContext*)param;
	char stats[513];

	CRV32 *cpu0 = ctx->emulator->m_cpu[0];
	snprintf(stats, 512, "CPU0 stats (over last second)\n");
	snprintf(stats, 512, "%sI$  read hits / misses: %d / %d\n", stats, cpu0->m_icache.m_hits, cpu0->m_icache.m_misses);
	snprintf(stats, 512, "%sEX retired instructions: %lld\n", stats, cpu0->m_retired);
	snprintf(stats, 512, "%sEX conditional branches taken / not taken: %d / %d\n", stats, cpu0->m_btaken, cpu0->m_bntaken);
	snprintf(stats, 512, "%sEX unconditional branches taken: %d\n", stats, cpu0->m_ucbtaken);
	snprintf(stats, 512, "%sD$  read hits / misses: %d / %d\n", stats, cpu0->m_dcache.m_readhits, cpu0->m_dcache.m_readmisses);
	snprintf(stats, 512, "%s   write hits / misses: %d / %d\n", stats, cpu0->m_dcache.m_writehits, cpu0->m_dcache.m_writemisses);

	if (s_statSurface)
		SDL_FreeSurface(s_statSurface);
	s_statSurface = TTF_RenderText_Blended_Wrapped(s_debugfont, stats, {255,0,255}, WIDTH);

	cpu0->m_icache.m_hits = 0;
	cpu0->m_icache.m_misses = 0;
	cpu0->m_dcache.m_readhits = 0;
	cpu0->m_dcache.m_readmisses = 0;
	cpu0->m_dcache.m_writehits = 0;
	cpu0->m_dcache.m_writemisses = 0;
	cpu0->m_retired = 0;
	cpu0->m_btaken = 0;
	cpu0->m_bntaken = 0;

	return interval;
}
#endif

void gdbstubprocess(socket_t gdbsocket, CEmulator* emulator, char* buffer, int n)
{
	// Check the first character of the buffer to determine the packet type
	switch (buffer[0])
	{
		case '+':
			// ACK packet
			//gdbresponseack(gdbsocket);
			break;
		case '-':
			// NACK packet
			break;
		case 0x03:
			// Interrupt packet, do not shift buffer
			gdbprocesscommand(gdbsocket, emulator, buffer);
			break;
		case '$':
			// Command packet
			buffer++;
			gdbprocesscommand(gdbsocket, emulator, buffer);
			break;
		default:
			// Unknown packet
			break;
	}
}

int gdbstubthread(void* data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);

	EmulatorContext* ctx = (EmulatorContext*)data;

#ifdef CAT_WINDOWS
	WSADATA wsaData;
	int wsaerr;
	WORD wVersionRequested = MAKEWORD(2, 2);
	wsaerr = WSAStartup(wVersionRequested, &wsaData);
#endif

	// Create a socket
	socket_t sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		fprintf(stderr, "Error opening socket\n");
		return -1;
	}

#ifdef CAT_WINDOWS
	//int timeout = 1000; // ms
	//retval = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	u_long nonBlocking = 1;
	if (ioctlsocket(sockfd, FIONBIO, &nonBlocking) == SOCKET_ERROR)
		fprintf(stderr, "ERROR: Socket can't be set to nonblocking mode\n");
#endif

	// Bind the socket to the port
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(1234);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Error binding socket\n");
		return -1;
	}

	// Listen for incoming connections
	listen(sockfd, 5);

	fprintf(stderr, "GDB stub on //localhost:1234\n");

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		fprintf(stderr, "Error accepting connection\n");
		return -1;
	}

	// Read from the socket
	char buffer[4096];
	int n;
	do
	{
		// TODO: We need nonblocking mode for this
		/*if (ctx->emulator->m_cpu[0]->m_breakpointHit)
			gdbsendstopreason(newsockfd, 0, ctx->emulator);
		if (ctx->emulator->m_cpu[1]->m_breakpointHit)
			gdbsendstopreason(newsockfd, 1, ctx->emulator);*/

		n = read(newsockfd, buffer, 4096);
		if (n < 0)
			fprintf(stderr, "Error reading from socket\n");
		else
		{
			buffer[n] = 0;
			fprintf(stderr, "> %s\n", buffer);
			// Respond to gdb command packets here
			gdbstubprocess(newsockfd, ctx->emulator, buffer, n);
		}
	} while (n > 0);
#else
	fd_set readfds;
	bool accepted = false;
	while(!accepted && s_alive)
	{
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

		int result = select(0, &readfds, NULL, NULL, NULL);
		if (result > 0)
		{
			if (FD_ISSET(sockfd, &readfds))
			{
				struct sockaddr_in clientAddr;
				int addrlen = sizeof(clientAddr);
				newsockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &addrlen);
				if (newsockfd != INVALID_SOCKET)
				{
					// Handle client connection (may need error checking for WSAEWOULDBLOCK)
					//fprintf(stderr, "Accepted connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					ioctlsocket(newsockfd, FIONBIO, &nonBlocking);
					accepted = true;
				}
				else
				{
					// Check for WSAEWOULDBLOCK error (non-blocking)
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						// No connection available, try again later
					}
					else
					{
						// Other error handling
					}
				}
			}
		}
	}

	char buffer[4096];
	int n;
	//bool done = false;
	do
	{
		for (uint32_t cpu = 0; cpu < 2; ++cpu)
		{
			for (auto& breakpoint : ctx->emulator->m_cpu[cpu]->m_breakpoints)
			{
				if (breakpoint.isHit && !breakpoint.isCommunicated)
				{
					breakpoint.isCommunicated = 1;
					gdbsendstopreason(newsockfd, cpu, breakpoint.address, ctx->emulator);

					// If the breakpoint is volatile, remove it
					if (breakpoint.isVolatile)
						ctx->emulator->RemoveBreakpoint(cpu, breakpoint.address);
				}
			}
		}

		n = recv(newsockfd, buffer, 4096, 0);
		if (n > 0)
		{
			buffer[n] = 0;
#ifdef GDB_COMM_DEBUG
			fprintf(stderr, "> %s\n", buffer);
#endif
			// Respond to gdb command packets here
			gdbstubprocess(newsockfd, ctx->emulator, buffer, n);
		}
		else
			buffer[0] = 0;

		/*if (n < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			fprintf(stderr, "Error reading from socket\n");
			done = true;
		}*/
	} while (s_alive);

#endif

#ifdef CAT_WINDOWS
#else
	close(newsockfd);
	close(sockfd);
#endif

	return 0;
}

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	EmulatorContext ectx;
	ectx.emulator = new CEmulator;
	bool success;

	fprintf(stderr, "tinysys emulator v1.1E\n");

	const uint32_t resetvector = 0x0FFE0000;
	char bootRom[256] = "rom.bin";
	if (argc>1)
		strncpy(bootRom, argv[1], strlen(argv[1]));

	success = ectx.emulator->Reset(bootRom, resetvector);

	if (!success)
	{
		fprintf(stderr, "Failed to load ROM\n");
		return -1;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}
	ectx.window = SDL_CreateWindow("tinysys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT+8, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (TTF_Init() != 0)
	{
		fprintf(stderr, "Error initializing SDL2_ttf: %s\n", TTF_GetError());
		return -1;
	}
	s_debugfont = TTF_OpenFont("DejaVuSansMono.ttf", 12);
	if (!s_debugfont)
	{
		fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
		return -1;
	}

	if (!SDL_GetWindowSurface(ectx.window))
	{
		fprintf(stderr, "Could not create window surface\n");
		return -1;
	}

	// Compositor surface
	ectx.compositesurface = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT+8, 32, SDL_PIXELFORMAT_ARGB8888);

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

	// ONE_MILLISECOND_IN_TICKS to convert from SDL ticks to device units
	const uint64_t ONE_MS_IN_TICKS = 10000;
	uint64_t startTick = SDL_GetTicks64() * ONE_MS_IN_TICKS;
	s_wallclock = SDL_GetTicks64() * ONE_MS_IN_TICKS - startTick;

#if defined(GALLIFREY)
	// Two HARTs
	SDL_Thread* emulatorthreadcpu0ID = SDL_CreateThread(emulatorthreadcpu0, "cpu0", &ectx);
	SDL_Thread* emulatorthreadcpu1ID = SDL_CreateThread(emulatorthreadcpu1, "cpu1", &ectx);
#else
	SDL_Thread* emulatorthreadID = SDL_CreateThread(emulatorthread, "cpu0&1", &ectx);
#endif
	SDL_Thread* audiothreadID = SDL_CreateThread(audiothread, "audio", ectx.emulator);
#if defined(CAT_LINUX)
	SDL_TimerID videoTimer = 0;
#else
	SDL_TimerID videoTimer = SDL_AddTimer(16, videoCallback, &ectx); // 60fps
#endif
#if defined(CPU_STATS)
	SDL_TimerID statsTimer = SDL_AddTimer(1000, statsCallback, &ectx);
#endif
	SDL_Thread* gdbStubThread = SDL_CreateThread(gdbstubthread, "gdbstub", &ectx);

	char bootString[256];
	snprintf(bootString, 255, "%s : %s", emulatorVersionString, bootRom);

	s_textSurface = TTF_RenderText_Blended_Wrapped(s_debugfont, bootString, {255,255,255}, WIDTH);

	fprintf(stderr, "Use the ~ key to reset the emulated CPUs in case of hangs during development\n");

	const uint8_t *keystates = SDL_GetKeyboardState(nullptr);
	uint8_t *old_keystates = new uint8_t[SDL_NUM_SCANCODES];
	memset(old_keystates, 0, SDL_NUM_SCANCODES);

	Axis6 prev_input;
	prev_input.leftx = -11111.f;
	prev_input.lefty = -11111.f;
	prev_input.rightx = -11111.f;
	prev_input.righty = -11111.f;
	prev_input.lefttrigger = -11111.f;
	prev_input.righttrigger = -11111.f;
	uint32_t prev_buttons = 0xFFFFFFFF;

	SDL_Event ev;
	do
	{
		if (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				s_alive = false;
			else if (ev.type == SDL_KEYUP)
			{
				if (ev.key.keysym.scancode == SDL_SCANCODE_GRAVE) // ESP32-C6 does something similar to pass the CPUs a reset signal via CSRs over a dedicated wire
				{
					CCSRMem* csr0 = ectx.emulator->m_bus->GetCSR(0);
					CCSRMem* csr1 = ectx.emulator->m_bus->GetCSR(1);
					CCSRMem* csr2 = ectx.emulator->m_bus->GetCSR(2);
					csr0->RequestReset();
					csr1->RequestReset();
					csr2->RequestReset();
				}
				else if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'c')
					ectx.emulator->QueueByte(3);
				/*else if (ev.key.keysym.sym != SDLK_LCTRL && ev.key.keysym.sym != SDLK_LSHIFT && ev.key.keysym.sym != SDLK_RSHIFT)
				{
					if (ev.key.keysym.mod & KMOD_SHIFT)
						ectx.emulator->QueueByte(0x5F & ev.key.keysym.sym);
					else
						ectx.emulator->QueueByte(ev.key.keysym.sym);
				}*/
			}
			else if (ev.type == SDL_CONTROLLERDEVICEADDED)
			{
				ControllerAdd(ectx);
			}
			else if (ev.type == SDL_CONTROLLERDEVICEREMOVED)
			{
				ControllerRemove(ectx);
			}

			// Read joystick events
			if (ectx.gamecontroller)
			{
				// Buttons
				uint32_t buttons = 0x00000000;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_A) ? 0x00000001 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_B) ? 0x00000002 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_X) ? 0x00000004 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_Y) ? 0x00000008 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_BACK) ? 0x00000010 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_GUIDE) ? 0x00000020 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_START) ? 0x00000040 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_LEFTSTICK) ? 0x00000080 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) ? 0x00000100 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) ? 0x00000200 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) ? 0x00000400 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_UP) ? 0x00000800 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ? 0x00001000 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ? 0x00002000 : 0;
				buttons |= SDL_GameControllerGetButton(ectx.gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? 0x00004000 : 0;
				if (buttons != prev_buttons)
				{
					prev_buttons = buttons;

					uint8_t outdata[4];
					outdata[0] = '@';				// joystick button packet marker
					outdata[1] = buttons&0xFF;		// lower byte of modifiers
					outdata[2] = (buttons>>8)&0xFF;	// upper byte of modifiers
					ectx.emulator->QueueByte(outdata[0]);
					ectx.emulator->QueueByte(outdata[1]);
					ectx.emulator->QueueByte(outdata[2]);
				}

				// Axes
				Axis6 input;
				input.leftx = SDL_GameControllerGetAxis(ectx.gamecontroller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
				input.lefty = SDL_GameControllerGetAxis(ectx.gamecontroller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
				input.rightx = SDL_GameControllerGetAxis(ectx.gamecontroller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
				input.righty = SDL_GameControllerGetAxis(ectx.gamecontroller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
				input.lefttrigger = SDL_GameControllerGetAxis(ectx.gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
				input.righttrigger = SDL_GameControllerGetAxis(ectx.gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
				ClampAnalogInput(input, 0.1f, 0.9f);
				if (prev_input.leftx != input.leftx || prev_input.lefty != input.lefty || prev_input.rightx != input.rightx || prev_input.righty != input.righty || prev_input.lefttrigger != input.lefttrigger || prev_input.righttrigger != input.righttrigger)
				{
					prev_input = input;

					uint16_t lx = (uint16_t)((input.leftx * 32767.0f) + 32768);
					uint16_t ly = (uint16_t)((input.lefty * 32767.0f) + 32768);
					uint16_t rx = (uint16_t)((input.rightx * 32767.0f) + 32768);
					uint16_t ry = (uint16_t)((input.righty * 32767.0f) + 32768);
					uint8_t lt = (uint8_t)(input.lefttrigger * 255.0f);
					uint8_t rt = (uint8_t)(input.righttrigger * 255.0f);

					uint8_t outdata[16];
					outdata[0] = '%';				// joystick axis packet marker
					outdata[1] = lx&0xFF;			// lower byte of left x
					outdata[2] = (lx>>8)&0xFF;		// upper byte of left x
					outdata[3] = ly&0xFF;			// lower byte of left y
					outdata[4] = (ly>>8)&0xFF;		// upper byte of left y
					outdata[5] = rx&0xFF;			// lower byte of right x
					outdata[6] = (rx>>8)&0xFF;		// upper byte of right x
					outdata[7] = ry&0xFF;			// lower byte of right y	
					outdata[8] = (ry>>8)&0xFF;		// upper byte of right y
					outdata[9] = lt;				// left trigger
					outdata[10] = rt;				// right trigger
					ectx.emulator->QueueByte(outdata[0]);
					ectx.emulator->QueueByte(outdata[1]);
					ectx.emulator->QueueByte(outdata[2]);
					ectx.emulator->QueueByte(outdata[3]);
					ectx.emulator->QueueByte(outdata[4]);
					ectx.emulator->QueueByte(outdata[5]);
					ectx.emulator->QueueByte(outdata[6]);
					ectx.emulator->QueueByte(outdata[7]);
					ectx.emulator->QueueByte(outdata[8]);
					ectx.emulator->QueueByte(outdata[9]);
					ectx.emulator->QueueByte(outdata[10]);
				}
			}
			// Read key states
			SDL_Keymod modifiers = SDL_GetModState();
			if (memcmp(old_keystates, keystates, SDL_NUM_SCANCODES))
			{
				for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
				{
					if (keystates[i] != old_keystates[i])
					{
						uint8_t outdata[8];
						outdata[0] = '^';					// scancode packet marker
						outdata[1] = i;						// scancode
						outdata[2] = keystates[i];			// state
						outdata[3] = modifiers&0xFF;		// lower byte of modifiers
						outdata[4] = (modifiers>>8)&0xFF;	// upper byte of modifiers
						ectx.emulator->QueueByte(outdata[0]);
						ectx.emulator->QueueByte(outdata[1]);
						ectx.emulator->QueueByte(outdata[2]);
						ectx.emulator->QueueByte(outdata[3]);
						ectx.emulator->QueueByte(outdata[4]);
					}
				}
			}
			// Copy the new keystates to the old keystates
			memcpy(old_keystates, keystates, SDL_NUM_SCANCODES);
		}

#if defined(CAT_LINUX)
		videoCallback(16, &ectx);
#endif
		s_wallclock = SDL_GetTicks64() * ONE_MS_IN_TICKS - startTick;
	} while(s_alive);

#if defined(CPU_STATS)
	SDL_FreeSurface(s_statSurface);
#endif

	SDL_FreeSurface(s_textSurface);
	TTF_CloseFont(s_debugfont);
	TTF_Quit();
	SDL_RemoveTimer(videoTimer);
	//SDL_KillThread(gdbStubThread); // We'll hang in accept otherwise
#if defined(GALLIFREY)
	SDL_WaitThread(emulatorthreadcpu0ID, nullptr);
	SDL_WaitThread(emulatorthreadcpu1ID, nullptr);
#else
	SDL_WaitThread(emulatorthreadID, nullptr);
#endif
	SDL_WaitThread(audiothreadID, nullptr);
	SDL_ClearQueuedAudio(ectx.emulator->m_audioDevice);
	SDL_FreeSurface(ectx.compositesurface);
	SDL_DestroyWindow(ectx.window);
	SDL_CloseAudioDevice(ectx.emulator->m_audioDevice);
	SDL_Quit();

	delete ectx.emulator;

	return 0;
}
