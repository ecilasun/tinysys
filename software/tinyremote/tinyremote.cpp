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

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	const char* cname = GetCommDeviceName();
	const char* vname = GetVideoDeviceName();

	printf("Usage: tinyremote commdevicename capturedevicename\ndefault comm device:%s default capture device:%s\nCtrl+C or PAUSE: quit current remote process\n", cname, vname);

	if (argc > 1)
		SetCommDeviceName(argv[1]);
	if (argc > 2)
		SetVideoDeviceName(argv[2]);

	int width = 640;
	int height = 480;
	int videowidth = 640;
	int videoheight = 480;

	CSerialPort serial;
	serial.Open();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow("tinysys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

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

		// TODO: Intercept '~' or CTRL+C

		// Detect key changes
		SDL_Keymod modifiers = SDL_GetModState();
		if (memcmp(old_keystates, keystates, SDL_NUM_SCANCODES))
		{
			//fprintf(stderr, "modifiers: %d\n", modifiers);

			// TODO: Send the scancode and state to the remote device
			for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
			{
				if (keystates[i] != old_keystates[i])
				{
					//fprintf(stderr, "key %d: %d\n", i, keystates[i]);
					uint8_t outdata[4];
					outdata[0] = '^';					// scancode packet marker
					outdata[1] = i;						// scancode
					outdata[2] = keystates[i];			// state
					outdata[3] = modifiers&0xFF;		// lower byte of modifiers
					outdata[4] = (modifiers>>8)&0xFF;	// upper byte of modifiers
					serial.Send(outdata, 5);
				}
			}

			// Copy the new keystates to the old keystates
			memcpy(old_keystates, keystates, SDL_NUM_SCANCODES);
		}
	} while(s_alive);

	SDL_DestroyWindow(window);

	serial.Close();
	printf("remote connection terminated\n");

#if defined(CAT_LINUX)
	terminate_video_capture(video_capture);
#else
	// TODO: Windows and MacOS
#endif

	return 0;
}
