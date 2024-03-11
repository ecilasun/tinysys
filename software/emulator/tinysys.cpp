#include <stdio.h>
#include "emulator.h"

#ifdef CAT_WINDOWS
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

#if defined(CAT_LINUX)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
    printf("tinysys emulator v0.1\n");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL2: %s\n", SDL_GetError());
        return -1;
    }

    SDL_CreateWindow("tinysys emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);

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

    bool alive = true;
    do
    {
        alive = emulator.Step();
    } while(alive);

    return 0;
}
