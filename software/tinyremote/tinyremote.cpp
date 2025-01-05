#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <chrono>
#include <thread>
#include <filesystem>

#include "platform.h"
#include "common.h"
#include "serial.h"
#include "video.h"
#include "lz4.h"

static bool s_alive = true;
static SDL_Window* s_window;
static SDL_Surface* s_surface;
static uint8_t *s_videodata;

uint32_t videoCallback(uint32_t interval, void* param)
{
	VideoCapture* video_capture = (VideoCapture*)param;

	const int videowidth = 640;
	const int videoheight = 480;

	bool haveFrame = video_capture->CaptureFrame(s_videodata);
	if (haveFrame)
	{
		if (SDL_MUSTLOCK(s_surface))
			SDL_LockSurface(s_surface);

		uint32_t* pixels = (uint32_t*)s_surface->pixels;
		uint32_t* vid = (uint32_t*)s_videodata;
		for (int y = 0; y < videoheight; ++y)
		{
			for (int x = 0; x < videowidth; ++x)
			{
				pixels[(y * s_surface->w + x)] = vid[y*videowidth+x];
			}
		}

		if (SDL_MUSTLOCK(s_surface))
			SDL_UnlockSurface(s_surface);

		SDL_UpdateWindowSurface(s_window);
	}

	return interval;
}

