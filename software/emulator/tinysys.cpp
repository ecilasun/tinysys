#include <stdio.h>
#include "emulator.h"

int main(int argc, char **argv)
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

    bool alive = true;
    do
    {
        alive = emulator.Step();
    } while(alive);

    return 0;
}
