# Emulator

The emulator for tinysys is aimed to be fast instead of clock-accurate.

It implements functionality of all onboard peripherals except the Xtensa CPU of the ESP32-S3 module. This unit is wired to reset the CPUs and read/write from/to the terminal of the host PC over a UART connection. It isn't required for the emulator, as it pulls its own keyboard input from a fake UART device.

Please see [EMULATOR](../emulator/README.md) for more information about how the emulator can be used for development.

### Back to [SDK Documentation](README.md)