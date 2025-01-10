#include "video.h"
#include <stdio.h>

#if defined(CAT_LINUX)
int vtype = 0;
v4l2_buffer vbufferinfo;
char* vbuffer = nullptr;
uint32_t *intermediate = nullptr;
unsigned int vbufferlen = 0;
bool isForeground = false;
Display* dpy;
#elif defined(CAT_DARWIN)
// MacOS
#else // CAT_WINDOWS
// Windows
#endif

// TODO: Use SDL3 so we can unify this
#if defined(CAT_LINUX) || defined(CAT_MACOS)
char capturedevicename[512] = "/dev/video0";
#else // CAT_WINDOWS
char capturedevicename[512] = "capture";
#endif

// Here's a really nice tutorial on how to do video capture across multiple devices:
// https://www.dynamsoft.com/codepool/windows-cpp-camera-barcode-scanner.html

const char* GetVideoDeviceName()
{
	return capturedevicename;
}

void SetVideoDeviceName(const char* name)
{
	strcpy(capturedevicename, name);
}

VideoCapture::VideoCapture()
{
}

VideoCapture::~VideoCapture()
{
}

bool VideoCapture::Initialize(int width, int height)
{
	frameWidth = width;
	frameHeight = height;

#if defined(CAT_LINUX)
	intermediate = (uint32_t*)malloc(width*height*4);

	// Video capture
	video_capture = open(capturedevicename, O_RDWR);
	if (video_capture < 0)
	{
		fprintf(stderr, "cannot open %s\n", capturedevicename);
		return -1;
	}

	v4l2_capability capability;
	if(ioctl(video_capture, VIDIOC_QUERYCAP, &capability) < 0)
	{
		perror("Failed to get device capabilities, VIDIOC_QUERYCAP");
		return -1;
	}

	v4l2_format imageFormat;
	imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	imageFormat.fmt.pix.width = width;
	imageFormat.fmt.pix.height = height;
	imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
	imageFormat.fmt.pix.bytesperline = width*2; // YCbYCrYCbYCr, 2 bytes per pixel
	imageFormat.fmt.pix.sizeimage = imageFormat.fmt.pix.bytesperline * height;
	if(ioctl(video_capture, VIDIOC_S_FMT, &imageFormat) < 0)
	{
		perror("device could not set format, VIDIOC_S_FMT");
		return -1;
	}

	v4l2_requestbuffers requestBuffer = {0};
	requestBuffer.count = 1; // one request buffer
	requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we an use for capturing frames
	requestBuffer.memory = V4L2_MEMORY_MMAP;
	if(ioctl(video_capture, VIDIOC_REQBUFS, &requestBuffer) < 0)
	{
		perror("could not request buffer from device, VIDIOC_REQBUFS");
		return false;
	}

	v4l2_buffer queryBuffer = {0};
	queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	queryBuffer.memory = V4L2_MEMORY_MMAP;
	queryBuffer.index = 0;
	if(ioctl(video_capture, VIDIOC_QUERYBUF, &queryBuffer) < 0)
	{
		perror("device did not return the buffer information, VIDIOC_QUERYBUF");
		return false;
	}
	vbuffer = (char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, video_capture, queryBuffer.m.offset);
	vbufferlen = queryBuffer.length;
	memset(vbuffer, 0, vbufferlen);

	memset(&vbufferinfo, 0, sizeof(vbufferinfo));
	vbufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbufferinfo.memory = V4L2_MEMORY_MMAP;
	vbufferinfo.index = 0;

	vtype = vbufferinfo.type;
	if(ioctl(video_capture, VIDIOC_STREAMON, &vtype) < 0)
	{
		perror("could not start streaming, VIDIOC_STREAMON");
		return false;
	}

	return true;
#elif defined(CAT_DARWIN)
	// MacOS
	return false;
#else // CAT_WINDOWS

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
	{
		fprintf(stderr, "CoInitializeEx failed\n");
		return false;
	}

	hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFStartup failed\n");
		return false;
	}

	devicecount = 0;

	IMFAttributes *pConfig = nullptr;

	// Create an attribute store to hold the search criteria.
	hr = MFCreateAttributes(&pConfig, 1);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFCreateAttributes failed\n");
		return false;
	}

	// Request video capture devices.
	hr = pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetGUID failed\n");
		return false;
	}

	IMFActivate **devices = nullptr;

	// Enumerate the devices,
	selecteddevice = 0xFFFFFFFF;
	devicecount = 0;
	hr = MFEnumDeviceSources(pConfig, &devices, &devicecount);
	if( FAILED(hr) || devicecount == 0 )
	{
		fprintf(stderr, "MFEnumDeviceSources failed or there were no capture devices\n");
		return false;
	}

	for (DWORD i = 0; i < devicecount; i++)
	{
		wchar_t *name = nullptr;
		UINT32 namelen = 0;
		hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &namelen);
		char asciiname[512];
		if (SUCCEEDED(hr))
		{
			wcstombs_s(nullptr, asciiname, name, namelen);
			CoTaskMemFree(name);
		}

		// Skip things that look like camera devices
		// TODO: This is a hack, need a better way to detect cameras or non-camera devices
		if (strstr(asciiname, capturedevicename) != nullptr)
		{
			// Found a non-camera device, use it
			fprintf(stderr, "Using video capture device(%d): %s\n", i, asciiname);
			selecteddevice = i;
		}

		fprintf(stderr, "Found video capture device(%d): %s\n", i, asciiname);
	}

	// Create a media source for the first device in the list.
	if (devicecount > 0 && selecteddevice != 0xFFFFFFFF)
	{
		hr = devices[selecteddevice]->ActivateObject(IID_PPV_ARGS(&mediaSource));
		for (DWORD i = 0; i < devicecount; i++)
		{
			devices[i]->Release();
		}
		CoTaskMemFree(devices);
		if (SUCCEEDED(hr))
		{
			fprintf(stderr, "Activated video capture device %d\n", selecteddevice);
		}
		else
		{
			fprintf(stderr, "ActivateObject failed\n");
			return false;
		}
	}
	else
	{
		hr = MF_E_NOT_FOUND;
	}

	// Create the source reader.
	if (SUCCEEDED(hr))
	{
		// Create the source reader.
		hr = MFCreateSourceReaderFromMediaSource(mediaSource, nullptr, &pReader);
		if (FAILED(hr))
		{
			fprintf(stderr, "MFCreateSourceReaderFromMediaSource failed\n");
			return false;
		}

		hr = pReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, TRUE);
		if (FAILED(hr))
		{
			fprintf(stderr, "SetStreamSelection failed\n");
			return false;
		}

		IMFMediaType* mediaType = nullptr;
		hr = MFCreateMediaType(&mediaType);
		if (FAILED(hr))
		{
			fprintf(stderr, "MFCreateMediaType failed\n");
			return false;
		}

		hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		if (FAILED(hr))
		{
			fprintf(stderr, "SetGUID failed\n");
			return false;
		}

		hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
		if (FAILED(hr))
		{
			fprintf(stderr, "SetGUID failed\n");
			return false;
		}

		hr = MFSetAttributeSize(mediaType, MF_MT_FRAME_SIZE, width, height);
		if (FAILED(hr))
		{
			fprintf(stderr, "MFSetAttributeSize failed\n");
			return false;
		}

		hr = MFSetAttributeRatio(mediaType, MF_MT_FRAME_RATE, 60, 1);
		if (FAILED(hr))
		{
			fprintf(stderr, "MFSetAttributeRatio failed\n");
			return false;
		}

		// Set the video format.
		hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaType);
		if (FAILED(hr))
		{
			fprintf(stderr, "SetCurrentMediaType failed\n");
			return false;
		}
	}

	return true;
