#include <stdio.h>
#include "emulator.h"

int main(int argc, char **argv)
{
    printf("tinysys emulator v0.1\n");

    CEmulator emulator;
    emulator.Reset();

    bool done;
    do{
        done = emulator.Step();
    } while(!done);

    return 0;
}
