# Welcome to your project!

First step is to create a copy of this folder under the samples directory, and rename it to match your project title.

After this, you'll want to rename the output file names from `starthere` to your project name in the Makefile.

Once that is out of the way, we can begin!

# What is this tinysys?

Tinysys is a simple computer with two 32 bit RISC-V cores. It has no branch prediction, no pipelining and will run at approximately 27MIPS, and has a coremark score of 0.54 to 0.6 depending on the configuration

However slow it might seem, it makes up for what it lacks with a few hardware devices. There is a DMA unit that can move 16 byte aligned memory blocks with optional zero-masking, USB host and peripheral interfaces to talk to a keyboard/joystick or the host PC, and has access to 256Mbytes of DDR3 memory. To see the speed at which memory can be moved around, compile and run the `dmatest 1` command on the device CLI, or the `memtest` sample for CPU side memory access speed.

There's a mini OS (~56Kbytes) provided in ROM which contains the necessary routines to help with file access, memory allocation and other facilities.

The SDK directory contains quite a few modules and combined with the samples you should have a good idea of what the device can achieve.

# Compiling and running your project

Tinysys is a very simple machine so it requires little more than a few files to do something useful. Observe the layout of the provided Makefile and make necessary changes as you see fit. Only thing to watch out for are the machine architecture and cache size/alignment flags. Rest is up to you!

To run the resulting .elf file, you'll need to copy it to a FAT32 formatted SDCard, insert the card into the machine, and type its name (without the .elf extension) + Enter. That should run your code, and unless it's infinitely looping, should drop you back to the command line of tinysys. If not, simply hit CTRL+C to break execution and you'll be back to the CLI.

NOTE: The gdb stub has been temporarily removed due space requirements, but it will be added in the future for gdb support. Until then, printf and LEDs are your friend!
