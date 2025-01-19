This folder contains the riscvtool helper utility, the ROM image, sample code and the SDK for tinysys.
> Please note that precompiled binaries for riscvtool have been provided for Windows and Linux, however if the supplied binaries do not run for you or if you wish to build a MacOS version, please rebuild them from source as explained below.

# Prerequisites

If you wish to actually run things on a real piece of hardware:
- If you did not acquire / build one, please find the KiCad files for the board and go through the paces to build one
- Next step is to acquire (or use an existing) QMTECH A7200T core board (I'll build my own single board computer later, promise!)

If you wish to work on software:
- To be able to compile your own executables, or ROM images, you'll need a prebuilt gcc risc-v toolchain from the following link.
<br>(NOTE: The default compiler setup for samples and ROM images uses gcc, so please make sure to get that package)
<br>https://www.embecosm.com/resources/tool-chain-downloads/
- In case you need it, you can find 'make' binaries here:
<br>https://gnuwin32.sourceforge.net/packages/make.htm
<br>(Make sure to also install libintl3 from same source and make sure it's on the path as well as the make.exe from the bin package)
- If you wish to build your own ROM images, you'll also need a working python3 install to make sure the waf build system functions as intended.
- To be able to compile riscvtool, you'll also need a working cpp compiler targeting the platform you're going to build on (windows/linux/macos etc)
- I advise using Visual Studio Code, but it's optional (all step below try to show both VSC and command line usage)
- To compile the ROM image and program the ESP32-C6 device (our communication module) for the first time, you'll need to install ESP-IDF addon for your Visual Studio Code
<br>See https://github.com/espressif/vscode-esp-idf-extension/releases/ for latest release

Or if you wish to simply browse / copy code or hardware details, feel free to do so, and don't forget to credit me and other people for their respective work!

# Build riscvtool (optional, required to build a new OS ROM)

If you wish to build new ROM images to experiment with, you'll need riscvtool which will generate the ROM binary format required by tinysys. You'll need a working python3 install for the following steps to work.

To build in Visual Studio Code:
- Open the 'software' directory in Visual Studio Code so that the root path is now /tinysys/software
- Use the ctrl+shift+b shortcut in Visual Studio Code and select 'configure'
- You can now ctrl+shift+b again select 'build', which will compile and generate the riscvtool for your platform

Alternatively, you can use the following command line sequences:
```
# To configure (required only once):
python waf --out='build/release' configure

# To build after code changes:
python waf build -v

# To 'clean' the output binaries
python waf clean
```
PS: The build process for riscvtool is the same on Linux and Windows.

# Building ELF executables

Please see [README.md](./samples/README.md) file in the samples directory

# Building ROM images

NOTE: Ordinarily you don't need to (and should not) build ROM images, there is one built into the gateware provided in the /gateware directory.

This is similar to building the samples, simply switch to 'ROMs/boot' directory and type:

```
make
```

This will give you one .mem file, and .elf file and a .bin file. The .elf file is not useful as a ROM image in this case, so you can ignore it.

The .bin image goes onto the sdcard, inside the '/boot' directory at the root of your FAT32 formatted sdcard. This image will be loaded by the resident ROM image and the device will chain to the code you've placed in this file, which is essentially how an OS update works for tinysys.

The .mem image is the actual physical ROM version of the same binary. If you don't plan to use an sdcard, the contents of the .mem file should be pasted over the 'Memory File/romimage.mem' file you can find in the project view. After replacing it, don't forget to 'Reset Runs' / 'Generate Bitstream' to generate your new gateware. (I recommend doing development using the .bin image and only replace the .mem once you're sure it's working to avoid lengthy development times, gateware usually takes about 20 minutes each run on a modetately fast desktop PC)

# Programming the ESP32 communications firmware

Tinysys uses an ESP32-C6 to communucate with the outside world. For this to function properly, the ROM image for the ESP chip has to be built. You need first plug in the previously-unprogrammed tinysys device, then open 'software/ESP32S3/defaultrom' in Visual Studio Code, and Hit the ESP-IDF 'Build, Flash and Monitor' button (looks like a little flame) to build and deploy it to the device.

# Default SDCard layout

Tinysys OS expects to find certain files in certain paths to function as expected. For instance, the 'rom.bin' file is expected to be in the '/boot' directory at the root of SDCard. Tinysys OS supports commands that can be executed from any directory (sort of a fixed $PATH if you will), and this is always expected to be 'sys/bin' Anything in that directory can be run from anywhere. Therefore it's a good place to put OS utilities or any used code that you might want to run from any path.

Therefore the default SDCard layout is:

```
sd:/sys/bin/... <- utility binaries, accessible from anywhere
sd:/sys/bin/rom.bin <- ROM overlay is always located here
```

# Creating your own project

To make your own project, the recommended way is to copy the 'starthere' sample directory to a new one and make your changes as needed. In that directory you will find a Makefile, and upload shell script/batch file to help get you started.

Please refer to the [README.md](./samples/starthere/README.md) file in `starthere` folder for more assistance.

# Uploading your files to tinysys

There is an executable on the default disk image of tinysys, called 'recv.elf', under the sys\bin directory. The riscvtool binary can start this application remotely and start sending a binary file to it.

The syntax for sending a file is:
```
On windows:
riscvtool -sendfile myfile.elf
On Linux and MacOS:
./riscvtool -sendfile myfile.elf
```

The recv.elf executable will show an upload progress, and make sure the file arrives safely before writing it to the sdcard, as well as report any errors that might occur during the transfer.

# Other notes

NOTE: All software has been tested with and geared to use a gcc-riscv 32bit environment under Windows and Linux, but as always one might fall behind the other at times.

# Acknowledgements

I'd like to thank Wade Brainerd for help in getting this device tested thoroughly in a live environment, and for the awesome saturated float to integer conversion instruction.

Also thanks goes to Ben Stragnell for great advice and pointers in how audio hardware should work.

Further thanks goes to everyone who's build something on this machine at least once to put it through its paces.

Happy hacking!

Copyright 2025 Engin Cilasun

Please see respective license.txt files for individual license terms
