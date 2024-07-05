# What is this?

This is a work-in-progress system emulator for tinysys

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
- DMA works including masked mode
- XADC is a placeholder and will return some huge temperature
- SPI sdcard emulator works, minor glitch in disk writes

And here's the list of what's missing

- ESP32-C6 module emulation is missing and won't be supported
- APU needs to support varying playback rates
- GPIO isn't supported (as that requires emulation of ESP32-C6)
- USB-A isn't supported

Please note that this is not a clock precise emulation and instructions will almost always retire in 1 CPU clock. Also all devices clock at the same speed so there's going to be some mismatch there as well. Since the aim of this emulator is to provide a testbed for ROM or user software in the absence of real hardware, it should be close enough to get development rolling.

Wish list

- A GDB proxy in the emulator to debug running risc-v code
