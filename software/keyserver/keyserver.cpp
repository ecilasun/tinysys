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

int log2quick(unsigned char c)
{
    switch(c)
    {
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        case 16: return 4;
        case 32: return 5;
        case 64: return 6;
        case 128: return 7;
    }
    return 0;
}

int main(int argc, char **argv)
{
    Display* dpy = XOpenDisplay(NULL);

    unsigned char keys_old[32];
    unsigned char keys_new[32];

    memset(keys_old, 0, 32);
    memset(keys_new, 0, 32);

    printf("Keyserver\nPlease hit END key to quit\n");

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
        tty.c_cc[VMIN] = 0;
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
    int isdown = 1;
    int isup = 0;
    do {
        usleep(5000);
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

        for (uint32_t i=0;i<32;++i)
        {
            unsigned int keycode = i*8 + log2quick(keys_new[i] ^ keys_old[i]);
            char ks = XKeycodeToKeysym(dpy, keycode, 0);

            int currdown = keys_new[ keycode>>3 ] & ( 1<<(keycode&7) );
            int prevdown = keys_old[ keycode>>3 ] & ( 1<<(keycode&7) );
            if (currdown && (!prevdown))
            {
               write(serial_port, &isdown, 1);
               write(serial_port, &ks, 1);
               printf("keydown: %d\n", ks);
            }
            if ((!currdown) && prevdown)
            {
               write(serial_port, &isup, 1);
               write(serial_port, &ks, 1);
               printf("keyup: %d\n", ks);
            }
        }

        // End key down
        KeyCode kc2 = XKeysymToKeycode( dpy, XK_End );
        bShiftPressed = !!( keys_new[ kc2>>3 ] & ( 1<<(kc2&7) ) );

        memcpy(keys_old, keys_new, 32);
    } while(!bShiftPressed);

    close(serial_port);

    XCloseDisplay(dpy);
}