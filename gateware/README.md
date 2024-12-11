# Latest Gateware Build is: r1.1D
# Latest ESP32 ROM is: r1.1D
# Latest Board version is: 2F

# Gateware binary
The gateware binary contains latest version of the device.
There is no need for a ROM file.<br>
If you have an older one, you can simply delete it, or use a `rom.bin` file with this version or higher.

# Current device architecture
This is the current architecture to use with gcc13 and higher: `rv32im_zicsr_zifencei_zfinx`

# ROM history
r1.01: Initial release version<br>
r1.03: Improvements for dual core boot stability, fixed audio hardware playback rate issue<br>
r1.06: Minor improvements to inner ROM loop and some other bug fixes<br>
r1.07: DMA hardware change in preparation for misaligned DMA support<br>
r1.08: Stability fixes<br>
r1.09: Added misaligned DMA support and removed ESP32/FPGA GPIO bridge device (to be re-done later), also added hblank interrupt mechanism and remote reboot ability (~ character over UART)<br>
r1.0A: Added 64Kbytes of scratchpad RAM, shared across all cores, WNS relaxed to leave room for any future bug fixes<br>
r1.0B: Minor device cleanup, reduced scratchpad to 16Kbytes, bumped mailbox to 16Kbytes<br>
r1.0C: Added a third CPU, CLI improvement to ROM and other minor fixes<br>
r1.1D: Improvements to reboot / task system and more ROM code cleanup, also updated file upload routines<br>

# Device gateware

This folder contains the gateware that will reprogram the FPGA board on tinysys to a functioning RISC-V CPU with several other peripherals.

The intended device is the QMTECH A7200T core board, which is a blank FPGA board with nothing but an XCA7200T AMD/Xilinx part and a 256Mbytes DDR3 memory. The firmare also needs at least a revision 2E tinysys board to be attached to it to provide USB serial / HID / audio and video connections as well as the sdcard port.

Programmin the gateware onto the device requires a JTAG adapter (such as the HS2 REVA from Digilent or a Xilinx compatible 6-pin adapter), a PC, and Vivado 2024.1 installed.

NOTE: Make sure to never attempt this unless absolutely necessary and a hardware bug needs addressing / fixing or a new feature is added to the device!

# Programming

First we need to attach the JTAG adapter to the PC, and make sure the Vivado drivers are installed for it.

For this step, attach a USB cable to the HS2(revA or later), attach one end to your PC, then attach the adapter end to the 6 pin connector on the already powered-up FPGA board, as seen below:

![JTAG Connector](./JTAG.png "JTAG Connector")

When properly connected, the text on the HS2 should be facing away from the board (easiest way to make sure it's correct is to find the GND marker on the 6 pin connector on the FPGA board, and line up the GND marker on the HS2 to that)

Start Vivado 2024.1 (or newer version) and on the first page, select Open Hardware Manager.
You should then see a note that says 'No hardware target is open.' Click the `Open Target` button next to it and select the `Auto Connect` option.

This should bring up a view which shows a 'xc7a200t' device listed. Right click on it and select `Add Configuration Memory Device`
In the search box, type `is25lp128f-spi-x1_x2_x4` and accept the changes.

Now this memory device should be listed unde the xc7a200t device, most likely below a XADC device.

Right click on the is25 and select `Program Memory Configuration Device`, then browse to and select the `tophat.bin` file in the gateware folder for the Configuration File.

Click OK and wait until the device is programmed (which will take a minute or so)

Once the progamming is complete, the device will not function until it's reset. Press the program/reset button (the small white button closer to the power connector barrel) and the device will reboot itself with the new gateware.

NOTE: the ROM image used by an updated gateware may differ from the one on the sdcard, make sure to either delete the one on sdcard or update it to a version newer or same as the gateware one, to ensure correct operation.
