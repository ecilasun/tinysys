#include "video.h"
#include <stdio.h>
#include <unordered_set>

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
char audiocapdevicename[512] = "/dev/video0";
#else // CAT_WINDOWS
char capturedevicename[512] = "capture";
char audiocapdevicename[512] = "capture";
#endif

// Here's a really nice tutorial on how to do video capture across multiple devices:
// https://www.dynamsoft.com/codepool/windows-cpp-camera-barcode-scanner.html

const char* GetVideoDeviceName()
{
	return capturedevicename;
}

const char* GetAudioCapDeviceName()
{
	return audiocapdevicename;
}

void SetVideoDeviceName(const char* name)
{
	strcpy(capturedevicename, name);
}

void SetAudioCapDeviceName(const char* name)
{
	strcpy(audiocapdevicename, name);
}

VideoCapture::VideoCapture()
{
}

VideoCapture::~VideoCapture()
{
}

#if defined(CAT_LINUS) || defined(CAT_MACOS)
// TODO: MacOS and Linux
#else

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return #val
#endif

const char* GetGUIDNameConst(const GUID& guid)
{
    IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_SUBTYPE);
    IF_EQUAL_RETURN(guid, MF_MT_ALL_SAMPLES_INDEPENDENT);
    IF_EQUAL_RETURN(guid, MF_MT_FIXED_SIZE_SAMPLES);
    IF_EQUAL_RETURN(guid, MF_MT_COMPRESSED);
    IF_EQUAL_RETURN(guid, MF_MT_SAMPLE_SIZE);
    IF_EQUAL_RETURN(guid, MF_MT_WRAPPED_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_NUM_CHANNELS);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_SECOND);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BLOCK_ALIGNMENT);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BITS_PER_SAMPLE);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_BLOCK);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_CHANNEL_MASK);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FOLDDOWN_MATRIX);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKREF);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKTARGET);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGREF);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGTARGET);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_PREFER_WAVEFORMATEX);
    IF_EQUAL_RETURN(guid, MF_MT_AAC_PAYLOAD_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_SIZE);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MAX);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MIN);
    IF_EQUAL_RETURN(guid, MF_MT_PIXEL_ASPECT_RATIO);
    IF_EQUAL_RETURN(guid, MF_MT_DRM_FLAGS);
    IF_EQUAL_RETURN(guid, MF_MT_PAD_CONTROL_FLAGS);
    IF_EQUAL_RETURN(guid, MF_MT_SOURCE_CONTENT_HINT);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_CHROMA_SITING);
    IF_EQUAL_RETURN(guid, MF_MT_INTERLACE_MODE);
    IF_EQUAL_RETURN(guid, MF_MT_TRANSFER_FUNCTION);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_PRIMARIES);
    IF_EQUAL_RETURN(guid, MF_MT_CUSTOM_VIDEO_PRIMARIES);
    IF_EQUAL_RETURN(guid, MF_MT_YUV_MATRIX);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_LIGHTING);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_NOMINAL_RANGE);
    IF_EQUAL_RETURN(guid, MF_MT_GEOMETRIC_APERTURE);
    IF_EQUAL_RETURN(guid, MF_MT_MINIMUM_DISPLAY_APERTURE);
    IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_APERTURE);
    IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_ENABLED);
    IF_EQUAL_RETURN(guid, MF_MT_AVG_BITRATE);
    IF_EQUAL_RETURN(guid, MF_MT_AVG_BIT_ERROR_RATE);
    IF_EQUAL_RETURN(guid, MF_MT_MAX_KEYFRAME_SPACING);
    IF_EQUAL_RETURN(guid, MF_MT_DEFAULT_STRIDE);
    IF_EQUAL_RETURN(guid, MF_MT_PALETTE);
    IF_EQUAL_RETURN(guid, MF_MT_USER_DATA);
    IF_EQUAL_RETURN(guid, MF_MT_AM_FORMAT_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG_START_TIME_CODE);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG2_PROFILE);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG2_LEVEL);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG2_FLAGS);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG_SEQUENCE_HEADER);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_0);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_0);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_1);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_1);
    IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_SRC_PACK);
    IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_CTRL_PACK);
    IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_HEADER);
    IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_FORMAT);
    IF_EQUAL_RETURN(guid, MF_MT_IMAGE_LOSS_TOLERANT); 
    IF_EQUAL_RETURN(guid, MF_MT_MPEG4_SAMPLE_DESCRIPTION);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
    IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_4CC); 
    IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_WAVE_FORMAT_TAG);
    
    // Media types

    IF_EQUAL_RETURN(guid, MFMediaType_Audio);
    IF_EQUAL_RETURN(guid, MFMediaType_Video);
    IF_EQUAL_RETURN(guid, MFMediaType_Protected);
    IF_EQUAL_RETURN(guid, MFMediaType_SAMI);
    IF_EQUAL_RETURN(guid, MFMediaType_Script);
    IF_EQUAL_RETURN(guid, MFMediaType_Image);
    IF_EQUAL_RETURN(guid, MFMediaType_HTML);
    IF_EQUAL_RETURN(guid, MFMediaType_Binary);
    IF_EQUAL_RETURN(guid, MFMediaType_FileTransfer);

    IF_EQUAL_RETURN(guid, MFVideoFormat_AI44); //     FCC('AI44')
    IF_EQUAL_RETURN(guid, MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8 
    IF_EQUAL_RETURN(guid, MFVideoFormat_AYUV); //     FCC('AYUV')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DV25); //     FCC('dv25')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DV50); //     FCC('dv50')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DVH1); //     FCC('dvh1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DVSD); //     FCC('dvsd')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DVSL); //     FCC('dvsl')
    IF_EQUAL_RETURN(guid, MFVideoFormat_H264); //     FCC('H264')
    IF_EQUAL_RETURN(guid, MFVideoFormat_I420); //     FCC('I420')
    IF_EQUAL_RETURN(guid, MFVideoFormat_IYUV); //     FCC('IYUV')
    IF_EQUAL_RETURN(guid, MFVideoFormat_M4S2); //     FCC('M4S2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MJPG);
    IF_EQUAL_RETURN(guid, MFVideoFormat_MP43); //     FCC('MP43')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MP4S); //     FCC('MP4S')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MP4V); //     FCC('MP4V')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MPG1); //     FCC('MPG1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MSS1); //     FCC('MSS1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MSS2); //     FCC('MSS2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_NV11); //     FCC('NV11')
    IF_EQUAL_RETURN(guid, MFVideoFormat_NV12); //     FCC('NV12')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P010); //     FCC('P010')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P016); //     FCC('P016')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P210); //     FCC('P210')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P216); //     FCC('P216')
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB24); //    D3DFMT_R8G8B8 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB565); //   D3DFMT_R5G6B5 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB8);
    IF_EQUAL_RETURN(guid, MFVideoFormat_UYVY); //     FCC('UYVY')
    IF_EQUAL_RETURN(guid, MFVideoFormat_v210); //     FCC('v210')
    IF_EQUAL_RETURN(guid, MFVideoFormat_v410); //     FCC('v410')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WMV1); //     FCC('WMV1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WMV2); //     FCC('WMV2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WMV3); //     FCC('WMV3')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WVC1); //     FCC('WVC1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y210); //     FCC('Y210')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y216); //     FCC('Y216')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y410); //     FCC('Y410')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y416); //     FCC('Y416')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y41P);
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y41T);
    IF_EQUAL_RETURN(guid, MFVideoFormat_YUY2); //     FCC('YUY2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_YV12); //     FCC('YV12')
    IF_EQUAL_RETURN(guid, MFVideoFormat_YVYU);

    IF_EQUAL_RETURN(guid, MFAudioFormat_PCM); //              WAVE_FORMAT_PCM 
    IF_EQUAL_RETURN(guid, MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT 
    IF_EQUAL_RETURN(guid, MFAudioFormat_DTS); //              WAVE_FORMAT_DTS 
    IF_EQUAL_RETURN(guid, MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF 
    IF_EQUAL_RETURN(guid, MFAudioFormat_DRM); //              WAVE_FORMAT_DRM 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF 
    IF_EQUAL_RETURN(guid, MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9 
    IF_EQUAL_RETURN(guid, MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3 
    IF_EQUAL_RETURN(guid, MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG 
    IF_EQUAL_RETURN(guid, MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC 
    IF_EQUAL_RETURN(guid, MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC 

    return NULL;
}

HRESULT EnumerateCaptureFormats(IMFMediaSource *pSource)
{
    IMFPresentationDescriptor *pPD = NULL;
    IMFStreamDescriptor *pSD = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    IMFMediaType *pType = NULL;
    DWORD cTypes = 0;
	std::unordered_set<char*> subtypenames;

    HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);
    if (FAILED(hr))
    {
        goto done;
    }

    BOOL fSelected;
    hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pSD->GetMediaTypeHandler(&pHandler);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pHandler->GetMediaTypeCount(&cTypes);
    if (FAILED(hr))
    {
        goto done;
    }

    for (DWORD i = 0; i < cTypes; i++)
    {
        hr = pHandler->GetMediaTypeByIndex(i, &pType);
        if (FAILED(hr))
        {
            goto done;
        }

		GUID subType;
		hr = pType->GetGUID(MF_MT_SUBTYPE, &subType);
		if (SUCCEEDED(hr))
		{
			const char *name = GetGUIDNameConst(subType);
			subtypenames.insert((char*)name);
		}

        pType->Release();
    }

	// Print the list of subtypes
	fprintf(stderr, " Available video formats on selected device:\n");
	for (auto it = subtypenames.begin(); it != subtypenames.end(); ++it)
	{
		if (*it != nullptr)
		{
			fprintf(stderr, "  %s\n", *it);
		}
		else
		{
			fprintf(stderr, "  Unknown subtype\n");
		}
	}

done:
    pPD->Release();
    pSD->Release();
    pHandler->Release();
    pType->Release();

	return hr;
}

HRESULT VideoCapture::CreateVideoSource(IMFMediaSource **ppSource)
{
	*ppSource = NULL;

	devicecount = 0;

	IMFAttributes *pConfig = NULL;
	IMFActivate **ppDevices = NULL;

	HRESULT hr = MFCreateAttributes(&pConfig, 1);

	if (SUCCEEDED(hr))
		hr = pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	else
	{
		fprintf(stderr, "MFCreateAttributes failed\n");
		return E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		hr = MFEnumDeviceSources(pConfig, &ppDevices, &devicecount);
		if (FAILED(hr) && hr != MF_E_NOT_FOUND)
		{
			fprintf(stderr, "MFEnumDeviceSources failed\n");
			return E_FAIL;
		}
	}
	else
	{
		fprintf(stderr, "SetGUID failed\n");
		return E_FAIL;
	}
	
	if (SUCCEEDED(hr))
	{
		selectedVideodevice = 0xFFFFFFFF;

		for (DWORD i = 0; i < devicecount; i++)
		{
			wchar_t *name = nullptr;
			UINT32 namelen = 0;
			hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &namelen);
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
				fprintf(stderr, ">Using video capture device(%d): %s\n", i, asciiname);
				selectedVideodevice = i;
			}
			else
				fprintf(stderr, " Found video capture device(%d): %s\n", i, asciiname);
		}

		if (selectedVideodevice != 0xFFFFFFFF)
		{
			hr = ppDevices[selectedVideodevice]->ActivateObject(IID_PPV_ARGS(ppSource));
			if (FAILED(hr))
			{
				fprintf(stderr, "ActivateObject failed\n");
				return E_FAIL;
			}
		}
		else
		{
			fprintf(stderr, "No video capture device found\n");
			hr = MF_E_NOT_FOUND;
		}
	}
	else
	{
		fprintf(stderr, "MFEnumDeviceSources failed\n");
		return E_FAIL;
	}

	for (DWORD i = 0; i < devicecount; i++)
		ppDevices[i]->Release();
	CoTaskMemFree(ppDevices);

	return hr;
}

HRESULT VideoCapture::CreateAudioSource(IMFMediaSource **ppSource)
{
	*ppSource = NULL;

	devicecount = 0;

	IMFAttributes *pConfig = NULL;
	IMFActivate **ppDevices = NULL;

	HRESULT hr = MFCreateAttributes(&pConfig, 1);

	if (SUCCEEDED(hr))
		hr = pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);

	if (SUCCEEDED(hr))
		hr = MFEnumDeviceSources(pConfig, &ppDevices, &devicecount);

	if (SUCCEEDED(hr))
	{
		selectedAudiodevice = 0xFFFFFFFF;

		for (DWORD i = 0; i < devicecount; i++)
		{
			wchar_t *name = nullptr;
			UINT32 namelen = 0;
			hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &namelen);
			char asciiname[512];
			if (SUCCEEDED(hr))
			{
				wcstombs_s(nullptr, asciiname, name, namelen);
				CoTaskMemFree(name);
			}
	
			// Skip things that look like camera devices
			// TODO: This is a hack, need a better way to detect cameras or non-camera devices
			if (strstr(asciiname, audiocapdevicename) != nullptr)
			{
				// Found a non-camera device, use it
				fprintf(stderr, ">Using audio capture device(%d): %s\n", i, asciiname);
				selectedAudiodevice = i;
			}
			else
				fprintf(stderr, " Found audio capture device(%d): %s\n", i, asciiname);
		}

		if (selectedAudiodevice != 0xFFFFFFFF)
			hr = ppDevices[selectedAudiodevice]->ActivateObject(IID_PPV_ARGS(ppSource));
		else
			hr = MF_E_NOT_FOUND;
	}

	for (DWORD i = 0; i < devicecount; i++)
		ppDevices[i]->Release();
	CoTaskMemFree(ppDevices);

	return hr;
}

HRESULT VideoCapture::CreateAggregateSource(IMFMediaSource *pVideoSource, IMFMediaSource *pAudioSource, IMFMediaSource **ppAggregateSource)
{
	HRESULT hr = S_OK;

	// Create aggregate source.
	hr = MFCreateCollection(&pCollection);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFCreateCollection failed\n");
		return hr;
	}

	hr = pCollection->AddElement(videosource);
	if (FAILED(hr))
	{
		fprintf(stderr, "Add(video) failed\n");
		return hr;
	}

	hr = pCollection->AddElement(audiosource);
	if (FAILED(hr))
	{
		fprintf(stderr, "Add(audio) failed\n");
		return hr;
	}

	hr = MFCreateAggregateSource(pCollection, &aggregatesource);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFCreateAggregateSource failed\n");
		return hr;
	}

	pCollection->Release();
	pCollection = nullptr;

	return hr;
}

