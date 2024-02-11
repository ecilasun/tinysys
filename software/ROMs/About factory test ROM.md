Factory test

This folder contains the firmware to load into the factory test module.
Its sole purpose is to use the onboard debug port and check to see if the
board powers up correctly, and executes code to emit a string on
the UART port.

Please do not use as a regular ROM image for the final device, it will not work and will break the system!

Thanks,
Engin Cilasun