bool WACK(CSerialPort *_serial, const uint8_t waitfor, uint8_t& received)
{
	int ack = 0;
	uint8_t dummy;
	uint8_t bytes = 0;
	uint32_t timeout = 0;
	do
	{
		if (_serial->Receive(&dummy, 1))
		{
			received = dummy;
			++bytes;
			if (dummy == waitfor)
				ack = 1; // Valid ACK
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		++timeout;
	} while (!bytes);

	return (timeout>16384) ? false : (ack ? true : false);
}

unsigned int getfilelength(const fpos_t &endpos)
{
#if defined(CAT_LINUX)
	return (unsigned int)endpos.__pos;
#elif defined(CAT_MACOS)
	return (unsigned int)endpos;
#else // CAT_WINDOWS
	return (unsigned int)endpos;
#endif
}

void ConsumeInitialTraffic(CSerialPort* _serial)
{
	// When we first open the port, ESP32 will respond with a "ESP-ROM:esp32xxxxxx" and tinysys version message
	// or sometimes with nothing. We need to consume this traffic before we can send any commands
	uint8_t startup = 0;
	while (_serial->Receive(&startup, 1))
	{
		// This usually reads something similar to "ESP-ROM:esp32s3-*"
		// It is likely that some other unfinished traffic will show up here
		printf("%c", startup);
	}
}

void SendFile(char *_filename, CSerialPort* _serial)
{
	char tmpstring[129];

	FILE *fp;
	fp = fopen(_filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "ERROR: can't open ELF file %s\n", _filename);
		return;
	}

	char cleanfilename[129];
	{
		using namespace std::filesystem;
		path p = absolute(_filename);
		strcpy(cleanfilename, p.filename().string().c_str());
	}

	unsigned int filebytesize = 0;
	fpos_t pos, endpos;
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fsetpos(fp, &pos);
	filebytesize = getfilelength(endpos);

	uint8_t *filedata = new uint8_t[filebytesize + 64];
	fread(filedata, 1, filebytesize, fp);
	fclose(fp);

	// Send the file bytes in chunks
	uint32_t packetSize = 1024;
	int worstSize = LZ4_compressBound(filebytesize);

	// Pack the data before sending it across
	char* encoded = new char[worstSize];
	uint32_t encodedSize = LZ4_compress_default((const char*)filedata, encoded, filebytesize, worstSize);
	fprintf(stderr, "Compression ratio = %.2f%% (%d->%d bytes)\n", 100.f*float(encodedSize)/float(filebytesize), filebytesize, encodedSize);

	// Now we can work out how many packets we need to send
	uint32_t numPackets = encodedSize / packetSize;
	uint32_t leftoverBytes = encodedSize % packetSize;

	uint8_t received;

	ConsumeInitialTraffic(_serial);

	// Start the receiver app on the other end
	snprintf(tmpstring, 128, "recv");
	_serial->Send((uint8_t*)tmpstring, 4);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	snprintf(tmpstring, 128, "\n");
	_serial->Send((uint8_t*)tmpstring, 1);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "Transfer initiation error: '%c'\n", received);
		delete [] filedata;
		return;
	}

	// Send encoded length
	uint32_t encodedLen = encodedSize;
	_serial->Send(&encodedLen, 4);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "Encoded buffer length error: '%c'\n", received);
		delete [] filedata;
		return;
	}

	// Send actual length
	uint32_t decodedLen = filebytesize;
	_serial->Send(&decodedLen, 4);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "Decoded buffer length error: '%c'\n", received);
		delete [] filedata;
		return;
	}

	// Send name length
	uint32_t nameLen = strlen(cleanfilename);
	_serial->Send(&nameLen, 4);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "File name length error: '%c'\n", received);
		delete [] filedata;
		return;
	}

	// Send name
	_serial->Send(cleanfilename, nameLen);
	if (!WACK(_serial, '+', received))
	{
		fprintf(stderr, "File name error: '%c'\n", received);
		delete [] filedata;
		return;
	}

	uint32_t i = 0;
	uint32_t packetOffset = 0;
	char progress[65];
	for (i=0; i<64; ++i)
		progress[i] = ' ';//176;
	progress[64] = 0;
	fprintf(stderr, "Uploading '%s' (packet size: %dx%d+%d bytes, packer buffer: %d bytes)\n", cleanfilename, numPackets, packetSize, leftoverBytes, worstSize);
	for (i=0; i<numPackets; ++i)
	{
		const char* source = (const char*)(encoded + packetOffset);

		int idx = (i*64)/numPackets;
		for (int j=0; j<=idx; ++j) // Progress bar
			progress[j] = '=';//219;
		fprintf(stderr, "\r [%s] %.2f%%\r", progress, (i*100)/float(numPackets));

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet size error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			return;
		}

		_serial->Send(&packetSize, 4);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			return;
		}

		_serial->Send((void*)source, packetSize);

		packetOffset += packetSize;
	}

	if (leftoverBytes)
	{
		const char* source = (const char*)(encoded + packetOffset);

		for (int j=0; j<64; ++j) // Progress bar
			progress[j] = '=';//219;
		fprintf(stderr, "\r [%s] %.2f%%\r", progress, 100.0f);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet size error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			return;
		}

		_serial->Send(&leftoverBytes, 4);

		if (!WACK(_serial, '+', received)) // Wait for a 'go' signal
		{
			fprintf(stderr, "Packet error at %d/%d (%d): '%c'\n", i, numPackets, received, packetSize);
			delete [] filedata;
			return;
		}

		_serial->Send((void*)source, packetSize);

		packetOffset += leftoverBytes;
	}

	fprintf(stderr, "\r\n");

	delete[] encoded;

	fprintf(stderr, "%d bytes uploaded\n", packetOffset);

	delete [] filedata;
}

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	const char* cname = GetCommDeviceName();
	const char* vname = GetVideoDeviceName();

	fprintf(stderr, "Usage: tinyremote commdevicename capturedevicename\ndefault comm device:%s default capture device:%s\nCtrl+C or PAUSE: quit current remote process\n", cname, vname);

	if (argc > 1)
		SetCommDeviceName(argv[1]);
	if (argc > 2)
		SetVideoDeviceName(argv[2]);

	int width = 640;
	int height = 480;

	CSerialPort serial;
	serial.Open();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}

	s_window = SDL_CreateWindow("tinysys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	s_surface = SDL_GetWindowSurface(s_window);

	s_videodata = new uint8_t[width*height*4];
	VideoCapture video_capture;
	video_capture.Initialize(width, height);

	SDL_TimerID videoTimer = SDL_AddTimer(16, videoCallback, &video_capture); // 60fps

	const uint8_t *keystates = SDL_GetKeyboardState(nullptr);
	uint8_t *old_keystates = new uint8_t[SDL_NUM_SCANCODES];
	memset(old_keystates, 0, SDL_NUM_SCANCODES);

	SDL_Event ev;
	do
	{
		if (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				s_alive = false;
			if (ev.type == SDL_DROPFILE)
				SendFile(ev.drop.file, &serial);
		}

		// Detect key changes
		SDL_Keymod modifiers = SDL_GetModState();
		if (memcmp(old_keystates, keystates, SDL_NUM_SCANCODES))
		{
			for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
			{
				if (keystates[i] != old_keystates[i])
				{
					// DEBUG: fprintf(stderr, "key %d: %d mod: %d\n", i, keystates[i], modifiers);

					uint8_t outdata[4];
					outdata[0] = '^';					// scancode packet marker
					outdata[1] = i;						// scancode
					outdata[2] = keystates[i];			// state
					outdata[3] = modifiers&0xFF;		// lower byte of modifiers
					outdata[4] = (modifiers>>8)&0xFF;	// upper byte of modifiers

					// IMPORTANT: We MUST capture ~ key since it's essential for the ESP32 to reboot the device CPUs when stuck
					// NOTE: You must hold down the ~ key for at least 250ms for the reboot to occur
					if (i==53 && keystates[i] == 1 && (modifiers & KMOD_SHIFT))
					{
						fprintf(stderr, "rebooting device\n");
						serial.Send((uint8_t*)"~", 1);
					}
					else if (i == 0x06 && keystates[i] == 1 && (modifiers & KMOD_CTRL))
					{
						fprintf(stderr, "quitting remote process\n");
						serial.Send((uint8_t*)"\03", 1);
					}
					else
						serial.Send(outdata, 5);
				}
			}

			// Copy the new keystates to the old keystates
			memcpy(old_keystates, keystates, SDL_NUM_SCANCODES);
		}
	} while(s_alive);

	serial.Close();
	fprintf(stderr, "remote connection terminated\n");

	SDL_RemoveTimer(videoTimer);
	video_capture.Terminate();

	SDL_FreeSurface(s_surface);
	SDL_DestroyWindow(s_window);
	SDL_Quit();

	return 0;
}
