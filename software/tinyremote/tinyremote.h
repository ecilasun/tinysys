#pragma once

#include "platform.h"
#include "common.h"
#include "serial.h"
#include "video.h"
#include "audio.h"
#include "lz4.h"

struct AppCtx
{
	VideoCapture* video;
	AudioPlayback* audio;
	CSerialPort* serial;
	SDL_GameController* gamecontroller;
};
