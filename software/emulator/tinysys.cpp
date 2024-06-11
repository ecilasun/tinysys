#include <stdio.h>
#include "emulator.h"

#ifdef CAT_WINDOWS
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

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
    SDL_Renderer* renderer = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL2: %s\n", SDL_GetError());
        return -1;
    }
    window = SDL_CreateWindow("tinysys emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    bool alive = true;
    SDL_Event ev;
    int ticks = 0;
    do
    {
        if (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_QUIT)
                alive = false;
        }

        if ((ticks % 65536) == 0) // TODO: Tune this to 60Hz-ish
        {
            uint32_t *pixels = (uint32_t*)surface->pixels;
            if (SDL_MUSTLOCK(surface))
                SDL_LockSurface(surface);
            emulator.UpdateVideoLink(pixels);
            if (SDL_MUSTLOCK(surface))
                SDL_UnlockSurface(surface);
            SDL_UpdateWindowSurface(window);
        }
        ++ticks;

        // TODO: Move emulator part to a thread
        alive = emulator.Step();
    } while(alive);

    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
