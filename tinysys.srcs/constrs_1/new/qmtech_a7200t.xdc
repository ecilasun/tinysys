## ------------------------------------------------------------------------------------------------------
## Constraints for the QMTECH A7200T board and the custom addon board
## ------------------------------------------------------------------------------------------------------

## (C) 2023 Engin Cilasun
## Applies to expansion board ISSUE-1E/1F
## Please do not change/remove the Clock Groups or False Paths regardless of the warnings during synth
## Also note that changing any of the pin positions will change the timing closure of the device
## due to changes in placing and routing

## ------------------------------------------------------------------------------------------------------
## Clocks
## ------------------------------------------------------------------------------------------------------

set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## ------------------------------------------------------------------------------------------------------
## Buttons on the FPGA board - unused
## ------------------------------------------------------------------------------------------------------

## set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

## ------------------------------------------------------------------------------------------------------
## LEDs
## ------------------------------------------------------------------------------------------------------

## LED[0]: pin U2:[U21]
## LED[1]: pin U2:[V20]
## LED[2]: pin U2:[U18]
## LED[3]: pin U2:[V19]

set_property -dict {PACKAGE_PIN U21 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]
set_property -dict {PACKAGE_PIN V20 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]
set_property -dict {PACKAGE_PIN U18 IOSTANDARD LVCMOS33} [get_ports {leds[2]}]
set_property -dict {PACKAGE_PIN V19 IOSTANDARD LVCMOS33} [get_ports {leds[3]}]


## ------------------------------------------------------------------------------------------------------
## UART Tx/Rx debug port (tie to an external USB-UART cable or other device)
## ------------------------------------------------------------------------------------------------------

## RXD_OUT: pin U4:23         [M1]
## TDX_IN:  pin U4:25         [K4]

# set_property -dict {PACKAGE_PIN M1 IOSTANDARD LVCMOS33} [get_ports uart_rxd_out]
# set_property -dict {PACKAGE_PIN K4 IOSTANDARD LVCMOS33} [get_ports uart_txd_in]

## ------------------------------------------------------------------------------------------------------
## Micro SD card
## ------------------------------------------------------------------------------------------------------

## Sorted by sdcard pin order
## SD     SPI
## DAT[2] -         pin U2:58  [Y18]  sdpin#1
## DAT[3] CSn       pin U2:56  [AA19] sdpin#2
## CMD    MOSI 	    pin U2:54  [AA20] sdpin#3
## VDD    VDD       VCC               sdpin#4
## CLK    SCLK      pin U2:52  [AB21] sdpin#5
## VSS    VSS       GND               sdpin#6
## DAT[0] MISO      pin U2:50  [V18]  sdpin#7
## DAT[1] -         pin U2:48  [U17]  sdpin#8
## SWTCH            pin U2:46  [U20]  sdpin#switch

## SPI mode - sorted by sdcard pin order
set_property -dict {PACKAGE_PIN AA19 IOSTANDARD LVCMOS33} [get_ports sdcard_cs_n]
set_property -dict {PACKAGE_PIN AA20 IOSTANDARD LVCMOS33} [get_ports sdcard_mosi]
set_property -dict {PACKAGE_PIN AB21 IOSTANDARD LVCMOS33} [get_ports sdcard_clk]
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports sdcard_miso]
set_property -dict {PACKAGE_PIN U20 IOSTANDARD LVCMOS33} [get_ports sdcard_swtch]

## SD mode - sorted by sdcard pin order
## set_property -dict {PACKAGE_PIN U20 IOSTANDARD LVCMOS33} [get_ports {sd_dat[2]}]
## set_property -dict {PACKAGE_PIN U17 IOSTANDARD LVCMOS33} [get_ports {sd_dat[3]}]
## set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports sd_cmd]
## set_property -dict {PACKAGE_PIN AB21 IOSTANDARD LVCMOS33} [get_ports sd_clk]
## set_property -dict {PACKAGE_PIN AA20 IOSTANDARD LVCMOS33} [get_ports {sd_dat[0]}]
## set_property -dict {PACKAGE_PIN AA19 IOSTANDARD LVCMOS33} [get_ports {sd_dat[1]}]
## set_property -dict {PACKAGE_PIN Y18 IOSTANDARD LVCMOS33} [get_ports spi_swtch]

## ------------------------------------------------------------------------------------------------------
## DVI output over HDMI
## ------------------------------------------------------------------------------------------------------

