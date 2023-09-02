# expansionboardA

This repository contains KiCAD 7 project files for an expansion module built for QMTECH A7 200T core board.
Required libraries (models and symbols) are included with the project.

<img src="render.png" width="512px">
<img src="render2.png" width="512px">

# Bill of materials

See expansionboardA.cv file for the list of components required for a fully populated board. Please note that some are optional (such as the USB controller chips) depending on your application.

# Default SoC, ROM image and samples

For hardware, visit https://github.com/ecilasun/tinysys. The project files require a recent version of Vivado suite to work.

For software, visit https://github.com/ecilasun/riscvtool and check the tinysys directory for the default ROM image and samples. The ROM image comes in two parts: the actual executable ROM and another one (fetch) that contains instruction sequences to inject on hardware/software interrupts. Samples demonstrate majority of the capabilities of the tinysys SoC itself.

# Device manual:
- Populating the board
    - USB-C power and serial
    - SD card reader
    - Video output
    - USB-A host
	- SRAM
    - Board carrier headers
    - Fan connector
    - Audio circuit
    - QMTECH A7-200T core board
- Installing tinysys SoC bitware
- Compiling and running demo software on the board
    - Compiler toolchain installation
    - Hello, board!
- Operating system how-tos
    - List of syscalls
    - Memory layout
    - Operating principles
    - Debugging
	- Replacement ROM on SD card
- Device manuals:
    - Device memory map
    - Raw audio
    - OPL2 audio
    - GPU
    - DMA
	- Rasterizer
    - Temperature sensor
    - Interrupt handling
    - Task system
	- Implementing a custom ROM
