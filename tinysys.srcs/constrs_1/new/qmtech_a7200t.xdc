## ------------------------------------------------------------------------------------------------------
## Constraints for the QMTECH A7200T board and the custom addon board
## ------------------------------------------------------------------------------------------------------

## (C) 2023 Engin Cilasun
## Applies to expansion board ISSUE-2D/2E/2F
## Please do not change/remove the Clock Groups or False Paths regardless of the warnings during synth
## Also note that changing any of the pin positions will change the timing closure of the device
## due to changes in placing and routing

## ------------------------------------------------------------------------------------------------------
## Clocks
## ------------------------------------------------------------------------------------------------------

set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## Audio clock @44.100KHz (/512 of clkaudio at 22591KHz)
create_generated_clock -name audiosampleclk -source [get_pins clockandresetinst/peripheralclkinst/clkaudio] -divide_by 512 -add -master_clock clkaudio_peripheralclocks [get_pins {socinstance/APU/counterinst/P[8]}]

## ------------------------------------------------------------------------------------------------------
## Buttons on the FPGA board - unused
## ------------------------------------------------------------------------------------------------------

## set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

## ------------------------------------------------------------------------------------------------------
## Debug LEDs
## ------------------------------------------------------------------------------------------------------

## 2D
## set_property -dict {PACKAGE_PIN N18 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]
## set_property -dict {PACKAGE_PIN L14 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]
## set_property -dict {PACKAGE_PIN N22 IOSTANDARD LVCMOS33} [get_ports {leds[2]}]
## set_property -dict {PACKAGE_PIN N20 IOSTANDARD LVCMOS33} [get_ports {leds[3]}]

## 2E/2F
set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports {leds[5]}]
set_property -dict {PACKAGE_PIN R18 IOSTANDARD LVCMOS33} [get_ports {leds[4]}]
set_property -dict {PACKAGE_PIN Y21 IOSTANDARD LVCMOS33} [get_ports {leds[3]}]
set_property -dict {PACKAGE_PIN T21 IOSTANDARD LVCMOS33} [get_ports {leds[2]}]
set_property -dict {PACKAGE_PIN N18 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]
set_property -dict {PACKAGE_PIN L14 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]

## ------------------------------------------------------------------------------------------------------
## UART Tx/Rx debug port (tie to an external USB-UART cable or other device)
## ------------------------------------------------------------------------------------------------------

## set_property -dict {PACKAGE_PIN T21 IOSTANDARD LVCMOS33} [get_ports debugrx]
## set_property -dict {PACKAGE_PIN Y21 IOSTANDARD LVCMOS33} [get_ports debugtx]

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

## 2D/2E/2F
## SPI mode - sorted by sdcard pin order
set_property -dict {PACKAGE_PIN AA19 IOSTANDARD LVCMOS33} [get_ports sdcard_cs_n]
set_property -dict {PACKAGE_PIN AA20 IOSTANDARD LVCMOS33} [get_ports sdcard_mosi]
set_property -dict {PACKAGE_PIN AB21 IOSTANDARD LVCMOS33} [get_ports sdcard_clk]
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports sdcard_miso]
set_property -dict {PACKAGE_PIN U20 IOSTANDARD LVCMOS33} [get_ports sdcard_swtch]
set_property PULLTYPE PULLUP [get_ports sdcard_swtch]

## SD mode - sorted by sdcard pin order
## set_property -dict {PACKAGE_PIN U20 IOSTANDARD LVCMOS33} [get_ports {sd_dat[2]}]
## set_property -dict {PACKAGE_PIN U17 IOSTANDARD LVCMOS33} [get_ports {sd_dat[3]}]
## set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports sd_cmd]
## set_property -dict {PACKAGE_PIN AB21 IOSTANDARD LVCMOS33} [get_ports sd_clk]
## set_property -dict {PACKAGE_PIN AA20 IOSTANDARD LVCMOS33} [get_ports {sd_dat[0]}]
## set_property -dict {PACKAGE_PIN AA19 IOSTANDARD LVCMOS33} [get_ports {sd_dat[1]}]
## set_property -dict {PACKAGE_PIN Y18 IOSTANDARD LVCMOS33} [get_ports spi_swtch]

## ------------------------------------------------------------------------------------------------------
## 24bpp (8:8:8) Video output - on-chip HDMI
## ------------------------------------------------------------------------------------------------------

