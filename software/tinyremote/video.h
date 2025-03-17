#pragma once

#include "platform.h"
#include "audio.h"

#if defined(CAT_LINUX)
#include <libv4l2.h>
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
const char* GetAudioCapDeviceName();
void SetVideoDeviceName(const char* name);
void SetAudioCapDeviceName(const char* name);

class VideoCapture
{
public:
	VideoCapture();
	~VideoCapture();

	bool Initialize(int width, int height, int fps, int format);
	void Terminate();

	bool CaptureFrame(uint8_t *videodata, AudioPlayback* audio);

#if defined(CAT_LINUX)
	int video_capture = -1;
#elif defined(CAT_DARWIN)
	// MacOS
#else // CAT_WINDOWS
	HRESULT CreateVideoSource(IMFMediaSource **ppSource);
	HRESULT CreateAudioSource(IMFMediaSource **ppSource);
	HRESULT CreateAggregateSource(IMFMediaSource *pVideoSource, IMFMediaSource *pAudioSource, IMFMediaSource **ppAggregateSource);
	HRESULT CreateSourceReader(IMFMediaSource *pAggregateSource, const uint32_t width, const uint32_t height, const uint32_t framerate, const uint32_t format);
	IMFMediaSource *videosource = nullptr;
	IMFMediaSource *audiosource = nullptr;
	IMFMediaSource *aggregatesource = nullptr;
	IMFCollection *pCollection = nullptr;
	IMFSourceReader *pAggregateReader = nullptr;
	uint32_t devicecount = 0;
	uint32_t selectedVideodevice = 0;
	uint32_t selectedAudiodevice = 0;
	uint32_t framerate = 60;
	uint32_t videoformat = 0;
	int16_t *audioBuffer = nullptr;
#endif
	uint32_t frameWidth = 0;
	uint32_t frameHeight = 0;
};
