To debug via command line, use:

```
riscv64-unknown-elf-gdb -b 115200 helloworld.elf
```

An alternative method is to use gdbgui from the riscvtool/tinysys/samples/** directory:

```
../../../gdbgui/gdbgui.pex --gdb-cmd="riscv64-unknown-elf-gdb -b 115200 doom.elf"
At command prompt of GDB use:
target remote /dev/ttyUSB1
```
