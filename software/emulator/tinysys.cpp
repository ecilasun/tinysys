#include <stdio.h>
#include "emulator.h"

#ifdef CAT_WINDOWS
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

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

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
    printf("tinysys emulator v0.1\n");

    CEmulator emulator;
    bool success;

    if (argc<=1)
        success = emulator.Reset("rom.bin");
    else
        success = emulator.Reset(argv[1]);

    if (!success)
    {
        printf("Failed to load ROM\n");
        return -1;
    }

    const int WIDTH = 640;
    const int HEIGHT = 480;
    SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL2: %s\n", SDL_GetError());
        return -1;
    }
    window = SDL_CreateWindow("tinysys emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface)
    {
        printf("Could not create window surface\n");
        return -1;
    }

    SDL_Thread* thread = SDL_CreateThread(emulatorthread, "emulator", &emulator);

    SDL_Event ev;
    int ticks = 0;
    do
    {
        if (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_QUIT)
                s_alive = false;
        }

        if ((ticks % 65536) == 0) // TODO: tune this to time not counter
        {
            if (SDL_MUSTLOCK(surface))
                SDL_LockSurface(surface);
            uint32_t* pixels = (uint32_t*)surface->pixels;
            emulator.UpdateVideoLink(pixels, surface->pitch);
            if (SDL_MUSTLOCK(surface))
                SDL_UnlockSurface(surface);
            SDL_UpdateWindowSurface(window);
        }
        ++ticks;
    } while(s_alive);

    SDL_WaitThread(thread, nullptr);
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