#endif
}

void VideoCapture::Terminate()
{
#if defined(CAT_LINUX)
	free(intermediate);

	if (video_capture<0)
		return;

	if (vbufferlen)
		munmap(vbuffer, vbufferlen);

	if(ioctl(video_capture, VIDIOC_STREAMOFF, &vtype) < 0)
	{
		perror("could not end streaming, VIDIOC_STREAMOFF");
		return;
	}
	close(video_capture);
#elif defined(CAT_DARWIN)
	// MacOS
#else // CAT_WINDOWS

	if (pReader)
	{
		pReader->Release();
		pReader = nullptr;
	}
	if (mediaSource)
	{
		mediaSource->Shutdown();
		mediaSource->Release();
	}
	MFShutdown();
#endif
}

inline float clamp(float x, float min, float max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}

void ConvertYUY2ToRGB(const unsigned char *yuy2Data, unsigned char *rgbData, int width, int height)
{
	int rgbIndex = 0;
	for (int i = 0; i < width * height * 2; i += 4)
	{
		unsigned char y1 = yuy2Data[i];
		unsigned char u = yuy2Data[i + 1];
		unsigned char y2 = yuy2Data[i + 2];
		unsigned char v = yuy2Data[i + 3];
	
#if defined(CAT_WINDOWS)
		rgbData[rgbIndex++] = clamp(y1 + 1.772 * (u - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y1 - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y1 + 1.402 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = 0xFF;
	
		rgbData[rgbIndex++] = clamp(y2 + 1.772 * (u - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y2 - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y2 + 1.402 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = 0xFF;
#else
		rgbData[rgbIndex++] = clamp(y1 + 1.402 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y1 - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y1 + 1.772 * (u - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = 0xFF;
	
		rgbData[rgbIndex++] = clamp(y2 + 1.402 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y2 - 0.344136 * (u - 128) - 0.714136 * (v - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = clamp(y2 + 1.772 * (u - 128), 0.0, 255.0);
		rgbData[rgbIndex++] = 0xFF;
#endif
	}
}

bool VideoCapture::CaptureFrame(uint8_t *videodata)
{
#if defined(CAT_LINUX)
	// Linux
#elif defined(CAT_DARWIN)
	// MacOS
#else // CAT_WINDOWS
	HRESULT hr;
	DWORD streamIndex, flags;
	LONGLONG timestamp;
	IMFSample *sample = nullptr;

	//FrameData frame;
	//frame.width = frameWidth;
	//frame.height = frameHeight;
	//frame.rgbData = nullptr;

	if (!pReader)
		return false;

	hr = pReader->ReadSample(
		MF_SOURCE_READER_FIRST_VIDEO_STREAM,
		0,
		&streamIndex,
		&flags,
		&timestamp,
		&sample);

	if (FAILED(hr))
	{
		fprintf(stderr, "ReadSample failed\n");
		return false; 
	}

	if (sample)
	{
		IMFMediaBuffer *buffer = nullptr;
		hr = sample->ConvertToContiguousBuffer(&buffer);
		if (FAILED(hr))
		{
			fprintf(stderr, "Failed to convert sample to contiguous buffer.\n");
			return false; 
		}

		BYTE *rawData = nullptr;
		DWORD maxLength = 0, currentLength = 0;
		hr = buffer->Lock(&rawData, &maxLength, &currentLength);
		if (SUCCEEDED(hr))
		{
			ConvertYUY2ToRGB(rawData, videodata, frameWidth, frameHeight);
			buffer->Unlock();
		}
		else
		{
			fprintf(stderr, "Failed to lock buffer.\n");
			return false;
		}
	}
#endif
	return true;
}
