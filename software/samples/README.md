This folder contains the samples for tinysys

# Building samples

To build all of the samples, use the following command in this directory:

```
make
```

This will produce .elf binaries in each of the sample folders, which you can then copy onto an sdcard and run on the device.

If you wish to build just one, change to the desired folder and use the same make command.

# Running samples on the emulator

This initially requires you to build the emulator binaries.

First, check out the [README.md](..\emulator\README.md) for build instructions.

Once the emulator is built, next step is to drop your binary into the [sdcard](..\emulator\sdcard\README.md) folder and then start the [emulator.bat](..\emulator\emulator.bat) file.

The emulator will do its best to match the hardware, except the ESP32 device which is not currently emulated.

# Running samples on the actual device

Once the dev environment is set up and riscvtool has been compiled, first switch to the desired directory on the device using:

```
cd somedirectory
```

then use the following command to send files across to the actual device:

```
On windows:
riscvtool -sendfile myprogram.elf

On Linux and MacOS:
./riscvtool -sendfile myprogram.elf

Alternatively, you might want to provide a COM port name:
riscvtool -sendfile \\.\COM6 myprogram.elf
```

followed by the name of your binary + Enter:

```
myprogram
```

to execute the binary.

The file transfer code will split the file into chunks, lz4 pack them, and send them across serial connection to be reconstructed into a file on the device.
