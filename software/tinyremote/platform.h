#pragma once

#if defined(CAT_LINUX)
// needs:
// libx11-dev
// libv4l-dev
// libsdl2-dev
// libsdl2-ttf-dev
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <termios.h>
#include <pthread.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "SDL.h"
#include "SDL_ttf.h"
//char commdevicename[512] = "/dev/ttyUSB0";
char commdevicename[512] = "/dev/ttyACM0";
char capturedevicename[512] = "/dev/video0";
#endif

#if defined(CAT_WINDOWS)
#include <windows.h>
#include <conio.h>
#include "SDL.h"
#include "SDL_ttf.h"
char commdevicename[512] = "\\\\.\\COM6";
char capturedevicename[512] = "\\\\.\\VIDEO0"; // ???
#endif
