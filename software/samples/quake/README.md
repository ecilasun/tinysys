# Quake port for tinysys

Based on work from
https://github.com/sysprog21/quake-embedded

This one is easy to build, just use:

```
make
```

on command line from the root folder and you'll end up with a quake.elf binary.

Copy this file to your sdcard next to your ID1 folder (quake game files for version 1.09) and you should be good to go.

Alternative to copying to sdcard manually is to use riscvtool to upload directly to the board if it's connected via USB cable to your PC. Just use the following command from the directory you've build quake in, and wait until the upload is complete (you might need to disconnect an already-connected serial terminal first depending on your platform)

```
..\..\riscvtool -sendfile quake.elf
```

NOTE: Emulator version has a small glitch reading files, which will be fixed soon.
