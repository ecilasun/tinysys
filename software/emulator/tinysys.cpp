#include <stdio.h>
#include <string.h>

#ifdef CAT_WINDOWS
#include <winsock2.h>
#define socket_t SOCKET
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define socket_t int
#endif

#include "emulator.h"
#include "SDL.h"
#include "SDL_ttf.h"

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
	CEmulator* emulator;
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Surface* compositesurface;
};

static bool s_alive = true;
static uint64_t s_wallclock = 0;

int emulatorthread(void* data)
{
	EmulatorContext* ctx = (EmulatorContext*)data;
	thread_local int scanline = 0;
	do
	{
		CEmulator *emulator = ctx->emulator;
		emulator->Step(s_wallclock);
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

	if (SDL_MUSTLOCK(ctx->compositesurface))
		SDL_LockSurface(ctx->compositesurface);

	uint32_t* pixels = (uint32_t*)ctx->compositesurface->pixels;

	for (int scanline = 0; scanline < 525; ++scanline)
		ctx->emulator->UpdateVideoLink(pixels, scanline, ctx->compositesurface->pitch);

	uint32_t W = ctx->compositesurface->w;
	uint32_t H = ctx->compositesurface->h-8;
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
	SDL_BlitSurface(ctx->compositesurface, nullptr, ctx->surface, nullptr);
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

uint8_t gdbchecksum(const char *data)
{
	uint8_t sum = 0;
	while (*data)
	{
		sum += (uint8_t)*data++;
	}
	return sum;
}

void gdbresponsepacket(socket_t gdbsocket, const char* buffer)
{
	char response[1024];
	snprintf(response, 1024, "+$%s#%02x", buffer, gdbchecksum(buffer));
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbresponseack(socket_t gdbsocket)
{
	char response[4];
	snprintf(response, 4, "+");
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbresponsenack(socket_t gdbsocket)
{
	char response[4];
	snprintf(response, 4, "-");
#ifdef CAT_WINDOWS
	send(gdbsocket, response, (int)strlen(response), 0);
#else
	write(gdbsocket, response, strlen(response));
#endif
}

void gdbreadthreads(socket_t gdbsocket, CEmulator* emulator, const char* buffer)
{
	char response[1024];
	snprintf(response, 1024, "l<?xml version=\"1.0\"?>\n<threads>\n");
	snprintf(response, 1024, "%s\t<thread id=\"1\" core=\"0\" name=\"emu\">emulated task</thread>\n", response);

	/*for (int cpu = 0; cpu < 2; ++cpu)
	{
		CRV32 *core = emulator->m_cpu[cpu];
		struct STaskContext* ctx = _task_get_context(cpu);

		// Skip the OS threads
		for (int j = cpu == 0 ? 2 : 1; j < ctx->numTasks; ++j)
		{
			struct STask* task = &ctx->tasks[j];
			// Apparently GDB expects thread IDs across CPUs to be unique
			snprintf(response, 1024, "%s\t<thread id=\"%d\" core=\"%d\" name=\"%s\" handle=\"%x\">%s [CPU%d]</thread>\n", response, j + 1, cpu, task->name, cpu * TASK_MAX + j, task->name, cpu);
		}
	}*/

	strcat(response, "</threads>\n");
	gdbresponsepacket(gdbsocket, response);
}

void gdbprocessquery(socket_t gdbsocket, CEmulator* emulator, const char* buffer)
{
	// Check the query type
	if(strstr(buffer, "qAttached") == buffer)
	{
		// Attached query
		gdbresponsepacket(gdbsocket, "1");
	}
	else if (strstr(buffer, "qXfer:threads:read:") == buffer)
	{
		// Read threads query
		gdbreadthreads(gdbsocket, emulator, buffer);
	}
	else if (strstr(buffer, "qTStatus") == buffer)
	{
		// Status query
		gdbresponsepacket(gdbsocket, "");
	}
	else if (strstr(buffer, "qSupported") == buffer)
	{
		// Supported query (packetsize is hex therefore 0x1000==4096 bytes)
		gdbresponsepacket(gdbsocket, "PacketSize=1000;qXfer:threads:read+;swbreak+;");
	}
	else if (strstr(buffer, "qSymbol") == buffer)
	{
		// Symbol query
		printf("Symbol\n");
	}
	else if (strstr(buffer, "qfThreadInfo") == buffer)
	{
		// Thread info query
		printf("Thread info\n");
	}
	else if (strstr(buffer, "qC") == buffer)
	{
		// Current thread query
		gdbresponsepacket(gdbsocket, "QC 1");
	}
	else
	{
		// Unknown query
		printf("Unknown query: %s\n", buffer);
	}
}

void gdbvcont(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'vCont'
	buffer += 5;

	// Parse commands
	char* command = strtok(buffer, ";");
	while (command != NULL)
	{
		if (strstr(command, "?") == command)
		{
			// vCont query
			gdbresponsepacket(gdbsocket, "vCont;c;C;s;S;");
		}
		else if (strstr(command, "c") == command)
		{
			// Continue
			gdbresponsepacket(gdbsocket, "S05");
		}
		else if (strstr(command, "s") == command)
		{
			// Step
			gdbresponsepacket(gdbsocket, "S05");
		}
		else
		{
			// Unknown command
			printf("Unknown vCont: %s\n", command);
		}
		command = strtok(NULL, ";");
	}
}

void gdbreadregisters(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	char response[1024];

	// x0
	snprintf(response, 1024, "00000000");

	// x1-x31
	CRV32* core = emulator->m_cpu[0];
	for (int i = 1; i < 32; ++i)
	{
		uint32_t reg = core->m_GPR[i];
		snprintf(response, 1024, "%s%02X%02X%02X%02X",
			response,
			(reg >> 0) & 0xFF,
			(reg >> 8) & 0xFF,
			(reg >> 16) & 0xFF,
			(reg >> 24) & 0xFF);
	}

	// PC
	{
		uint32_t reg = core->m_PC;
		snprintf(response, 1024, "%s%02X%02X%02X%02X",
			response,
			(reg >> 0) & 0xFF,
			(reg >> 8) & 0xFF,
			(reg >> 16) & 0xFF,
			(reg >> 24) & 0xFF);
	}

	printf("R:%s\n", response);
	gdbresponsepacket(gdbsocket, response);
}

void gdbbinarypacket(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'X'
	buffer++;

	// Parse the address and length
	uint32_t addrs;
	uint32_t len;
	uint32_t readoffset = sscanf(buffer, "%x,%x", &addrs, &len);

	// This is a support check if len is 0
	if (len == 0)
	{
		// We support binary data
		gdbresponsepacket(gdbsocket, "OK");
	}
	else
	{
		// Skip the address and length
		buffer = strchr(buffer, ':');
		buffer++;

		// Decode the encoded binary data, paying attention to escape sequences and repeat counts
		uint8_t* data = new uint8_t[len];
		uint32_t i = 0;
		uint8_t lastchar = 0;
		while (i < len)
		{
			if (*buffer == '}') // Escape sequence
			{
				buffer++;
				uint8_t original = *buffer ^ 0x20;
				data[i++] = original;
			}
			else if (*buffer == '*') // Repeat last character as a sequence
			{
				buffer++;
				uint8_t count = *buffer - 29;
				for (uint8_t j = 0; j < count; ++j)
					data[i++] = lastchar;
			}
			else // Normal character
				data[i++] = *buffer;
			lastchar = *buffer;
			buffer++;
		}

		// Debug print the binary data
		printf("0x%X : ", addrs);
		for (uint32_t i = 0; i < len; ++i)
			printf("%02X ", data[i]);
		
		delete[] data;
		
		// Invalidate data caches of both CPUs and write incoming data to memory
		//emulator->m_cpu[0]->m_dcache.Invalidate();
		//emulator->m_cpu[1]->m_dcache.Invalidate();
		//emulator->m_bus->WriteMemory(addrs, data, len);

		// Respond with an ACK on successful memory write
		gdbresponsepacket(gdbsocket, "OK");
	}
}

void gdbsetreg(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Skip 'P'
	buffer++;

	uint32_t reg, val;
	sscanf(buffer, "%x=%x", &reg, &val);

	// Reverse the byte order
	val = (val >> 24) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | (val << 24);

	// TODO: when setting PC, our entire task system breaks down
	// Therefore we need to add a dummy task to the task list and set its PC to the supplied value

	if (reg == 32) // PC is a special case and is always at lastgpr+1
		;//emulator->m_cpu[0]->m_PC = val;
	else
		emulator->m_cpu[0]->m_GPR[reg] = val;

	printf("reg X%d set to %08X\n", reg, val);

	gdbresponsepacket(gdbsocket, "OK");
}

void gdbprocesscommand(socket_t gdbsocket, CEmulator* emulator, char* buffer)
{
	// Check the command type
	switch (buffer[0])
	{
		case '?':
			gdbresponsepacket(gdbsocket, "S00");
			break;
		case 'X':
			// Read binary data
			gdbbinarypacket(gdbsocket, emulator, buffer);
			break;
		case 'P':
			// Write single register
			gdbsetreg(gdbsocket, emulator, buffer);
			break;
		case 'D':
			// Disconnect request
			gdbresponsepacket(gdbsocket, "OK");
			// TODO: remove all breakpoints and resume task
			break;
		case 'H':
			// Set thread - Hc or Hg
			if (buffer[1] == 'c')
			{
				//emulator->m_cpu[???]->SetCurrent('c');
				gdbresponsepacket(gdbsocket, "OK");
			}
			else if (buffer[1] == 'g')
			{
				//emulator->m_cpu[???]->SetCurrent('g');
				gdbresponsepacket(gdbsocket, "OK");
			}
			break;
		case 'g':
			gdbreadregisters(gdbsocket, emulator, buffer);
			break;
		case 'G':
			// Read registers
			break;
		case 'm':
			// Read memory
			break;
		case 'M':
			// Write memory
			break;
		case 'c':
			// Continue
			break;
		case 's':
			// Step
			break;
		case 'v':
			// vCont
			if (strstr(buffer, "vCont") == buffer)
				gdbvcont(gdbsocket, emulator, buffer);
			else if(strstr(buffer, "vMustReplyEmpty") == buffer)
				gdbresponsepacket(gdbsocket, "");
			else
				printf("Unknown v command: %s\n", buffer);
			break;
		case 'q':
			// Query
			gdbprocessquery(gdbsocket, emulator, buffer);
			break;
		default:
			// Unknown command
			break;
	}
}

void gdbstubprocess(socket_t gdbsocket, CEmulator* emulator, char* buffer, int n)
{
	// Check the first character of the buffer to determine the packet type
	switch (buffer[0])
	{
		case '+':
			// ACK packet
			break;
		case '-':
			// NACK packet
			break;
		case '$':
			// Command packet
			buffer++;
			gdbprocesscommand(gdbsocket, emulator, buffer);
			break;
		case 3:
			// Ctrl-C packet
			break;
		default:
			// Unknown packet
			break;
	}
}

int gdbstubthread(void* data)
{
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

	// Accept a connection
#ifdef CAT_WINDOWS
	socket_t newsockfd = accept(sockfd, nullptr, nullptr);
#else
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	socket_t newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
#endif
	if (newsockfd < 0)
	{
		fprintf(stderr, "Error accepting connection\n");
		return -1;
	}

	fprintf(stderr, "GDB stub listening on //localhost:1234\n");

	// Read from the socket
	char buffer[4096];
	int n;
	do
	{
#ifdef CAT_WINDOWS
		n = recv(newsockfd, buffer, 4096, 0);
#else
		n = read(newsockfd, buffer, 4096);
#endif
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

	fprintf(stderr, "tinysys emulator v1.0D\n");

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
	ectx.window = SDL_CreateWindow("tinysys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT+8, SDL_WINDOW_SHOWN);

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

	ectx.surface = SDL_GetWindowSurface(ectx.window);
	if (!ectx.surface)
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

	SDL_Thread* emulatorthreadID = SDL_CreateThread(emulatorthread, "emulator", &ectx);
	SDL_Thread* audiothreadID = SDL_CreateThread(audiothread, "audio", ectx.emulator);
	SDL_TimerID videoTimer = SDL_AddTimer(16, videoCallback, &ectx); // 60fps
#if defined(CPU_STATS)
	SDL_TimerID statsTimer = SDL_AddTimer(1000, statsCallback, &ectx);
#endif
	SDL_Thread* gdbStubThread = SDL_CreateThread(gdbstubthread, "gdbstub", &ectx);

	char bootString[256];
	snprintf(bootString, 255, "%s : %s", emulatorVersionString, bootRom);

	s_textSurface = TTF_RenderText_Blended_Wrapped(s_debugfont, bootString, {255,255,255}, WIDTH);

	fprintf(stderr, "Use the ~ key to reset the emulated CPUs in case of hangs during development\n");

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
				else if (ev.key.keysym.sym != SDLK_LCTRL && ev.key.keysym.sym != SDLK_LSHIFT && ev.key.keysym.sym != SDLK_RSHIFT)
				{
					if (ev.key.keysym.mod & KMOD_SHIFT)
						ectx.emulator->QueueByte(0x5F & ev.key.keysym.sym);
					else
						ectx.emulator->QueueByte(ev.key.keysym.sym);
				}
			}
		}

		s_wallclock = SDL_GetTicks64() * ONE_MS_IN_TICKS - startTick;
	} while(s_alive);

#if defined(CPU_STATS)
	SDL_FreeSurface(s_statSurface);
#endif

	SDL_FreeSurface(s_textSurface);
	TTF_CloseFont(s_debugfont);
	TTF_Quit();
	SDL_RemoveTimer(videoTimer);
	SDL_WaitThread(emulatorthreadID, nullptr);
	SDL_WaitThread(audiothreadID, nullptr);
	SDL_ClearQueuedAudio(ectx.emulator->m_audioDevice);
	SDL_FreeSurface(ectx.compositesurface);
	SDL_FreeSurface(ectx.surface);
	SDL_DestroyWindow(ectx.window);
	SDL_CloseAudioDevice(ectx.emulator->m_audioDevice);
	SDL_Quit();

	return 0;
}
