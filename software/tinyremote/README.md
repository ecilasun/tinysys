# TinyRemote

Tinyremote can be used to connect to a tinysys board on a remote machine and control it while being able to see the video output.

Currently it only supports Linux and Windows, but future support for MacOS is also in the works.

Build prerequisites on Linux:

```
sudo apt-get install libx11-dev libv4l-dev libv4l-dev libsdl2-dev libsdl2-ttf-dev
```

Windows includes the required SDL headers and binaries in the 3rdparty folder.

For MacOS, SDL2 and SLD2_ttf have to be installed via homebrew and reside in the following paths at the time of writing:

```
/opt/homebrew/Cellar/sdl2/2.30.5/include/SDL2
/opt/homebrew/Cellar/sdl2/2.30.5/lib/

/opt/homebrew/Cellar/sdl2_ttf/2.22.0/include/SDL2
/opt/homebrew/Cellar/sdl2_ttf/2.22.0/lib/
```