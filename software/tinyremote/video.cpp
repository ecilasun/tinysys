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

#if defined(CAT_LINUX) || defined(CAT_MACOS)
char capturedevicename[512] = "/dev/video0";
#else // CAT_WINDOWS
char capturedevicename[512] = "\\\\.\\VIDEO0"; // ???
#endif

const char* GetVideoDeviceName()
{
	return capturedevicename;
}

void SetVideoDeviceName(const char* name)
{
	strcpy(capturedevicename, name);
}

int initialize_video_capture(int width, int height)
{
#if defined(CAT_LINUX)
	intermediate = (uint32_t*)malloc(width*height*4);

	// Video capture
	int video_capture = open(capturedevicename, O_RDWR);
	if (video_capture < 0)
	{
		printf("cannot open %s\n", capturedevicename);
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
		return -1;
	}

	v4l2_buffer queryBuffer = {0};
	queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	queryBuffer.memory = V4L2_MEMORY_MMAP;
	queryBuffer.index = 0;
	if(ioctl(video_capture, VIDIOC_QUERYBUF, &queryBuffer) < 0)
	{
		perror("device did not return the buffer information, VIDIOC_QUERYBUF");
		return -1;
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
		return -1;
	}

	return video_capture;
#else
	// TODO: Windows and MacOS
	return -1;
#endif
}

void terminate_video_capture(int video_capture)
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
#else
	// TODO: Windows and MacOS
#endif
}
