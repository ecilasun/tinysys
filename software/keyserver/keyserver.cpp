// needs:
// libx11-dev

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <chrono>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

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

int main(int argc, char **argv)
{
    Display* dpy = XOpenDisplay(NULL);

    unsigned char keys_old[32];
    unsigned char keys_new[32];

    memset(keys_old, 0, 32);
    memset(keys_new, 0, 32);

    printf("Keyserver\nEND: quit keyserver\nPAUSE: quit remote process\n");

    int serial_port = open("/dev/ttyUSB0", O_RDWR);
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

    bool bShiftPressed = false;
    
    auto start = std::chrono::steady_clock::now();
    uint8_t isdown = 1;
    uint8_t isup = 2;
	uint8_t startToken = ':';
    KeyCode kc2 = XKeysymToKeycode( dpy, XK_End );
    KeyCode lshift = XKeysymToKeycode( dpy, XK_Shift_L );
    KeyCode rshift = XKeysymToKeycode( dpy, XK_Shift_R );
    KeyCode lalt = XKeysymToKeycode( dpy, XK_Alt_L );
    KeyCode ralt = XKeysymToKeycode( dpy, XK_Alt_R );
    KeyCode lctrl = XKeysymToKeycode( dpy, XK_Control_L );
    KeyCode rctrl = XKeysymToKeycode( dpy, XK_Control_R );
    do {
        //usleep(5000);
        XQueryKeymap( dpy, (char*)keys_new );

        // Re-trigger key state submit after 0.160 seconds
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        //int resubmit = 0;
        if(elapsed_seconds.count()>0.160)
        {
            start = end;
            //resubmit = 1;
        }

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
                printf("%.2X -> %.2X\n", code, scancode); // DEBUG
                uint8_t outdata[4];
                outdata[0] = startToken;
                outdata[1] = modifierstate;
                outdata[2] = keystate;
                outdata[3] = scancode;
                write(serial_port, outdata, 4);
                read(serial_port, &dummy, 1);
            }
        }

        // End key is down
        bShiftPressed = !!( keys_new[ kc2>>3 ] & ( 1<<(kc2&7) ) );

        memcpy(keys_old, keys_new, 32);
    } while(!bShiftPressed);

    close(serial_port);

    XCloseDisplay(dpy);
}