HRESULT VideoCapture::CreateSourceReader(IMFMediaSource *pAggregateSource, const uint32_t width, const uint32_t height, const uint32_t framerate, const uint32_t format)
{
	// Create the source reader.
	HRESULT hr = MFCreateSourceReaderFromMediaSource(pAggregateSource, nullptr, &pAggregateReader);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFCreateSourceReaderFromMediaSource(aggregate) failed\n");
		return hr;
	}

	hr = pAggregateReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, TRUE);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetStreamSelection failed\n");
		return hr;
	}

	IMFMediaType* mediaType = nullptr;
	hr = MFCreateMediaType(&mediaType);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFCreateMediaType(video) failed\n");
		return hr;
	}

	hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetGUID failed\n");
		return hr;
	}

	hr = mediaType->SetGUID(MF_MT_SUBTYPE, videoformat == 0 ? MFVideoFormat_YUY2 : (videoformat == 1 ? MFVideoFormat_RGB24 : MFVideoFormat_MJPG));
	if (FAILED(hr))
	{
		fprintf(stderr, "SetGUID failed\n");
		return hr;
	}

	hr = MFSetAttributeSize(mediaType, MF_MT_FRAME_SIZE, width, height);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFSetAttributeSize failed\n");
		return hr;
	}

	hr = MFSetAttributeRatio(mediaType, MF_MT_FRAME_RATE, framerate, 1);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFSetAttributeRatio failed\n");
		return hr;
	}

	// Set the video format.
	hr = pAggregateReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaType);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetCurrentMediaType(video) failed\n");
		return hr;
	}

	IMFMediaType* audioType = nullptr;
	hr = MFCreateMediaType(&audioType);
	if (FAILED(hr))
	{
		fprintf(stderr, "MFCreateMediaType(audio) failed\n");
		return hr;
	}

	hr = audioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetGUID failed\n");
		return hr;
	}

	hr = audioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetGUID failed\n");
		return hr;
	}

	hr = audioType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
	if (FAILED(hr))
	{
		fprintf(stderr, "MF_MT_AUDIO_NUM_CHANNELS failed\n");
		return hr;
	}

	hr = audioType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
	if (FAILED(hr))
	{
		fprintf(stderr, "MF_MT_AUDIO_BITS_PER_SAMPLE failed\n");
		return hr;
	}

	hr = audioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000);
	if (FAILED(hr))
	{
		fprintf(stderr, "MF_MT_AUDIO_SAMPLES_PER_SECOND failed\n");
		return hr;
	}

	// Set the audio format.
	hr = pAggregateReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, audioType);
	if (FAILED(hr))
	{
		fprintf(stderr, "SetCurrentMediaType(audio) failed\n");
		return hr;
	}

	return hr;
}

