# What is this?

This is a work-in-progress system emulator for tinysys

# Details

Here's a list of features implemented so far

- Dual CPU cores work (with FPU minus custom saturate instruction)
- Interrupts work
- UART is tied to console (I/O)
- Video output works
- There's a system RAM debug view (1 pixel per Kbyte)
- CSRs and their special purpose registers work
- MAIL device works
- LED device exists but no graphical representation present

And here's the list of what's missing

- The whole ESP32-C6 module emulation is missing
- APU
- SPI sdcard emulator
- DMA
- XADC
- GPIO
- USB-A

Please note that this is not a clock precise emulation and instructions will almost always retire in 1 CPU clock. Also all devices clock at the same speed so there's going to be some mismatch there as well. Since the aim of this emulator is to provide a testbed for ROM or user software in the absence of real hardware, it should be close enough to get development rolling.

Wish list

- A GDB proxy in the emulator to debug running risc-v code
