# What is this?

This is an emulator for the tinysys project

# Prerequisites
To build, please make sure you have at least version 2.3 of the following libraries available:
- libsdl2-dev
- libsdl2-ttf-dev

# Usage
To build the project, make sure you have C++ VC compiler tools installed, use the following sequence:
```
python3 waf configure -obuild/release
python3 waf build
```

Alternatively, you can use Visual Studio to build the emulator, using the emulator.sln solution file in the emulator/ folder

This will copy the emulator executable to the bin directory of project root folder, which you can then run using:
```
emulator.bat
or
./emulator.sh
```
depending on your OS.

The emulator will then scan the *sdcard* directory for executable files and generate a fake SDCard layout in memory which the emulator can access as if it's a real device.

To test your app, make sure the emulator is not running, drop your binary into the sdcard folder, and then start the emulator.

Following that, you can then run it by typing the name of your executable, excluding the .elf extension.

P.S. The emulator has been tested on latest macOS and Windows 11, Linux builds might require some manual work.

# Details

Here's a list of features implemented so far

- Dual CPU cores work with FPU (custom float->sat instruction included)
- Interrupts work
- UART is tied to console (I/O)
- Video output works
- There's a CPU stats overlay: update wscript to include the CPU_STATS define and rebuild if you wish to use it
- CSRs and their special purpose registers work
- MAIL device works
- LED device work with graphical representation present
- APU works and plays sound via SDL (including all playback frequencies supported by the real hardware)
- SPI sdcard emulator fully works
- GDB stub is working to a certain point, needs better breakpoint handling and single step support

And here's the list of what's missing

- ESP32-S3 module emulation is missing and won't be supported (don't want to deal with Xtensa emulation at this point)

Please note that this is not a clock precise emulation and instructions will almost always retire in 1 CPU clock. Also all devices clock at the same speed so there's going to be some mismatch there as well. Since the aim of this emulator is to provide a testbed for ROM or user software in the absence of real hardware, it should be close enough to get development rolling.

Wish list

- TODO


### Back to [SDK Documentation](../SDK/README.md)