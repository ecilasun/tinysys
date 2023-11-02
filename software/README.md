This folder contains the riscvtool helper utility, the ROM image, sample code and the SDK for tinysys

# Prerequisites

First, you'll need the risc-v toolchain from https://github.com/riscv/riscv-gnu-toolchain or its precompiled version (make sure to have support for 32bit ELF and rv32imc variant)
You'll also need a working Python so that the WAF build system can build the riscvtool.

There's a convenience script in this directory that will automate this task for you. Simply run:

```
./buildrisctoolchain.sh
```

Please see the last section for risc-v compiler toolchain on Windows.

# Build riscvtool

For this purpose, you may first want to open the 'software' directory in Visual Studio Code so that the root path is now /tinysys/software.
Before you can build the riscvtool itself, use the ctrl+shift+b shortcut in Visual Studio Code and select 'configure'. After this initial step you can use the same shortcut and select 'build'.
Alternatively, you can use the following command sequences:
```
# To configure (required only once):
python waf --out='build/release' configure

# To build after code changes:
python waf build -v

# To 'clean' the output binaries
python waf clean
```
PS: The build process for riscvtool is the same on Linux and Windows.

# Build samples (and optionally the ROM image)

To build the ROM file and the samples, either switch to the 'ROMs/boot' or 'samples' directory and use this on the command line:

```
make
```

Note that some samples have a different path layout, for instance DOOM makefile is placed in 'samples/doom/src/riscv' or 'samples/imguidemo' which is where you'd run make from.

Ordinarily you don't need to build ROM images. But if for some reason you'd like to do that, please note that you'll get two files generated: rom.mem and rom.bin.
rom.mem file is the one to use in the hardware design (to be copied over the contents of the existing romimage.mem file) and the .bin file is the same binary in overlay format, which can be copied to the root folder of the SDCard and will replace the device ROM at device boot time.

The rom image generated from the fetch folder is a set of instructions to be used by the fetch unit when it encounters an interrupt or other event, and you should not have to change it in most cases (and I can't recommend that you do). If you really need to, please make sure to go over the hardware fetch unit and update the device to match the contents of the ROM image, with correct instruction offsets and sizes. Currently there's no automation to provide you with a correct offset & length table to aid in this, but is something that's planned for the future.

# Adding tinysys as a generic serial device over USB - Linux

Until an actual driver is built for this device, it can happily work as a generic serial device using the built-in generic USB serial driver. The device has no VID/PID assigned to it at this point, and currently uses VID:FFFF and PID:0001 for testing purposes, thus Linux won't pick a driver for it automatically.

To add it as a generic USB serial devie, follow this process:

- Unplug tinysys board's USB cable
- Type following in a terminal
```
sudo nano /sys/bus/usb-serial/drivers/generic/new_id
```
- Enter & save the following VID/PID pair onto the new_id file
```
FFFF 0001
```
- Re-plug tinysys board's USB cable. Now that the device has been associated with the generic serial driver, it should enumerate automatically.
- At this point, to see which ttyUSB device we're assigned to, type the following
```
 sudo dmesg
```
- You can now start PuTTY or another serial terminal program, connect to ttyUSB port provided above, and send commands and receive responses from the device.

# Adding tinysys as a generic serial device over USB - Windows

TBD: We use generic USB serial driver on Windows, provide instructions

# Uploading executables to the device - WiP

TBD: Normally device uses SDCard, USB bulk storage is possible to implement, investigate

# Debugging with GDB - WiP

One prerequisite of being able to debug code on tinysys is to install an UART-serial adapter, at least for the time being. In the future the USB serial device will expose debug functionality alongside serial data facilities which will make the UART port redundant.

You can attach with GDB to debug your code over the UART port using:

```
NOTE: The speed of the internal UART is fixed at 115200 bauds and won't change
riscv64-unknown-elf-gdb -b 115200 --tui samples/mysample.elf
Make sure to use the UART port here and not the USB serial port
target remote /dev/ttyUSB1
```

This will break into the currently executing program. Use 'c' command to resume execution, or Ctrl+C to break at an arbitrary breakpoint. You can also set breakpoints when the program is paused by using 'b DrawConsole' for instance. On resume with 'c' the program will be stopped at the new breakpoint address.
 
Please note that this is an entirely software based feature and its usage pattern / implementation may change over time.

NOTE: One very useful visual too to aid in debugging is gdbgui which you can find here:
https://www.gdbgui.com/gettingstarted/

# More details on RISC-V compiler toolchain

NOTE: If the RISC-V compiler binaries (riscv64-unknown-elf-gcc or riscv64-unknown-elf-g++) are missing from your system, please follow the instructions at https://github.com/riscv/riscv-gnu-toolchain
It is advised to build the rv32i / rv32im / rv32imc libraries

If you want to work on Windows and don't want to compile the toolchain, you can use the following link and download the latest riscv-v-gcc installer executable (risc-v-gcc10.1.0.exe at the time of writing this)

https://gnutoolchains.com/risc-v/

This will place all the toolchain files under C:\SysGCC\risc-v by default and make sure to have that path added to your %PATH% by using the 'Add binary directory to %PATH%' option.

NOTE: One thing worth mentioning is that at this time Win32 compiler toolchain was not aware of march=rv32imc_zicsr_zifencei so please set the makefile to use march=rv32imc in sample projects and ROM projects instead.

# Other notes

Be advised that all software has been tested with and geared to use a gcc-riscv 32bit environment under Linux, therefore any compiler changes and/or bit width changes are untested at this moment, as well as some glitches / mismatches that might occur under Windows setups.

Copyright 2023 Engin Cilasun
Please see license.txt file for details
