# Emulator

The emulator for tinysys is aimed to be fast instead of clock-accurate.

It implements functionality of all onboard peripherals except the ESP32-C6 module (even though it's also a RISC-V CPU!). This unit is wired to read keyboard input, reset the CPUs and write to the terminal of the host PC over a UART connection and isn't required for the emulator, as it pulls its own keyboard input from a fake UART device.

Please see [EMULATOR](../emulator/README.md) for more information about how the emulator can be used for development.

### Back to [SDK Documentation](README.md)