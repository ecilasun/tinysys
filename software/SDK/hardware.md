# Hardware overview
Even though tinysys looks like a functioning computer, it is in fact implemented on a prototyping board and is written entirely in SystemVerilog language.

The SV description of each hardware unit is then converted to a list of connected gates, which forms the gateware.

The nature of FPGAs do not allow for gigahertz level clocks on the low end consumer / automative versions, but what they lack in speed they make up for in versatility.

This allows for some really specialized hardware that can access data via very wide busses, and true parallel device execution.

### The CPUs
RISC V is the CPU of choice for tinysys, though it could easily have been OpenRISC or a combination of other CPU designs to form a heterogenous system.

For easer of development, the choice was made to implement the open RISC V standard, as there are mature enough compilers and tools today that allow for portin complex applications including Linux to this architecture.

There are multiple CPUs on board (inside the FPGA) of tinysys. Technically, they're termed 'hardware threads' and the term HART is used for short.

Each HART contains its own dedicated data (D$) and code (I$) cache, which are 32 and 16 Kbytes in size respectively.

There is a floating point unit (without the classification and exception mechanisms implemented) on the device per HART, which handle single precision floating point math operations. The floating point registers share the same space as integer registers, which allows for a very simple task switching approach, where one does not need to spend clock cycles to store/restore FPU registers or their corresponding FPU control CSR.

### Memory
The memory controller of tinysys currently uses the Xilinx DDR controller IP, which is set up to work in 128 bit burst mode.

The cache controllers aim to return data and code fast and not access memory as much as possible, which helps execution speeds as expected.

The only downside is that, due to the nature of how this cache works, data is not delivered in a single clock cycle and might take from 3 to 4 cycles. This is still faster than hitting memory, populating the cache, and returning data form the now-correctly-populated cache.

### Memory layout

The memory address space has been divided into RAM and devices, at address 0x80000000.

All devices are placed at addresses 0x8... onwards, and system RAM starts at address 0x0... This is the opposite of what most embedded CPUs do, but for ease of software development tinysys has adopted this layout.

The device memory space is split into 64Kbyte regions. Not all devices have access to the full 64Kbyte memory space, except the scratchpad memory at address 0x80000000.

Here is a list of all devices, their starting addresses, and their purpose. Please note that this is for illustration purposes only; it is highly recommended to use the corresponding device functions from the SDK instead of these numbers so that the code is compatible with future OS/device changes.

> **Warning:** Accessing any memory regions outside the following, or those reserved by the OS will most likely hang the hardware and require a reset. Try to adhere to memory allocation functions and utilities provided in the SDK files to get valid addresses when possible to avoid this scenario.

```
	SYSTEM_RAM  0x00000000 - 0x0FFFFFFF OS RAM: 256Mbytes of system memory, further subdivided into dedicated regions by the OS
	DEVICE_SPAD 0x80000000 - 0x8000FFFF Scratchpad: 64Kbyte word addressible, uncached memory, accessible from all cores coherently
	DEVICE_LEDS 0x80010000 - 0x8001FFFF Debug LEDs: 4 LEDs that are on board the device, suitable for simple debugging purposes
	DEVICE_VPUC 0x80020000 - 0x8002FFFF VPU fifo: Command fifo for the video processing unit (VPU)
	DEVICE_SPIC 0x80030000 - 0x8003FFFF SPI fifo: Command fifo for the SPI device tied to the SDCard controller
	DEVICE_DEV0 0x80040000 - 0x8004FFFF Unused - reserved for future use / do not access
	DEVICE_USBA 0x80050000 - 0x8005FFFF USB fifo: Command fifo for the SPI device tied to the USB host chip
	DEVICE_APUC 0x80060000 - 0x8006FFFF APU fifo: Command fifo for the audio unit, used to control audio playback
	DEVICE_MAIL 0x80070000 - 0x8007FFFF Mailbox: A 4Kbyte word addressible uncached memory region used to store task state for the scheduler
	DEVICE_UART 0x80080000 - 0x8008FFFF UART: I/O port used to read from and write data to the UART device tied to the ESP32-C6 module
	DEVICE_CSR0 0x80090000 - 0x8009FFFF CSR0: The 4Kbyte memory mapped CSR file for hardware thread zero
	DEVICE_CSR1 0x800A0000 - 0x800AFFFF CSR1: The 4Kbyte memory mapped CSR file for hardware thread one
	DEVICE_DEV1 0x800B0000 - 0x800BFFFF Unused - reserved for future use / do not access
	DEVICE_DEV2 0x800C0000 - 0x800CFFFF Unused - reserved for future use / do not access
	DEVICE_DEV3 0x800D0000 - 0x800DFFFF Unused - reserved for future use / do not access
	DEVICE_DEV4 0x800E0000 - 0x800EFFFF Unused - reserved for future use / do not access
	DEVICE_DEV5 0x800F0000 - 0x800FFFFF Unused - reserved for future use / do not access
```

### Memory arbitration
There is a memory arbiter implemented in the FPGA fabric, which is the backbone of all I/O coming from the CPU or other devices.

This unit can deal memory access using round-robin arbitration where each device gets a fair share of the memory bus.

The CPUs, audio DMA and the VPU scanline buffer are the main clients of this bus. All of the internal hardware also live on the bus as uncached memory mapped devices, such as the command FIFOs for most devices.

When video scan-out is enabled, the CPUs will share memory access with the VPU, and the VPU will take up most of the read bandwidth while it's populating its scan-out cache. This means, for memory I/O intensive algorithms, one might benefit from turning off video scan-out slightly, for example when startup up a game that needs to generate some procedural data.

### Hardware interrupts
Most of the devices, such as the USB module or the SDCard module, will generate EI (external interrupts) when certain conditions occur, such as inserting or removing an SDCard, or when a horizontal scanline is encountered, depending on setup.

In addition to the peripheral interrupts, there is a timer interrupt that will periodically fire. This interrupt type is how the task system implements its scheduler. Each task is given a 'timeout' value after which they will break execution and switch to the next, or if so desired, preemt execution by calling `TaskYield()`. In all cases, the timer control registers are utilized to trigger the scheduler interrupt on each HART so they can manage their task pools simultaneously.

Last one is the software interrupt facility, which currently tracks illegal instruction execution.

All of these interrupts are in use by the OS for specific work, however there is nothing in the hardware design that enforces OS level control over these. Implementing a custom rom.bin and placing it into the /sys/bin/ folder at the SDCard root will allow for overriding everything, including the interrupt mechanisms, that tinysys offers.

### ESP32 communications helper
There is an ESP32 chip on board (which also uses a RISC V core) that is hard wired to act as the USB(or Bluetooth)-serial converter. The serial input received feeds into the serial input buffer by the OS via interrupts, which in turn feeds the keyboard entry buffer. This means one can send text commands over USB or Bluetooth of a cell phone to the device while the OS terminal is alive.

### Back to [SDK Documentation](README.md)