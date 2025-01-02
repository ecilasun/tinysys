#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "platform.h"
#include "common.h"


#if defined(CAT_LINUX)
int vtype = 0;
v4l2_buffer vbufferinfo;
char* vbuffer = nullptr;
uint32_t *intermediate = nullptr;
unsigned int vbufferlen = 0;
bool isForeground = false;
bool appDone = false;
Display* dpy;
#endif

static bool s_alive = true;

class CSerialPort{
	public:

	CSerialPort() { }
	~CSerialPort() { }

	bool Open()
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		// Open COM port
		serial_port = open(commdevicename, O_RDWR);
		if (serial_port < 0 )
		{
			printf("Error %i from open('%s'): %s\nPlease try another COM port path.\n", errno, commdevicename, strerror(errno));
			return false;
		}

		struct termios tty;
		if(tcgetattr(serial_port, &tty) != 0)
		{
			printf("Error %i from tcgetattr: %s\nPlease make sure your user has access to the COM device %s.\n", errno, strerror(errno), commdevicename);
			return false;
		}

		// Set tty. flags
		tty.c_cflag &= ~PARENB; // No parity
		tty.c_cflag &= ~CSTOPB; // One stop bit
		tty.c_cflag &= ~CSIZE;
		tty.c_cflag |= CS8; // 8 bits
		tty.c_cflag &= ~CRTSCTS;
		tty.c_cflag |= CREAD | CLOCAL; // Not model (local), write

		tty.c_lflag &= ~ICANON;
		tty.c_lflag &= ~ECHO;
		tty.c_lflag &= ~ISIG;
		tty.c_iflag &= ~(IXON | IXOFF | IXANY);
		tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

		tty.c_oflag &= ~OPOST;
		tty.c_oflag &= ~ONLCR;
		//tty.c_oflag &= ~OXTABS;
		//tty.c_oflag &= ~ONOEOT;

		tty.c_cc[VTIME] = 50;
		tty.c_cc[VMIN] = 10;

		cfsetispeed(&tty, B460800);
		cfsetospeed(&tty, B460800); // or only cfsetspeed(&tty, B460800);

		if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
			printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));

		printf("%s open\n", commdevicename);
		return true;

#else // CAT_WINDOWS
		hComm = CreateFileA(commdevicename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hComm != INVALID_HANDLE_VALUE)
		{
			serialParams.DCBlength = sizeof(serialParams);
			if (GetCommState(hComm, &serialParams))
			{
				serialParams.BaudRate = /*CBR_*/460800;		// 460800 baud
				serialParams.fBinary = true;
				serialParams.fDtrControl = DTR_CONTROL_DISABLE;
				serialParams.fRtsControl = RTS_CONTROL_DISABLE;
				serialParams.fParity = 0;
				serialParams.ByteSize = 8;				// 8 bit bytes
				serialParams.StopBits = ONESTOPBIT;		// 1 stop bit
				serialParams.Parity = NOPARITY;			// no parity
				serialParams.fOutX = 0;					// no xon/xoff
				serialParams.fInX = 0;
				if (SetCommState(hComm, &serialParams) != 0)
				{
					printf("%s open\n", commdevicename);
					timeouts.ReadIntervalTimeout = MAXDWORD;
					timeouts.ReadTotalTimeoutConstant = 0;
					timeouts.ReadTotalTimeoutMultiplier = 0;
					timeouts.WriteTotalTimeoutConstant = 0;
					timeouts.WriteTotalTimeoutMultiplier = 0;
					if (SetCommTimeouts(hComm, &timeouts) != 0)
						return true;
					else
						printf("ERROR: can't set communication timeouts\n");
				}
				else
					printf("ERROR: can't set communication parameters\n");
			}
			else
				printf("ERROR: can't get communication parameters\n");
		}
		else
			printf("ERROR: can't open COM port %s\n", commdevicename);
		return false;
#endif
	}

	uint32_t Receive(void *_target, unsigned int _rcvlength)
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		int n = read(serial_port, _target, _rcvlength);
		if (n < 0)
			printf("ERROR: read() failed\n");
		return n;
#else
		DWORD bytesread = 0;
		BOOL success = ReadFile(hComm, _target, _rcvlength, &bytesread, nullptr);
		return success ? bytesread : 0;
#endif
	}

	uint32_t Send(void *_sendbytes, unsigned int _sendlength)
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		int n = write(serial_port, _sendbytes, _sendlength);
		if (n < 0)
			printf("ERROR: write() failed\n");
		return n;