set_property -dict {PACKAGE_PIN Y4 IOSTANDARD TMDS_33} [get_ports HDMI_CLK_p]
set_property -dict {PACKAGE_PIN AA4 IOSTANDARD TMDS_33} [get_ports HDMI_CLK_n]
set_property -dict {PACKAGE_PIN Y3 IOSTANDARD TMDS_33} [get_ports {HDMI_TMDS_p[0]}]
set_property -dict {PACKAGE_PIN AA3 IOSTANDARD TMDS_33} [get_ports {HDMI_TMDS_n[0]}]
set_property -dict {PACKAGE_PIN V4 IOSTANDARD TMDS_33} [get_ports {HDMI_TMDS_p[1]}]
set_property -dict {PACKAGE_PIN W4 IOSTANDARD TMDS_33} [get_ports {HDMI_TMDS_n[1]}]
set_property -dict {PACKAGE_PIN AB3 IOSTANDARD TMDS_33} [get_ports {HDMI_TMDS_p[2]}]
set_property -dict {PACKAGE_PIN AB2 IOSTANDARD TMDS_33} [get_ports {HDMI_TMDS_n[2]}]

# CEC, SDA, SCL, DPD_DET
#set_property -dict { PACKAGE_PIN ??? IOSTANDARD LVCMOS33 } [get_ports HDMI_CEC]
#set_property -dict { PACKAGE_PIN ??? IOSTANDARD LVCMOS33 } [get_ports HDMI_SDA]
#set_property -dict { PACKAGE_PIN ??? IOSTANDARD LVCMOS33 } [get_ports HDMI_SCL]
#set_property -dict { PACKAGE_PIN ??? IOSTANDARD LVCMOS33 } [get_ports HDMI_HPD]

## ------------------------------------------------------------------------------------------------------
## 12bpp (4:4:4) Video output - SII164CTG64
## ------------------------------------------------------------------------------------------------------

## 2D/2E/2F
## 12bpp output
## set_property -dict {PACKAGE_PIN AA3 IOSTANDARD LVCMOS33} [get_ports vvsync]
## set_property -dict {PACKAGE_PIN Y3 IOSTANDARD LVCMOS33} [get_ports vhsync]
## set_property -dict {PACKAGE_PIN W4 IOSTANDARD LVCMOS33} [get_ports vde]
## set_property -dict {PACKAGE_PIN V4 IOSTANDARD LVCMOS33} [get_ports vclk]
## set_property -dict {PACKAGE_PIN AB2 IOSTANDARD LVCMOS33} [get_ports {vdat[0]}]
## set_property -dict {PACKAGE_PIN AB3 IOSTANDARD LVCMOS33} [get_ports {vdat[1]}]
## set_property -dict {PACKAGE_PIN AB1 IOSTANDARD LVCMOS33} [get_ports {vdat[2]}]
## set_property -dict {PACKAGE_PIN AA1 IOSTANDARD LVCMOS33} [get_ports {vdat[3]}]
## set_property -dict {PACKAGE_PIN Y1 IOSTANDARD LVCMOS33} [get_ports {vdat[4]}]
## set_property -dict {PACKAGE_PIN W1 IOSTANDARD LVCMOS33} [get_ports {vdat[5]}]
## set_property -dict {PACKAGE_PIN U1 IOSTANDARD LVCMOS33} [get_ports {vdat[6]}]
## set_property -dict {PACKAGE_PIN T1 IOSTANDARD LVCMOS33} [get_ports {vdat[7]}]
## set_property -dict {PACKAGE_PIN U5 IOSTANDARD LVCMOS33} [get_ports {vdat[8]}]
## set_property -dict {PACKAGE_PIN T5 IOSTANDARD LVCMOS33} [get_ports {vdat[9]}]
## set_property -dict {PACKAGE_PIN T4 IOSTANDARD LVCMOS33} [get_ports {vdat[10]}]
## set_property -dict {PACKAGE_PIN R4 IOSTANDARD LVCMOS33} [get_ports {vdat[11]}]

## Add the following if using 24bpp (NOTE: Boards 2D/2E/2F do not have these connected!)
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[12]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[13]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[14]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[15]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[16]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[17]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[18]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[19]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[20]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[21]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[22]}]
## set_property -dict {PACKAGE_PIN ?? IOSTANDARD LVCMOS33} [get_ports {vdat[23]}]

## ------------------------------------------------------------------------------------------------------
## Audio output - CS4344-CZZR
## ------------------------------------------------------------------------------------------------------

