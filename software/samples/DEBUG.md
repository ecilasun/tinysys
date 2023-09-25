The first step is to start the program you'd wish to debug on the remote device:

```
On the command line of the device, or via serial connection, type:
hellodebugger
```

To start the debugger via command line, use:

```
riscv64-unknown-elf-gdb -b 115200 hellodebugger.elf
To run without an initial executable:
riscv64-unknown-elf-gdb -b 115200
```

After this step, at command prompt of GDB, use one of the following based on your platform (please remember to replace the ttyUSB or COM port name to the one you've set up on the development machine):

```
Windows:
target extended-remote //.//COM9
Linux:
target extended-remote /dev/ttyUSB1
```

At this point you can upload the binary that you'd like to debug, if it hasn't alredy been loaded
```
load ../../samples/hellodebugger/hellodebugger.elf
```

After the debugger connects to the remote machine, we'll need to select the currently running program which is usually thread #3 using the following command:
```
T 3
```

An alternative method is to use gdbgui from the riscvtool/tinysys/samples/** directory:

```
../../../gdbgui/gdbgui.pex --gdb-cmd="riscv64-unknown-elf-gdb -b 115200 doom.elf"
```

NOTE: If things do not go as planned, use the following before the 'remote' command to observe the packet traffic between GDB and the debug stub:
```
set debug remote 1
```


NOTES:
Initial packet to receive seems to be:
```
$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+#df
```