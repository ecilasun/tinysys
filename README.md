# tinysys

# Overview of the processing units

## CPU
Central processing unit. Handles instruction execution.

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