## 2D/2E/2F
## set_property -dict {PACKAGE_PIN H2 IOSTANDARD LVCMOS33} [get_ports au_sdin]
## set_property -dict {PACKAGE_PIN K1 IOSTANDARD LVCMOS33} [get_ports au_sclk]
## set_property -dict {PACKAGE_PIN K2 IOSTANDARD LVCMOS33} [get_ports au_lrclk]
## set_property -dict {PACKAGE_PIN M1 IOSTANDARD LVCMOS33} [get_ports au_mclk]

## ------------------------------------------------------------------------------------------------------
## JTAG
## ------------------------------------------------------------------------------------------------------

## 2H
## set_property -dict {PACKAGE_PIN E2 IOSTANDARD LVCMOS33} [get_ports JTAGTCK]
## set_property -dict {PACKAGE_PIN E1 IOSTANDARD LVCMOS33} [get_ports JTAGTDO]
## set_property -dict {PACKAGE_PIN C2 IOSTANDARD LVCMOS33} [get_ports JTAGTDI]
## set_property -dict {PACKAGE_PIN B1 IOSTANDARD LVCMOS33} [get_ports JTAGTMS]


## ------------------------------------------------------------------------------------------------------
## ESP32-C6-WROOM1-N8
## ------------------------------------------------------------------------------------------------------

## [IO8:IO9] - BOOTMODE[1:0] - 2'bx1=SPI flash boot, 2'b10=Download boot, 2'b00=invalid
## IO12 - USB D-
## IO13 - USB D+

## TXD0 - TBD
## set_property -dict {PACKAGE_PIN H18 IOSTANDARD LVCMOS33} [get_ports esp_rxd0_in]
## RXD0 - TBD
## set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVCMOS33} [get_ports esp_txd0_out]
## IO22 (TXD1)
set_property -dict {PACKAGE_PIN AB18 IOSTANDARD LVCMOS33} [get_ports esp_rxd1_in]
## IO23 (RXD1)
set_property -dict {PACKAGE_PIN Y19 IOSTANDARD LVCMOS33} [get_ports esp_txd1_out]

## ESP to SDCard link
## IO1: CS - sdcard_cs_n (AA19)
## set_property -dict {PACKAGE_PIN H14 IOSTANDARD LVCMOS33} [get_ports esp_cs]
## IO4: MOSI - sdcard_mosi (AA20)
## set_property -dict {PACKAGE_PIN K17 IOSTANDARD LVCMOS33} [get_ports esp_mosi]
## IO5: SCK - sdcard_clk (AB21)
## set_property -dict {PACKAGE_PIN J17 IOSTANDARD LVCMOS33} [get_ports esp_sck]
## IO6: MISO - sdcard_miso (V18)
## set_property -dict {PACKAGE_PIN L19 IOSTANDARD LVCMOS33} [get_ports esp_miso]
## IO?: SWITCH - sdcard_swtch (U32)
## set_property -dict {PACKAGE_PIN ??? IOSTANDARD LVCMOS33} [get_ports esp_switch]

## IO18: REBOOT from esp32
set_property -dict {PACKAGE_PIN V19 IOSTANDARD LVCMOS33} [get_ports cpu_reboot]
set_property PULLTYPE PULLDOWN [get_ports cpu_reboot]

## Enable pin of ESP32
set_property -dict {PACKAGE_PIN AA18 IOSTANDARD LVCMOS33} [get_ports esp_ena]
set_property PULLTYPE PULLDOWN [get_ports esp_ena]

## ------------------------------------------------------------------------------------------------------
## USB-A - MAX3421EECJ over SPI interface, USB Host
## ------------------------------------------------------------------------------------------------------

## 2D/2E/2F
## set_property -dict {PACKAGE_PIN AB8 IOSTANDARD LVCMOS33} [get_ports usba_resn]
## set_property -dict {PACKAGE_PIN AB6 IOSTANDARD LVCMOS33} [get_ports usba_ss_n]
## set_property -dict {PACKAGE_PIN AA8 IOSTANDARD LVCMOS33} [get_ports usba_clk]
## set_property -dict {PACKAGE_PIN AB5 IOSTANDARD LVCMOS33} [get_ports usba_mosi]
## set_property -dict {PACKAGE_PIN AB7 IOSTANDARD LVCMOS33} [get_ports usba_miso]
## set_property -dict {PACKAGE_PIN AA4 IOSTANDARD LVCMOS33} [get_ports usba_int]
## ## set_property -dict {PACKAGE_PIN AA5 IOSTANDARD LVCMOS33} [get_ports usba_gpx]

## set_property PULLTYPE PULLUP [get_ports usba_int]
## set_property PULLTYPE PULLUP [get_ports usba_resn]

## ------------------------------------------------------------------------------------------------------
## NMI switch
## ------------------------------------------------------------------------------------------------------

