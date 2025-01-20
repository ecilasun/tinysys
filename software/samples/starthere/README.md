# Welcome to your project!

First step is to create a copy of this folder under the samples directory, and rename it to match your project title.

After this, you'll want to rename the output file names from `starthere` to your project name in the Makefile.

Once that is out of the way, we can begin!

# What is this tinysys?

Tinysys is a simple computer with two 32 bit RISC-V cores. It has no branch prediction, no pipelining and will run at approximately 27MIPS, and has a coremark score of 0.54 to 0.6 depending on the configuration

However slow it might seem, it makes up for what it lacks with a few hardware devices implemented in the FPGA fabric. To see the speed at which memory can be moved around, run the `memtest` sample for CPU side memory clear speed.

There's a mini OS (~52Kbytes) provided in ROM which contains the necessary routines to help with file access, memory allocation and other facilities.

The SDK directory contains quite a few modules and combined with the samples you should have a good idea of what the device can achieve.

# Compiling and running your project

Tinysys is a very simple machine so it requires little more than a few files to do something useful. Observe the layout of the provided Makefile and make necessary changes as you see fit. Only thing to watch out for are the machine architecture and cache size/alignment flags. Rest is up to you!

To run the resulting .elf file, you have two choices.

You can use the riscvtool to send the compiled binary to the machine using base64 encoded format. First, got the directory where your binary is, and use the following command:

```
On windows:
riscvtool -sendfile myfile.elf
On Linux and MacOS:
./riscvtool -sendfile myfile.elf
```

This will start a progress bar and copy the file to the target device, however since base64 encoding expands a file, it will not complete this transfer quickly. The file is written to the SDCard at the current directory.

Alternatively, you'll need to copy your binary onto a FAT32 formatted SDCard, insert the card into the machine.

After either method, type the name of your binary (without the .elf extension) from a connected terminal or keyboard, and the binary will start executing.

If you wish to stop execution, use CTRL+C to break out of the application.

If your system appears to be unresponsive, use the tilde key (~) to break the execution from a connected terminal (this will not work with a keyboard, in that case assuming you're close to the device, please use the reset button on the board instead)

NOTE: The gdb stub has been temporarily removed due space requirements, but it will be added in the future for gdb support. Until then, printf and LEDs are your friend!