## hdmi_tx_clk_p pin U4:41 [T1]
## hdmi_tx_clk_n pin U4:42 [U1]
## hdmi_tx_p[1]  pin U4:43 [W1]
## hdmi_tx_n[1]  pin U4:44 [Y1]
## hdmi_tx_p[2]  pin U4:45 [AA1]
## hdmi_tx_n[2]  pin U4:46 [AB1]
## hdmi_tx_p[0]  pin U4:47 [AB3]
## hdmi_tx_n[0]  pin U4:48 [AB2]

set_property -dict {PACKAGE_PIN T1 IOSTANDARD TMDS_33} [get_ports hdmi_tx_clk_p]
set_property -dict {PACKAGE_PIN U1 IOSTANDARD TMDS_33} [get_ports hdmi_tx_clk_n]
set_property -dict {PACKAGE_PIN W1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_p[1]}]
set_property -dict {PACKAGE_PIN Y1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_n[1]}]
set_property -dict {PACKAGE_PIN AA1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_p[2]}]
set_property -dict {PACKAGE_PIN AB1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_n[2]}]
set_property -dict {PACKAGE_PIN AB3 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_p[0]}]
set_property -dict {PACKAGE_PIN AB2 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_n[0]}]

## ------------------------------------------------------------------------------------------------------
## ADC bus - MCP3008 10 bit 8 channel analog-digital converter
## ------------------------------------------------------------------------------------------------------

## adcs   pin U2:38 [N17]
## addin  pin U2:40 [R18]
## addout pin U2:42 [Y21]
## adclk  pin U2:44 [T21]

set_property -dict {PACKAGE_PIN T21 IOSTANDARD LVCMOS33} [get_ports adclk]
set_property -dict {PACKAGE_PIN Y21 IOSTANDARD LVCMOS33} [get_ports addout]
set_property -dict {PACKAGE_PIN R18 IOSTANDARD LVCMOS33} [get_ports addin]
set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports adcs]

## ------------------------------------------------------------------------------------------------------
## I2C bus
## ------------------------------------------------------------------------------------------------------

## i2c_scl pin U2:26 [K13]
## i2c_sda pin U2:24 [J14]

## set_property -dict {PACKAGE_PIN K13 IOSTANDARD LVCMOS33} [get_ports i2c_scl]
## set_property -dict {PACKAGE_PIN J14 IOSTANDARD LVCMOS33} [get_ports i2c_sda]
## set_property PULLUP TRUE [get_ports i2c_scl]
## set_property PULLUP TRUE [get_ports i2c_sda]

## ------------------------------------------------------------------------------------------------------
## Audio output - CS4344-CZZR
## ------------------------------------------------------------------------------------------------------

## au_sdin  pin U2:29  [M20]
## au_sclk  pin U2:31  [M22]
## au_lrclk pin U2:33  [L15]
## au_mclk  pin U2:35  [N19]

set_property -dict {PACKAGE_PIN M20 IOSTANDARD LVCMOS33} [get_ports au_sdin]
set_property -dict {PACKAGE_PIN M22 IOSTANDARD LVCMOS33} [get_ports au_sclk]
set_property -dict {PACKAGE_PIN L15 IOSTANDARD LVCMOS33} [get_ports au_lrclk]
set_property -dict {PACKAGE_PIN N19 IOSTANDARD LVCMOS33} [get_ports au_mclk]

## ------------------------------------------------------------------------------------------------------
## USB-C - MAX3420EECJ over SPI interface, USB Device
## ------------------------------------------------------------------------------------------------------

## usbres_n   pin U2:14  [H20] -> hold low to reset the chip
## usbgpi     pin U2:13  [G20] -> GPI, suggested to keep grounded in MAX3420E errata
## usbbss_n   pin U2:12  [K21] -> slave select input, active low (required to send spi commands)
## usbclk     pin U2:11  [K22] -> up to 26MHz
## usbmosi    pin U2:10  [H17] -> spi mosi
## usbmiso    pin U2:9   [H18] -> spi miso
## usbint     pin U2:8   [J22] -> set the IE bit(bit#0) in the CPUCTL(r16) register to enable interrupts
## usbgpx     pin U2:7   [H22] -> operate / vbus_det / busact or start-of-frame indicator depending on gpxa/gpxb register contents

set_property -dict {PACKAGE_PIN H20 IOSTANDARD LVCMOS33} [get_ports usbc_resn]
# set_property -dict {PACKAGE_PIN G20 IOSTANDARD LVCMOS33} [get_ports usbc_gpi]
set_property -dict {PACKAGE_PIN K21 IOSTANDARD LVCMOS33} [get_ports usbc_ss_n]
set_property -dict {PACKAGE_PIN K22 IOSTANDARD LVCMOS33} [get_ports usbc_clk]
set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVCMOS33} [get_ports usbc_mosi]
set_property -dict {PACKAGE_PIN H18 IOSTANDARD LVCMOS33} [get_ports usbc_miso]
set_property -dict {PACKAGE_PIN J22 IOSTANDARD LVCMOS33} [get_ports usbc_int]
set_property -dict {PACKAGE_PIN H22 IOSTANDARD LVCMOS33} [get_ports usbc_gpx]

