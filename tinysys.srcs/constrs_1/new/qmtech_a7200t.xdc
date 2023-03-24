## ------------------------------------------------------------------------------------------------------
## Constraints for the QMTECH A7200T board and the custom addon board 
## ------------------------------------------------------------------------------------------------------

## (C) 2023 Engin Cilasun
## Applies to expansion board version v0.9 through v0.9B
## Please do not change/remove the Clock Groups or False Paths regardless of the warnings during synth
## Also note that changing any of the pin positions will change the timing closure of the device
## due to changes in placing and routing

## ------------------------------------------------------------------------------------------------------
## Clocks
## ------------------------------------------------------------------------------------------------------

set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## ------------------------------------------------------------------------------------------------------
## Buttons on the FPGA board
## ------------------------------------------------------------------------------------------------------

set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

## ------------------------------------------------------------------------------------------------------
## LEDs on the FPGA board
## ------------------------------------------------------------------------------------------------------

set_property -dict {PACKAGE_PIN F3 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]
set_property -dict {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]

## ------------------------------------------------------------------------------------------------------
## UART Tx/Rx
## ------------------------------------------------------------------------------------------------------

## Sorted by UART module pin order
## VCC:     pin U4:3&4
## GND:     pin U4:1&2&5&6
## RXD_OUT: pin U4:7         [B1]
## TDX_IN:  pin U4:9         [C2]

set_property -dict {PACKAGE_PIN B1 IOSTANDARD LVCMOS33} [get_ports uart_rxd_out]
set_property -dict {PACKAGE_PIN C2 IOSTANDARD LVCMOS33} [get_ports uart_txd_in]

## ------------------------------------------------------------------------------------------------------
## Micro SD card
## ------------------------------------------------------------------------------------------------------

## Sorted by sdcard pin order
## SD     SPI
## DAT[2] -         pin U4:23  [M1] sdpin#1
## DAT[3] CSn       pin U4:21  [K2] sdpin#2
## CMD    MOSI 	    pin U4:19  [K1] sdpin#3
## VDD    VDD       VCC             sdpin#4
## CLK    SCLK      pin U4:17  [H2] sdpin#5
## VSS    VSS       GND             sdpin#6
## DAT[0] MISO      pin U4:15  [G1] sdpin#7
## DAT[1] -         pin U4:13  [E2] sdpin#8
## SWTCH            pin U4:11  [E1] sdpin#switch

## SPI mode - sorted by sdcard pin order
set_property -dict {PACKAGE_PIN K2 IOSTANDARD LVCMOS33} [get_ports spi_cs_n]
set_property -dict {PACKAGE_PIN K1 IOSTANDARD LVCMOS33} [get_ports spi_mosi]
set_property -dict {PACKAGE_PIN H2 IOSTANDARD LVCMOS33} [get_ports spi_clk]
set_property -dict {PACKAGE_PIN G1 IOSTANDARD LVCMOS33} [get_ports spi_miso]
## set_property -dict {PACKAGE_PIN E1 IOSTANDARD LVCMOS33} [get_ports spi_swtch]

## SD mode - sorted by sdcard pin order
## set_property -dict {PACKAGE_PIN M1 IOSTANDARD LVCMOS33} [get_ports {sd_dat[2]}]
## set_property -dict {PACKAGE_PIN K2 IOSTANDARD LVCMOS33} [get_ports {sd_dat[3]}]
## set_property -dict {PACKAGE_PIN K1 IOSTANDARD LVCMOS33} [get_ports sd_cmd]
## set_property -dict {PACKAGE_PIN H2 IOSTANDARD LVCMOS33} [get_ports sd_clk]
## set_property -dict {PACKAGE_PIN G1 IOSTANDARD LVCMOS33} [get_ports {sd_dat[0]}]
## set_property -dict {PACKAGE_PIN E2 IOSTANDARD LVCMOS33} [get_ports {sd_dat[1]}]
## set_property -dict {PACKAGE_PIN E1 IOSTANDARD LVCMOS33} [get_ports spi_swtch]

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
## ADC bus
## ------------------------------------------------------------------------------------------------------

## adclk  pin U4:18 [G2]
## addout pin U4:16 [F1]
## addin  pin U4:14 [D2]
## adcs   pin U4:12 [D1]

## set_property -dict {PACKAGE_PIN G2 IOSTANDARD TMDS_33} [get_ports adclk]
## set_property -dict {PACKAGE_PIN F1 IOSTANDARD TMDS_33} [get_ports addout]
## set_property -dict {PACKAGE_PIN D2 IOSTANDARD TMDS_33} [get_ports addin]
## set_property -dict {PACKAGE_PIN D1 IOSTANDARD TMDS_33} [get_ports adcs]

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

set_clock_groups -name asyncA -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name asyncB -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT2]]
set_clock_groups -name asyncC -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT3]]
set_clock_groups -name asyncD -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]]
set_clock_groups -name asyncE -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT5]]
set_clock_groups -name asyncF -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT6]]
set_clock_groups -name asyncG -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins socinstance/axi4ddr3sdraminst/ddr3instance/u_mig_7series_0_mig/u_ddr3_infrastructure/gen_mmcm.mmcm_i/CLKFBOUT]]

## ------------------------------------------------------------------------------------------------------
## False paths
## ------------------------------------------------------------------------------------------------------

## There is no path from GPU to I$, only appears so because we're connected to the same bus
set_false_path -from [get_pins {socinstance/GPU/m_axi\\.araddr_reg/CLK}] -to [get_pins socinstance/instructionfetch/instructioncacheinst/instructioncachectlinst/dout_reg*/CE]
