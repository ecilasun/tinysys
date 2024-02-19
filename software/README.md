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
- This will return a line similar to: "generic converter now attached to ttyUSB0"
- You can now start PuTTY or another serial terminal program, connect to ttyUSB* port provided above, and send commands and receive responses from the device.

NOTE: Run the following to diagnose if the device is indeed present and visible by the system:
```
lsusb
```
You should see "FFFF:0001 ENGIN tinysys usb serial" as part of the output

NOTE:
If for some reason the path to generic usb serial driver isn't there try the following:
```
sudo modprobe usbserial vendor=0xFFFF product=0x0001
logout/login
then check the path to see if it's there
```

# Adding tinysys as a generic serial device over USB - Windows

This is rather simpler on windows. All one needs to do is plug in the device and find it in the list under the Device Manager control panel. It should be displayed as 'tinysys USB serial'
Once you locate the device, right click and select Update Driver, then Browse my computer for drivers, and finally Let me pick from a list of available drivers on my computer.

What we're looking for is now the generic usb serial driver, which is listed under USB Serial Device / Microsoft / USB Serial Device. Select this, then hit Next. If any warning pop up, accept and you should have the device ready. This will be confirmed by a USB device plug sound, and you should now see a USB Serial Device (COMx) listed under Ports, where x is usually COM9 on Windows 11

NOTE: The serial device will be only accepting 115200 bauds,8bits,1stopbit,noparity settings and may not function with any other, even though you change the settings on the control panel. This is because the device side driver is not going to cope with traffic throttling to keep it small and simple.


# Uploading executables to the device - WiP

To upload files to the device, first make sure you've got serial communication working. This can be tested by setting up the serial driver for the board as described above, then starting a terminal connected to that port set to 115200 baud, 8 bits, 1 stop bit, no parity.
After the terminal connects to the board, you can try typing 'help' and the display attached to the board should display the help text, while also echoing back what you're typing to the terminal.

Once the above is confirmed working, uploading binaries is straighforward. For example on Windows, simply run a command similar to the following, with the port and file names set to the ones on your local device:

```
build\release\riscvtool.exe test.elf -sendfile \\.\COM9
```

on Linux the comand would be similar to:
```
./build/release/riscvtool test.elf -sendfile /dev/ttyUSB0
```

During the upload process the device will show a message to indicate upload status.

NOTE: Please make sure the file name is not decorated, as it'll be sent as-is and the device will try to create the file using any path names, which might fail. This will be fixed in an upcoming revision.

# More details on RISC-V compiler toolchain

NOTE: If the RISC-V compiler binaries (riscv64-unknown-elf-gcc or riscv64-unknown-elf-g++) are missing from your system, please follow the instructions at https://github.com/riscv/riscv-gnu-toolchain
It is advised to build the rv32i / rv32im / rv32imc libraries

If you want to work on Windows and don't want to compile the toolchain, you can use the following link and download the latest riscv-v-gcc installer executable (risc-v-gcc10.1.0.exe at the time of writing this)

https://gnutoolchains.com/risc-v/

This will place all the toolchain files under C:\SysGCC\risc-v by default and make sure to have that path added to your %PATH% by using the 'Add binary directory to %PATH%' option.

NOTE: One thing worth mentioning is that at this time Win32 compiler toolchain was not aware of march=rv32imc_zicsr_zifencei so please set the makefile to use march=rv32imc in sample projects and ROM projects instead.

# Other notes

Be advised that all software has been tested with and geared to use a gcc-riscv 32bit environment under Linux, therefore any compiler changes and/or bit width changes are untested at this moment, as well as some glitches / mismatches that might occur under Windows setups.

Copyright 2024 Engin Cilasun
Please see license.txt file for details
