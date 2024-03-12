# tinysys

# System specifications

- RISC-V based CPU (full architecture name with extensions: rv32im_zicsr_zifencei_zfinx)
- Supports instruction fence and data cache flush/invalidate operations
- Single precision FPU
- Float and integer GPRs use same register space (zfinx extension)
- 32 GRPs x 32 bits
- 4096 CSRs x 32 bits (some registers reserved for CPU and some are immutable)
- DMA with optional zero-masking for 16 byte aligned memory copies
- Integer multiply / divide
- Software, hardware and timer interrupts
- 16Kbytes of direct mapped instruction cache (256 lines x 64 bytes)
- 32Kbytes of direct mapped data cache (512 lines x 64 bytes)
- 128 bit AXI4 bus with 32 bit address line
- Memory arbiter for on-board devices
- Memory mapped external hardware
- USB host and peripheral controller chips and ports
- 16 bit stereo audio output chip (24 bit native in reality)
- DVI 1.0 compatible video output (12 bits per pixel, RGB or paletted modes)
- SDCard port for file I/O
- 4 debug LEDs
- System enumerates as a serial device when plugged into a developer PC
- Supports one USB peripheral for input (keyboard / mouse / gamepad)
- Custom preemptive multitasking OS, with file I/O and basic memory allocator via syscalls
- Optionally, a rom.bin image can be loaded from SDCard to replace the OS in ROM

# Overview of the processing units

## CPU
Based on 32 bit RISC-V ISA. Implements base integer instruction set, required cache operations (I$ and D$) and a large CSR file.
The core currently has an average instruction retirement rate of 6 CPI (clocks per instruction) and runs at 166.66667MHZ, which
is also the speed of the AXI4 bus on which all peripherals and memory reside.

### Fetch/Decode/IRQ
This unit reads an instruction at PC, decodes it and outputs it (together with its PC) to instruction output FIFO (IFIFO). If it's an interrupt entry/exit or some other special instruction (for instance I$ flush) then it is handled entirely within the fetch unit. This unit is also responsible for inserting pre/post interrupt code from an internal ROM at interrupt or exception time. If a branch instruction is encountered, fetch unit will drop into an idle state and wait for the execute unit to resolve the target branch address.

### Execute/Load/Store
This unit will read an instruction from the IFIFO if available, load registers with values, execute (ALU/BLU/CSR/SYS) and decide on new branch target if there's a branch involved. After deciding on the branch address, fetch unit is notified so it can resume instruction fetches. Where possible, load or store operations will overlap with fetch and execution.

## VPU
Video processing unit. Handles scan-out of various video sizes (320x240 and 640x480) and bit depths (8bpp index color or 16bpp RGB color)

## APU
Audio processing unit. Handles mixing the OPL2/RAW audio outputs, and also manages 44/22/11KHz stereo playback and buffer handling of RAW audio.

## DMA
Direct memory access unit. Used to copy blocks of memory within memory address space, and won't DMA between or from other devices. It can optionally ignore zeros on input data and won't write them to the output location (i.e. automatic masking)

## OPL2
This is a Yamaha OPL2 compatible device using an opensource implementation (mono output, 16bits)

# Overview of the bus

## AXI4 bus
The AXI4 bus, running at 166.67MHz, connects all of the processing units to memory or devices. In the case of memory, access is cached per perhipheral if needed. Device access is always uncached.

# Custom instructions

## Convert from float to 4 bit integer, saturated (FCVTSWU5SAT)

This instuction has been contributed by Wade Brainerd. It's very useful in converting floating point values to device specific 4 bit color values.

The following python script helps encode a hex representation for convenience

```
op = 0b1010011		# OP-FP (floating point operation)
rd = 0b01010		# destination register - a0 in this sample (a0 == x10)
rs1 = 0b01011		# source register - a1 in this sample (a1 == x11)
funct7 = 0b1100001	# funct5 F7_FCVTSWU5SAT - this is our new sub-instruction
inst = op | (rd << 7) | (rs1 << 15) | (funct7 << 25)

print(f"{inst:x}")
```

You can then use it in code as follows:

```
inline uint32_t ftoui4sat(float value)
{
  uint32_t retval;
  asm (
    "mv a1, %1;"
    ".word 0xc2058553;" // fcvtswu4sat a0, a1 // note A0==cpu.x10, A1==cpu.x11
    "mv %0, a0; "
    : "=r" (retval)
    : "r" (value)
    : "a0", "a1"
  );
  return retval;
}
```

P.S. Perhaps in the future it might be possible to convince gcc so that we won't have to add the extra mv instructions and directly let the actual registers known.
