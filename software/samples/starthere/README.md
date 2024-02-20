# Welcome to your project!

First step is to create a copy of this folder under the samples directory, and rename it to match your project title.

After this, you'll want to rename the output file names from `starthere` to your project name in the Makefile.

Once that is out of the way, we can begin!

# What is this tinysys?

Tinysys is a simple computer with a 32 bit RISC-V core. It has no branch prediction, no pipelining and will run at approximately 27MIPS, and has a coremark score of 0.54

However slow it might seem, it makes up for what it lacks with a few hardware devices. There is an OPL2 compatible audio layer, a DMA unit that can move 16 byte aligned memory blocks, USB host and peripheral interfaces to talk to a keyboard/joystick or the host PC, and has 256Mbytes of fast DDR3 memory. To see the speed at which memory can be moved around, compile and run the `dmatest 1` command on the device CLI.

There's also a mini OS (~47Kbytes) on ROM which contains the necessary routines to help with file access, memory allocation and other facilities.

The SDK directory contains quite a few modules and combined with the samples you should have pretty much anything covered.

If there are unanswered questions, see the Q/A section in this document.

# Compiling and running your project

To build your project, simply run `make` and under most circumstances there should be no issues. The device is know to be able to run anything from raw assembly to C++20 as long as the said project is encapsulated in a properly built riscv .elf file. Modify your Makefile copy as needed.

Once you're sure you have something that should work on the device, first step is to upload it to the device. To achieve this, depending on your platform run either of these:
```
Windows: upload.bat mybinary.elf
Linux: ./upload.sh mybinary.elf
```

This will trigger an upload process where first the binary will be copied to the device RAM (in 4Kbyte chunks, with 64bit checsum code accompanying each), then after all blocks arrive in RAM it will be dumped to the sdcard. This ensures that if during transfer there are any issues the existing binary on the sdcard won't be damaged.

To run your executable on the device, connect to the device over `putty` or use the local keyboard to type the binary file's name without the .elf extension.

# Q/A

Q: What feature level does the RISC-V processor contain?<br>
A: If you're familiar with RISC-V ISA, this one implements the `rv32imc_zicsr_zifencei` subset of instructions

Q: Where do I find details for programming on the RISC-V?<br>
A: [RISC-V Documentation On GitHub](https://github.com/riscv/riscv-isa-manual/releases/tag/Ratified-IMAFDQC) would be a good starting point for the current ISA documents. Make sure to use only the feature set mentioned above!

Q: Is there a memory map that I can check?<br>
A: Yes! Please see the SDK/basesystem.h file for a complete map of current memory locations used by the OS and device memory addresses.

Q: What happens if I crash the device in an unrecoverable way?<br>
A: This is being thought about but the best way for now is to reach someone with access to the USB cable attached to your device and unplug-replug it. That said, it should be hard to crash the device under normal circumstances, and CTRL+C from local keyboard or over serial terminal should put you back into the CLI.

Q: How do I reset the device?
A: Type `reboot` on the CLI and you should be back at the start (this will only reboot the ROM, keeps most memory contents and hardware states intact)

Q: The CLI text was always greeen now it's suddenly orange, what happened?
A: That means we've somehow lost the `rom.bin` file on the sdcard but not to worry, there's a copy in ROM!

Q: Is there any memory protection?
A: Think of this as very 1980s personal computer, there is absolutely nothing in the way of the programmer, which also means you could overwrite the ROM and cause some issues. Not to worry, a reboot will fix most anything!