# Firmware for the on-board ESP32-S3-WROOM-1 module

This firmware allows the ESP32 to act as a communications processor for tinysys.

The default firmware behavior is to pass through serial data coming from USB to the UART pins connected to the FPGA and also provide the same function in the opposite direction.

In additon, sending the character '~' with no following character within a certain amount of time will pull a reset line to reboot the CPUs on tinysys, therefore the ESP32 can be used as a recovery device for code that gets stuck when developing remotely (when no button access is possible)