#else // CAT_WINDOWS
		DWORD byteswritten = 0;
		// Send the command
		BOOL success = WriteFile(hComm, _sendbytes, _sendlength, &byteswritten, nullptr);
		if (!success)
			printf("ERROR: write() failed\n");
		return success ? (uint32_t)byteswritten : 0;
#endif
	}

	void Close()
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		close(serial_port);
#else // CAT_WINDOWS
		CloseHandle(hComm);
#endif
	}

#if defined(CAT_LINUX) || defined(CAT_MACOS)
	int serial_port{-1};
#else // CAT_WINDOWS
	HANDLE hComm{INVALID_HANDLE_VALUE};
	DCB serialParams{0};
	COMMTIMEOUTS timeouts{0};
#endif
};

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

void *capture_input( void *ptr)
{
    CSerialPort* serial = (CSerialPort*)ptr;
#if defined(CAT_LINUX)

    unsigned char keys_old[32];
    unsigned char keys_new[32];

    memset(keys_old, 0, 32);
    memset(keys_new, 0, 32);

    uint8_t isdown = 1;
    uint8_t isup = 2;
    uint8_t startToken = ':';
    KeyCode lshift = XKeysymToKeycode( dpy, XK_Shift_L );
    KeyCode rshift = XKeysymToKeycode( dpy, XK_Shift_R );
    KeyCode lalt = XKeysymToKeycode( dpy, XK_Alt_L );
    KeyCode ralt = XKeysymToKeycode( dpy, XK_Alt_R );
    KeyCode lctrl = XKeysymToKeycode( dpy, XK_Control_L );
    KeyCode rctrl = XKeysymToKeycode( dpy, XK_Control_R );

    while(!appDone)
    {
        // Non-event
        if (isForeground)
        {
            if (XQueryKeymap(dpy, (char*)keys_new))
                {
                uint8_t dummy;
                for (uint32_t code = 0; code < 256; code++)
                {
                    uint8_t currdown = keys_new[code>>3] & GetMask(code);
                    uint8_t prevdown = keys_old[code>>3] & GetMask(code);
                    uint8_t scancode = GetScancode(code);

                    uint8_t modifierstate = 0;
                    if (!!( keys_new[ (lshift)>>3 ] & ( 1<<((lshift)&7) ) ) ||
                        !!( keys_new[ (rshift)>>3 ] & ( 1<<((rshift)&7) ) ))
                        modifierstate |= 0x22;
                    if (!!( keys_new[ (lalt)>>3 ] & ( 1<<((lalt)&7) ) ) ||
                        !!( keys_new[ (ralt)>>3 ] & ( 1<<((ralt)&7) ) ))
                        modifierstate |= 0x44;
                    if (!!( keys_new[ (lctrl)>>3 ] & ( 1<<((lctrl)&7) ) ) ||
                        !!( keys_new[ (rctrl)>>3 ] & ( 1<<((rctrl)&7) ) ))
                        modifierstate |= 0x11;

                    uint8_t keystate = 0;
                    if (currdown && (!prevdown))
                        keystate |= isdown;
                    if ((!currdown) && prevdown)
                        keystate |= isup;

                    if (keystate)
                    {
                        // printf("%.2X -> %.2X\n", code, scancode); // DEBUG output
                        uint8_t outdata[4];
                        outdata[0] = startToken;
                        outdata[1] = modifierstate;
                        outdata[2] = keystate;
                        outdata[3] = scancode;
						serial->Send(outdata, 4);
						serial->Receive(&dummy, 1);
                    }
                }
                memcpy(keys_old, keys_new, 32);
            }
        }
    }

    return nullptr;
#else
	return nullptr;
#endif
}

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
	printf("Usage: tinyremote commdevicename capturedevicename\ndefault comm device:%s default capture device:%s\nCtrl+C or PAUSE: quit current remote process\n", commdevicename, capturedevicename);

    if (argc > 1)
		strcpy(commdevicename, argv[1]);
    if (argc > 2)
		strcpy(capturedevicename, argv[2]);

	int width = 640;
	int height = 480;
	int videowidth = 640;
	int videoheight = 480;

#if defined(CAT_LINUX)
    dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        printf("Cannot open display\n");
        return -1;
    }
#else
	// TODO: Windows and MacOS
#endif

	CSerialPort serial;
	serial.Open();

