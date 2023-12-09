# tinysys

# System specifications

- RISC-V based CPU (full architecture name with extensions: rv32_imc_Zicsr_Zifencei_Zicntr)
- 32 GRPs x 32 bits
- 4096 CSRs x 32 bits (some registers reserved for CPU)
- Integer multiply / divide
- Software, hardware and timer interrupts
- 16Kbytes of direct mapped instruction cache (256 lines x 64 bytes)
- 32Kbytes of direct mapped data cache (512 lines x 64 bytes)
- 128 bit AXI4 bus with 32 bit address line
- Memory arbiter for 8 on-board devices
- Memory mapped external hardware
- USB host and peripheral controller chips and ports
- 16 bit stereo audio output chip (24 bit native in reality)
- HDMI port for video
- SDCard port for file I/O
- 4 debug LEDs
- System enumerates as a serial device when plugged into a developer PC
- Supports one USB peripheral for input (keyboard / mouse / gamepad)
- Soft-reset button
- Preemptive multitasking OS, with file I/O and basic memory allocator via syscalls
- GDB support over serial port
- Optional ROM overlay can be loaded from SDCard

# Overview of the processing units

## CPU
Based on 32 bit RISC-V ISA. Implements base integer instruction set, required cache operations (I$ and D$) and a large CSR file.
The core currently has an average instruction retirement rate of 6 CPI (clocks per instruction) and runs at 166.66667MHZ, which
is also the speed of the AXI4 bus on which all peripherals and memory reside.

### Fetch/Decode/IRQ
This unit reads an instruction at PC, decodes it and outputs it (together with its PC) to instruction output FIFO (IFIFO). If it's an interrupt entry/exit or some other special instruction (for instance I$ flush) then it is handled entirely within the fetch unit. This unit is also responsible for inserting pre/post interrupt code from an internal ROM at interrupt or exception time. If a branch instruction is encountered, fetch unit will drop into an idle state and wait for the execute unit to resolve the target branch address.

### Execute/Load/Store
This unit will read an instruction from the IFIFO if available, load registers with values, execute (ALU/BLU/CSR/SYS) and decide on new branch target if there's a branch involved. After deciding on the branch address, fetch unit is notified so it can resume instruction fetches. Where possible, load or store operations will overlap with fetch and execution.

## GPU
Graphics processing unit. Handles scan-out of various video sizes (320x240 and 640x480) and bit depths (8bpp index color or 16bpp RGB color)

## APU
Audio processing unit. Handles mixing the OPL2/RAW audio outputs, and also manages 44/22/11KHz playback and buffer handling of RAW audio.

## RPU
Raster processing unit. Used to rasterize primitives directly to memory. Rasterized output is in a tiled format which requires decoding to use.

## DMA
Direct memory access unit. Used to copy blocks of memory within memory address space, and won't DMA between or from other devices.

# Overview of the bus

## AXI4 bus
The AXI4 bus, running at 166.67MHz, connects all of the processing units to memory or devices. In the case of memory, access is cached per perhipheral if needed. Device access is always uncached.

# Custom instructions

## Store mask (smask)

This instruction sets the following 32bit write to memory to use a 4 bit byte mask so that only the bytes corresponding to high bits of the mask end up being written to memory.
As soon as any following store instruction gets executed, subsequent stores revert back to regular behavior.

```smask addressregister, maskimmed, sourceregister```
