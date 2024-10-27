# SDK Documentation

## Audio Processing Unit
The audio processing unit is responsible for generating stereo signals at various output frequencies.

Please see [APU](apu.md) for documentation about audio.

## Direct Memory Access Unit
The direct memory access unit is used to move blocks of memory faster than the CPU can deliver.

Please see [DMA](dma.md) for documentation about memory block moves.

## Multitasking
The task library helps start and stop tasks that can run on either of the two CPUs in the system.

Please see [TASK](task.md) for documentation about multitasking and the two CPU cores.

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