#if defined(CAT_LINUX)
    // Window
    int screen_num = DefaultScreen(dpy);
    Visual *visual = DefaultVisual(dpy, screen_num);
    unsigned long background = WhitePixel(dpy, screen_num);
    unsigned long border = BlackPixel(dpy, screen_num);

    char *videodata = (char*)malloc(width*height*4);

    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0,0, width, height, 2, border, background);
    XStoreName(dpy, win, "tinysys remote");
    Pixmap pixmap = XCreatePixmap(dpy, win, width, height, 24);
    XImage *img = XCreateImage(dpy, visual, DefaultDepth(dpy,screen_num), ZPixmap, 0, videodata, width, height, 32, 0);

    XSelectInput(dpy, win, ButtonPressMask|StructureNotifyMask|KeyPressMask|KeyReleaseMask|KeymapStateMask|FocusChangeMask);
    XMapWindow(dpy, win);

    Atom wmDelete=XInternAtom(dpy, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(dpy, win, &wmDelete, 1);

    int video_capture = initialize_video_capture(width, height);

    pthread_t inputCaptureThread;
    pthread_create(&inputCaptureThread, NULL, capture_input, (void*)&serial);

    XEvent ev;
    while(!appDone)
    {
        if (video_capture>0)
        {
            // Video
            if(ioctl(video_capture, VIDIOC_QBUF, &vbufferinfo) < 0){
                perror("could not queue buffer, VIDIOC_QBUF");
                return -1;
            }

            if(ioctl(video_capture, VIDIOC_DQBUF, &vbufferinfo) < 0)
            {
                perror("could not dequeue the buffer, VIDIOC_DQBUF");
                return -1;
            }

            if (vbufferinfo.bytesused)
            {
                for(int y=0; y<videoheight; ++y)
                {
                    int pY = y*videowidth;
                    for(int x=0;x<videowidth/2;++x)
                    {
                        int idx0 = x*2+pY;
                        int idx1 = x*2+pY;
                        // current pixel luma, shared blue
                        uint8_t Y0 = vbuffer[idx1*2+0]; // Y0
                        uint8_t Cb = vbuffer[idx1*2+1]; // Cb
                        // adjacent pixel luma, shared red
                        uint8_t Y1 = vbuffer[(idx1+1)*2+0]; // Y1
                        uint8_t Cr = vbuffer[(idx1+1)*2+1]; // Cr

                        // Convert the pair to RGB
                        uint32_t A = YUVtoRGBX32(Y0, Cb, Cr);
                        uint32_t B = YUVtoRGBX32(Y1, Cb, Cr);

                        // Two successive pixels with individual luma and shared CbCr
                        intermediate[idx0] = A;
                        intermediate[idx0+1] = B;
                    }
                }

                // TODO: scale intermediate onto outputimage
                uint32_t *outputimage = (uint32_t*)img->data;
                memcpy(outputimage, intermediate, videowidth*videoheight*4);

                // Blit away
                XPutImage(dpy, pixmap, DefaultGC(dpy, screen_num), img, 0, 0, 0, 0, width, height);
                XCopyArea(dpy, pixmap, win, DefaultGC(dpy, screen_num), 0, 0, width, height, 0, 0);
                XSync(dpy, False);
            }
        }

        while(XPending(dpy))
        {
            XNextEvent(dpy, &ev);
            switch(ev.type)
            {
                case KeymapNotify:
                {
                    XRefreshKeyboardMapping(&ev.xmapping);
                }
                break;
                case FocusIn:
                {
                    isForeground = true;
                }
                break;
                case FocusOut:
                {
                    isForeground = false;
                }
                break;
                case ConfigureNotify:
                {
                    if (width != ev.xconfigure.width || height != ev.xconfigure.height)
                    {
                        width = ev.xconfigure.width;
                        height = ev.xconfigure.height;

                        terminate_video_capture(video_capture);
                        initialize_video_capture(videowidth, videoheight);

                        //free(videodata); -> destroyimage takes care of this
                        XFreePixmap(dpy, pixmap);
                        pixmap = XCreatePixmap(dpy, win, width, height, 24);
                        XDestroyImage(img);
                        videodata = (char*)malloc(width*height*4);
                        img = XCreateImage(dpy, visual, DefaultDepth(dpy,screen_num), ZPixmap, 0, videodata, width, height, 32, 0);
                    }
                }
                break;
                case ClientMessage:
                {
                    appDone = true;
                }
                break;
                case DestroyNotify:
                {
                    XCloseDisplay(dpy);
                }
                break;
            }
        }
    };

    pthread_join(inputCaptureThread, nullptr);
#else

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL2: %s\n", SDL_GetError());
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow("tinysys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

	SDL_Event ev;
	do
	{
		if (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				s_alive = false;
		}
	} while(s_alive);

	SDL_DestroyWindow(window);
#endif

    serial.Close();
    printf("remote connection terminated\n");

#if defined(CAT_LINUX)
    terminate_video_capture(video_capture);
#else
	// TODO: Windows and MacOS
#endif

	return 0;
}
