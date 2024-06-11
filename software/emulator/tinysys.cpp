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

        if ((ticks % 8192) == 0) // TODO: Tune this to 60Hz-ish
        {
            //SDL_BlitSurface(mysurface, NULL, SDL_GetWindowSurface(window), NULL);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderDrawLine(renderer, 0, 0, 640, 480);
            SDL_RenderDrawLine(renderer, 640, 0, 0, 480);
            SDL_RenderPresent(renderer);
            SDL_UpdateWindowSurface(window);
        }

        alive = emulator.Step();
        ++ticks;
    } while(alive);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
