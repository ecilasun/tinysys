#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "platform.h"
#include "common.h"
#include "serial.h"
#include "video.h"

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