## MAX3420E's INT pin requires an external pullup resistor when in INTLEVEL mode
#set_property PULLUP true [get_ports usbc_int]

## ------------------------------------------------------------------------------------------------------
## USB-A - MAX3421EECJ over SPI interface, USB Host
## ------------------------------------------------------------------------------------------------------

## usbres_n   pin U2:51  [AB22] -> hold low to reset the chip
## usbbss_n   pin U2:60  [AA18] -> slave select input, active low (required to send spi commands)
## usbclk     pin U2:59  [AB18] -> up to 26MHz
## usbmosi    pin U2:55  [AB20] -> spi mosi
## usbmiso    pin U2:57  [Y19] -> spi miso
## usbint     pin U2:53  [AA21] -> set the IE bit(bit#0) in the CPUCTL(r16) register to enable interrupts

set_property -dict {PACKAGE_PIN AB22 IOSTANDARD LVCMOS33} [get_ports usba_resn]
set_property -dict {PACKAGE_PIN AA18 IOSTANDARD LVCMOS33} [get_ports usba_ss_n]
set_property -dict {PACKAGE_PIN AB18 IOSTANDARD LVCMOS33} [get_ports usba_clk]
set_property -dict {PACKAGE_PIN AB20 IOSTANDARD LVCMOS33} [get_ports usba_mosi]
set_property -dict {PACKAGE_PIN Y19  IOSTANDARD LVCMOS33} [get_ports usba_miso]
set_property -dict {PACKAGE_PIN AA21 IOSTANDARD LVCMOS33} [get_ports usba_int]
set_property PULLUP true [get_ports usba_int]

## ------------------------------------------------------------------------------------------------------
## NMI switch
## ------------------------------------------------------------------------------------------------------

## sysresetn     pin U4:59  [AA8]

set_property -dict {PACKAGE_PIN AA8 IOSTANDARD LVCMOS33} [get_ports sysresetn]

## ------------------------------------------------------------------------------------------------------
## DDR3 SDRAM (MT41K128M16XX-15E)
## ------------------------------------------------------------------------------------------------------

## width: 16, period: 2500, mask: 1
## set_property {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports init_calib_complete]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_p[0]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_n[0]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_p[1]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_n[1]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[0]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[1]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[2]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[3]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[4]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[5]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[6]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[7]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[8]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[9]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[10]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[11]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[12]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[13]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[14]}]
set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[15]}]

## ------------------------------------------------------------------------------------------------------
## Timing
## ------------------------------------------------------------------------------------------------------

##set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets {clockandresetinst/sys_clock_i}]
set_false_path -to [get_ports {leds[*]}]

## ------------------------------------------------------------------------------------------------------
## Programming
## ------------------------------------------------------------------------------------------------------

## Important: Make sure to use bitstream compression to avoid excessively long boot times on the board

set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]

## ------------------------------------------------------------------------------------------------------
## Clock groups
## ------------------------------------------------------------------------------------------------------

## NOTE: aclk (CLKOUT0) is never related to any device clocks and always crosses using a FIFO

set_clock_groups -name asyncA -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name asyncB -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT2]]
set_clock_groups -name asyncC -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT3]]
set_clock_groups -name asyncD -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]]
set_clock_groups -name asyncE -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT5]]
set_clock_groups -name asyncF -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT6]]
set_clock_groups -name asyncG -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins socinstance/axi4ddr3sdraminst/ddr3instance/u_mig_7series_0_mig/u_ddr3_infrastructure/gen_mmcm.mmcm_i/CLKFBOUT]]
set_clock_groups -name asyncI -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT0]]
set_clock_groups -name asyncJ -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT6]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT0]]

## ------------------------------------------------------------------------------------------------------
## False paths
## ------------------------------------------------------------------------------------------------------

## There is no path from GPU to I$, only appears so because we're connected to the same bus
## set_false_path -from [get_pins {socinstance/GPU/m_axi\\.araddr_reg/CLK}] -to [get_pins socinstance/fetchdecodeinst/instructioncacheinst/instructioncachectlinst/dout_reg*/CE]

## Human input
## set_input_delay -clock [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] 0.000 [get_ports sysresetn]
## set_input_delay -clock [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] 0.000 [get_ports sdcard_swtch]
