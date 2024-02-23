// needs:
// libx11-dev
// libv4l-dev

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

static uint8_t masktable[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

static uint8_t keycodetoscancode[256] =
{
//  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x29, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, // 0
0x24, 0x25, 0x26, 0x27, 0x2d, 0x2e, 0x2a, 0x2b, 0x14, 0x1A, 0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, // 1
0x12, 0x13, 0x2f, 0x30, 0X28, 0xe0, 0x04, 0x16, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x33, // 2
0x34, 0x35, 0xe1, 0x31, 0x1d, 0x1b, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0xe5,    0, // 3
0xe2, 0x2c,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 4
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 5
    0,    0,    0,    0,    0,    0,    0,    0,    0, 0xe4,    0,    0, 0xe6,    0,    0, 0x52, // 6
    0, 0x50, 0x4f,    0, 0x51,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x03, // 7 // 7F -> Ctrl+C (0x03)
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 8
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 9
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // A
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // B
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // C
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // D
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // E
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // F
};

#define clamp(x) (x>255?255:(x<0?0:x))
#define YUV2RO(C, D, E) clamp((298 * (C) + 409 * (E) + 128) >> 8)
#define YUV2GO(C, D, E) clamp((298 * (C) - 100 * (D) - 208 * (E) + 128) >> 8)
#define YUV2BO(C, D, E) clamp((298 * (C) + 516 * (D) + 128) >> 8)
uint32_t YUVtoRGBX32(int y, int u, int v)
{
    y -= 16; u -= 128; v -= 128;
    return (YUV2RO(y,u,v) & 0xff)<<16 | (YUV2GO(y,u,v) & 0xff)<<8 | (YUV2BO(y,u,v) & 0xff) | 0xFF000000;
}

int main(int argc, char **argv)
{
    Display* dpy = XOpenDisplay(NULL);
    int width = 640;
    int height = 480;

    if (!dpy)
    {
        printf("Cannot open display\n");
        return -1;
    }

    unsigned char keys_old[32];
    unsigned char keys_new[32];

    printf("Keyserver\nEND: quit keyserver\nPAUSE: quit remote process\n");

    // Serial
    int serial_port = open("/dev/ttyUSB0", O_RDWR); // TODO: move to command line option
    if (serial_port <0 )
    {
        printf("cannot open /dev/ttyUSB0\n");
        return -1;
    }

    struct termios tty;
    if(tcgetattr(serial_port, &tty) == 0)
    {
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
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200); // or only cfsetspeed(&tty, B115200);

        if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
        {
            printf("serial port error\n");
            return -1;
        }
    }
    else
    {
        printf("serial port error\n");
        return -1;
    }

    // Window
    int screen_num = DefaultScreen(dpy);
    Visual *visual = DefaultVisual(dpy, screen_num);
    unsigned long background = WhitePixel(dpy, screen_num);
    unsigned long border = BlackPixel(dpy, screen_num);

    char *videodata = (char*)malloc(width*height*4);
    XImage *img = XCreateImage(dpy, visual, DefaultDepth(dpy,screen_num), ZPixmap, 0, videodata, width, height, 32, 0);

    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0,0, width, height, 2, border, background);

    XSelectInput(dpy, win, ButtonPressMask|StructureNotifyMask|KeyPressMask|KeyReleaseMask|KeymapStateMask);
    XMapWindow(dpy, win);

    uint8_t isdown = 1;
    uint8_t isup = 2;
    uint8_t startToken = ':';
    KeyCode lshift = XKeysymToKeycode( dpy, XK_Shift_L );
    KeyCode rshift = XKeysymToKeycode( dpy, XK_Shift_R );
    KeyCode lalt = XKeysymToKeycode( dpy, XK_Alt_L );
    KeyCode ralt = XKeysymToKeycode( dpy, XK_Alt_R );
    KeyCode lctrl = XKeysymToKeycode( dpy, XK_Control_L );
    KeyCode rctrl = XKeysymToKeycode( dpy, XK_Control_R );

    memset(keys_old, 0, 32);
    memset(keys_new, 0, 32);

    // Video capture
    int video_capture = open("/dev/video0", O_RDWR);
    if (video_capture < 0)
    {
        printf("cannot open /dev/video0\n");
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
        return 1;
    }
    char* buffer = (char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, video_capture, queryBuffer.m.offset);
    memset(buffer, 0, queryBuffer.length);

    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    int type = bufferinfo.type;
    if(ioctl(video_capture, VIDIOC_STREAMON, &type) < 0)
    {
        perror("could not start streaming, VIDIOC_STREAMON");
        return -1;
    }

    XEvent ev;
    while(1)
    {
        while(XPending(dpy))
        {
            XNextEvent(dpy, &ev);
            switch(ev.type)
            {
                case KeymapNotify:
                    XRefreshKeyboardMapping(&ev.xmapping);
                break;
                case ConfigureNotify:
                {
                    if (width != ev.xconfigure.width || height != ev.xconfigure.height)
                    {
                        width = ev.xconfigure.width;
                        height = ev.xconfigure.height;
                    }
                }
                break;
                case DestroyNotify:
                {
                    XCloseDisplay(dpy);
                }
                break;
            }
        }

        // Non-event
        XQueryKeymap( dpy, (char*)keys_new );

        uint8_t dummy;
        for (uint32_t code = 0; code < 256; code++)
        {
            uint8_t currdown = keys_new[code>>3] & masktable[code&7];
            uint8_t prevdown = keys_old[code>>3] & masktable[code&7];
            uint8_t scancode = keycodetoscancode[code];

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
                write(serial_port, outdata, 4);
                read(serial_port, &dummy, 1);
            }
        }
        memcpy(keys_old, keys_new, 32);

        // Video
        if(ioctl(video_capture, VIDIOC_QBUF, &bufferinfo) < 0){
            perror("could not queue buffer, VIDIOC_QBUF");
            return -1;
        }

        if(ioctl(video_capture, VIDIOC_DQBUF, &bufferinfo) < 0)
        {
            perror("could not dequeue the buffer, VIDIOC_DQBUF");
            return -1;
        }

        if (bufferinfo.bytesused)
        {
            uint32_t *outputimage = (uint32_t*)img->data;
            for(int y=0; y<height; ++y)
            {
                int pY = y*width;
                for(int x=0;x<width/2;++x)
                {
                    int idx0 = x*2+pY;
                    int idx1 = x*2+pY;
                    // current pixel luma, shared blue
                    uint8_t Y0 = buffer[idx1*2+0]; // Y0
                    uint8_t Cb = buffer[idx1*2+1]; // Cb
                    // adjacent pixel luma, shared red
                    uint8_t Y1 = buffer[(idx1+1)*2+0]; // Y1
                    uint8_t Cr = buffer[(idx1+1)*2+1]; // Cr

                    // Convert the pair to RGB
                    uint32_t A = YUVtoRGBX32(Y0, Cb, Cr);
                    uint32_t B = YUVtoRGBX32(Y1, Cb, Cr);

                    // Two successive pixels with individual luma and shared CbCr
                    outputimage[idx0] = A;
                    outputimage[idx0+1] = B;
                }
            }
            XPutImage(dpy,win,DefaultGC(dpy,screen_num),img,0,0,0,0,width,height);
            XSync(dpy, False);
        }
    };

    close(serial_port);

    if(ioctl(video_capture, VIDIOC_STREAMOFF, &type) < 0)
    {
        perror("could not end streaming, VIDIOC_STREAMOFF");
        return -1;
    }
    close(video_capture);
}
