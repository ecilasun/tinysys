#include <stdio.h>
#include <string.h>

#include "gdbstub.h"
#include "emulator.h"
#include "SDL.h"
#include "SDL_ttf.h"

struct Axis6
{
	float leftx;
	float lefty;
	float rightx;
	float righty;
	float lefttrigger;
	float righttrigger;
};