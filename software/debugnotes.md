To start risc32-unknown-elf-gdb set up for our target architecture:
```
C:\riscv32-gcc\bin\riscv32-unknown-elf-gdb.exe
set debug remote 1
set architecture riscv:rv32
```

To start debugging on the the actual hardware:
```
target remote \\.\COM6
```

To start debugging on the emulator:
```
target remote localhost:1234
```

Then load your code to debug and wait for the upload to finish:
```
load samples/led/led.elf
```

Alternatively you can place the set/target/load commands into a file (for example gdbcmd) and instead use:
```
C:\riscv32-gcc\bin\riscv32-unknown-elf-gdb.exe -x gbdcmd
```
to make it easier to start debugging. See this folder for a sample gdbcmd file you can use to add your own setup.

Now you can debug your program as usual with GDB
