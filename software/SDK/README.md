# SDK Documentation

## Base System
Base system routines provide useful facilities such as reading the clock, or setting up critical sections.

Please see [BASE](base.md) for documentation about base system utilities.

## Audio Processing Unit
The audio processing unit is responsible for generating stereo signals at various output frequencies.

Please see [APU](apu.md) for documentation about audio.

## Multitasking
The task library helps start and stop tasks that can run on any hardware thread in the system.

Please see [TASK](task.md) for documentation about multitasking and the CPU cores.

## Video Processing Unit
The video processing unit controls video scan out from selected system memory location as well as controlling color depth and video dimensions.

Please see [VPU](vpu.md) for documentation about video output control.

## Debug LEDs
The debug LEDs provide an easy means to debug code when there are no alternatives, or to show status.

Please see [LED](led.md) for documentation about debug LEDs.

## Emulator
The emulator tries to provide as close an experience to the real hardware as possible, including a virtual SDCard interface to make development easy.

Please see [EMULATOR](emulator.md) for documentation about debug LEDs.

## Low level hardware overview
The hardware of tinysys consists of an FPGA board that carries some custom devices and two RISC-V cores. The FPGA board lives on a custom board that contains some external peripheral chips that interface the board to the outside world, such as video, audio and USB connectivity.

Please see [HARDWARE](hardware.md) for brief information about the internal devices.
