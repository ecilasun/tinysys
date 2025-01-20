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

To test your app, make sure the emulator is not running, drop your binary into the sdcard folder, and start the editor.

Following that, you can then run it by typing the name of your executable, excluding the .elf extension.

P.S. The emulator has been tested on latest macOS and Windows 11, Linux builds might require some manual work.

# Details

Here's a list of features implemented so far

- Dual CPU cores work with FPU (custom float->sat instruction included)
- Interrupts work
- UART is tied to console (I/O)
- Video output works
- There's a system RAM debug view (1 pixel per Kbyte)
- CSRs and their special purpose registers work
- MAIL device works
- LED device exists but no graphical representation present
- APU works and plays sound via SDL
- XADC is a placeholder and will return some huge temperature
- SPI sdcard emulator fully works
- APU supports varying playback rates

And here's the list of what's missing

- ESP32-C6 module emulation is missing and won't be supported
- USB-A port emulation isn't supported

Please note that this is not a clock precise emulation and instructions will almost always retire in 1 CPU clock. Also all devices clock at the same speed so there's going to be some mismatch there as well. Since the aim of this emulator is to provide a testbed for ROM or user software in the absence of real hardware, it should be close enough to get development rolling.

Wish list

- A GDB proxy in the emulator to debug running risc-v code


### Back to [SDK Documentation](../SDK/README.md)