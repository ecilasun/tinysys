To debug via command line, use:

```
riscv64-unknown-elf-gdb -b 115200 helloworld.elf
or
riscv64-unknown-elf-gdb -b 115200 --tui helloworld.elf
```

After this step, at command prompt of GDB, use one of the following based on your platform (please remember to replace the ttyUSB or COM port name to the one you've set up on the development machine):

```
Linux:
target remote /dev/ttyUSB1
Windows:
target remote //.//COM9
```

An alternative method is to use gdbgui from the riscvtool/tinysys/samples/** directory:

```
../../../gdbgui/gdbgui.pex --gdb-cmd="riscv64-unknown-elf-gdb -b 115200 doom.elf"
```

If things do not go as planned, use the following before the 'remote' command to observe the packet traffic between GDB and the debug stub:
```
set debug remote 1
```


NOTES:
Initial packet to receive seems to be:
```
$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+#df
```