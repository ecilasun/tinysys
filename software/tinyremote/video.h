#pragma once

#include "platform.h"

#if defined(CAT_LINUX)
#include <v4l2.h>
#elif defined(CAT_DARWIN)
// MacOS
#else // CAT_WINDOWS
// Windows Media Foundation
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mferror.h>
#include <mfreadwrite.h>
#endif

const char* GetVideoDeviceName();
void SetVideoDeviceName(const char* name);

class VideoCapture
{
public:
	VideoCapture();
	~VideoCapture();

	bool Initialize(int width, int height);
	void Terminate();

	bool CaptureFrame(uint8_t *videodata);

#if defined(CAT_LINUX)
	int video_capture = -1;
#elif defined(CAT_DARWIN)
	// MacOS
#elif defined(CAT_WINDOWS)
	IMFMediaSource *mediaSource = nullptr;
	IMFSourceReader *pReader = nullptr;
	uint32_t devicecount = 0;
	uint32_t selecteddevice = 0;
#endif
	uint32_t frameWidth = 0;
	uint32_t frameHeight = 0;
};
