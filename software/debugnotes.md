To start risc32-unknown-elf-gdb set up for our target architecture:
```
C:\riscv32-gcc\bin\riscv32-unknown-elf-gdb.exe led.elf
set debug remote 1
set architecture riscv:rv32
```

To debug on the the actual hardware:
```
target remote \\.\COM6
```

To debug on the emulator:
```
target remote localhost:1234
```