#endif


bool VideoCapture::Initialize(int width, int height, int fps, int format)
{
	frameWidth = width;
	frameHeight = height;
	framerate = fps;
	videoformat = format;

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

	hr = CreateVideoSource(&videosource);
	if (FAILED(hr))
	{
		fprintf(stderr, "CreateVideoSource failed\n");
		return false;
	}

	// List all available video formats
	EnumerateCaptureFormats(videosource);

	hr = CreateAudioSource(&audiosource);
	if (FAILED(hr))
	{
		fprintf(stderr, "CreateAudioSource failed\n");
		return false;
	}

	hr = CreateAggregateSource(videosource, audiosource, &aggregatesource);
	if (FAILED(hr))
	{
		fprintf(stderr, "CreateAggregateSource failed\n");
		return false;
	}

	hr = CreateSourceReader(aggregatesource, width, height, framerate, videoformat);
	if (FAILED(hr))
	{
		fprintf(stderr, "CreateSourceReader failed\n");
		return false;
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

	if (audiosource)
	{
		audiosource->Shutdown();
		audiosource->Release();
	}

	if (videosource)
	{
		videosource->Shutdown();
		videosource->Release();
	}

	if (pAggregateReader)
	{
		pAggregateReader->Release();
		pAggregateReader = nullptr;
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
		/*int col = (i/2) % width;
		int row = (i/2) / width;
		int dim = row % 2;*/
		unsigned char y1 = yuy2Data[i];// >> dim;
		unsigned char u = yuy2Data[i + 1];
		unsigned char y2 = yuy2Data[i + 2];// >> dim;
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

bool VideoCapture::CaptureFrame(uint8_t *videodata, AudioPlayback* audio)
{
#if defined(CAT_LINUX)
	// Linux
#elif defined(CAT_DARWIN)
	// MacOS
#else // CAT_WINDOWS

	bool retval = true;

	HRESULT hr;
	DWORD streamIndex, flags;
	LONGLONG timestamp;
	IMFSample *sample = nullptr;

	if (!pAggregateReader)
	{
		fprintf(stderr, "No valid aggregate reader\n");
		return false;
	}

	hr = pAggregateReader->ReadSample(
		MF_SOURCE_READER_ALL_STREAMS,
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
		//fprintf(stderr, "Stream index: %d\n", streamIndex);
		if (streamIndex == 0)
		{
			//fprintf(stderr, "Video stream\n");
			IMFMediaBuffer *buffer = nullptr;
			hr = sample->ConvertToContiguousBuffer(&buffer);
			if (FAILED(hr))
			{
				retval = false;
				fprintf(stderr, "Failed to convert sample to contiguous buffer.\n");
			}
			else
			{
				BYTE *rawData = nullptr;
				DWORD maxLength = 0, currentLength = 0;
				hr = buffer->Lock(&rawData, &maxLength, &currentLength);
				if (SUCCEEDED(hr))
				{
					//if (videoformat==0)
						ConvertYUY2ToRGB(rawData, videodata, frameWidth, frameHeight);
					//else if (videoformat==1 || videoformat==2) // TODO: Deal with these formats or use auto-conversion
					//	CopyVideoData(rawData, videodata, frameWidth, frameHeight);

					// NOTE about media transforms:
					/*MFT_REGISTER_TYPE_INFO inputFilter = { MFMediaType_Video, MFVideoFormat_MJPG };
					MFT_REGISTER_TYPE_INFO outputFilter = { MFMediaType_Video, MFVideoFormat_YUY2 };
					UINT32 unFlags = MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER;
					
					HRESULT r = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, unFlags, &inputFilter, &outputFilter, &ppActivate, &numDecodersMJPG);
					if (FAILED(r)) throw gcnew Exception("");
					if (numDecodersMJPG < 1) throw gcnew Exception("");
					
					// Activate transform
					IMFTransform *pMPEG4 = NULL;
					r = ppActivate[0]->ActivateObject(__uuidof(IMFTransform), (void**)&pMPEG4);
					if (FAILED(r)) throw gcnew Exception("MJPG decoder not available.");*/

					buffer->Unlock();
				}
				else
				{
					retval = false;
					fprintf(stderr, "Failed to lock buffer.\n");
				}
			}
			buffer->Release();
		}
		else
		{
			//fprintf(stderr, "Audio stream\n");
			IMFMediaBuffer *buffer = nullptr;
			hr = sample->ConvertToContiguousBuffer(&buffer);
			if (FAILED(hr))
			{
				retval = false;
				fprintf(stderr, "Failed to convert sample to contiguous buffer.\n");
			}
			else
			{
				BYTE *rawData = nullptr;
				DWORD maxLength = 0, currentLength = 0;
				hr = buffer->Lock(&rawData, &maxLength, &currentLength);
				if (SUCCEEDED(hr))
				{
					if (audioBuffer == nullptr)
						audioBuffer = new int16_t[32768*2];

					float* audiosamples = (float*)rawData;
					for(int i = 0; i < currentLength / sizeof(float); ++i)
						audioBuffer[i] = (int16_t)(audiosamples[i] * 32768.f);

					buffer->Unlock();

					SDL_QueueAudio(audio->selectedplaybackdevice, audioBuffer, currentLength/2);
				}
				else
				{
					fprintf(stderr, "Failed to lock buffer.\n");
					retval = false;
				}
			}
			buffer->Release();
		}
		sample->Release();
	}
	else
	{
		retval = false;
		fprintf(stderr, "No sample\n");
	}
#endif

	return retval;
}
