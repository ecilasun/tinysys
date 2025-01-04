#pragma once

#include "platform.h"

const char* GetVideoDeviceName();
void SetVideoDeviceName(const char* name);

int initialize_video_capture(int width, int height);
void terminate_video_capture(int video_capture);