## 2B
## set_property -dict {PACKAGE_PIN Y4 IOSTANDARD LVCMOS33} [get_ports sysresetn]

## 2D
## set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVCMOS33} [get_ports sysresetn]

## 2E has no soft reset switch
## 2F has GPIO which can control a reset sequence depending on firmware

## set_property PULLUP true [get_ports sysresetn]

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
## False paths
## ------------------------------------------------------------------------------------------------------

set_false_path -to [get_ports {leds[*]}]
## Audio clock false path for generated 44.1KHz clock
set_false_path -from [get_clocks audiosampleclk] -to [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]]

## ------------------------------------------------------------------------------------------------------
## Timing
## ------------------------------------------------------------------------------------------------------

## set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets {clockandresetinst/sys_clock_i}]

## Output delay for LEDs - may not be required
## set_output_delay -clock [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] 5.000 [get_ports {{leds[0]} {leds[1]} {leds[2]} {leds[3]} {leds[4]} {leds[5]}}]

## Relative to bus clock
## set_input_delay -clock [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1] -min -add_delay 1.000 [get_ports cpu_reboot]
## set_input_delay -clock [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1] -max -add_delay 2.000 [get_ports cpu_reboot]

## ------------------------------------------------------------------------------------------------------
## Programming
## Important: Make sure to use bitstream compression to avoid excessively long boot times on the board
## ------------------------------------------------------------------------------------------------------

set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]

## ------------------------------------------------------------------------------------------------------
## Clock groups
## ------------------------------------------------------------------------------------------------------

set_clock_groups -name grpA -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpB -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT0]]
set_clock_groups -name grpC -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpD -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpE -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT3]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpF -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT3]]
set_clock_groups -name grpG -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]]
set_clock_groups -name grpH -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpI -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpJ -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]]
set_clock_groups -name grpK -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]]
set_clock_groups -name grpL -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT0]]
set_clock_groups -name grpM -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]]
set_clock_groups -name grpN -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name grpO -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]] -group [get_clocks -of_objects [get_pins socinstance/axi4ddr3sdraminst/ddr3instance/u_mig_7series_0_mig/u_ddr3_infrastructure/gen_mmcm.mmcm_i/CLKFBOUT]]
set_clock_groups -name grpP -asynchronous -group [get_clocks -of_objects [get_pins socinstance/axi4ddr3sdraminst/ddr3instance/u_mig_7series_0_mig/u_ddr3_infrastructure/gen_mmcm.mmcm_i/CLKFBOUT]] -group [get_clocks -of_objects [get_pins clockandresetinst/peripheralclkinst/inst/mmcm_adv_inst/CLKOUT1]]

## ------------------------------------------------------------------------------------------------------
## PBLOCKs
## ------------------------------------------------------------------------------------------------------

create_pblock pblock_CPU0
add_cells_to_pblock [get_pblocks pblock_CPU0] [get_cells -quiet [list socinstance/hart0]]
resize_pblock [get_pblocks pblock_CPU0] -add {CLOCKREGION_X1Y3:CLOCKREGION_X1Y4}
create_pblock pblock_CPU1
add_cells_to_pblock [get_pblocks pblock_CPU1] [get_cells -quiet [list socinstance/hart1]]
resize_pblock [get_pblocks pblock_CPU1] -add {CLOCKREGION_X1Y0:CLOCKREGION_X1Y1}

create_pblock pblock_leddevice
add_cells_to_pblock [get_pblocks pblock_leddevice] [get_cells -quiet [list socinstance/leddevice]]
resize_pblock [get_pblocks pblock_leddevice] -add {SLICE_X2Y120:SLICE_X7Y128}
resize_pblock [get_pblocks pblock_leddevice] -add {RAMB18_X0Y48:RAMB18_X0Y49}
resize_pblock [get_pblocks pblock_leddevice] -add {RAMB36_X0Y24:RAMB36_X0Y24}

create_pblock pblock_uartinst
add_cells_to_pblock [get_pblocks pblock_uartinst] [get_cells -quiet [list socinstance/uartinst]]
resize_pblock [get_pblocks pblock_uartinst] -add {SLICE_X20Y112:SLICE_X23Y120}
resize_pblock [get_pblocks pblock_uartinst] -add {DSP48_X1Y46:DSP48_X1Y47}
resize_pblock [get_pblocks pblock_uartinst] -add {RAMB18_X1Y46:RAMB18_X1Y47}
resize_pblock [get_pblocks pblock_uartinst] -add {RAMB36_X1Y23:RAMB36_X1Y23}
