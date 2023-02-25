## Constraints for the QMTECH A7200T board including some i/o setup for custom expansion board

set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## Buttons
set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

## LEDs
set_property -dict {PACKAGE_PIN F3 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]
set_property -dict {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]

## UART
## VCC:     pin U4:3&4
## GND:     pin U4:1&2&5&6
## RXD_OUT: pin U4:8         [A1]
## TDX_IN:  pin U4:10        [B2]
set_property -dict {PACKAGE_PIN A1 IOSTANDARD LVCMOS33} [get_ports uart_rxd_out]
set_property -dict {PACKAGE_PIN B2 IOSTANDARD LVCMOS33} [get_ports uart_txd_in]

## MICRO SD CARD
## DAT[1]:	    	pin U4:11  [E2]
## DAT[0] / MISO:	pin U4:13  [G1]
## CLK:			    pin U4:15  [H2]
## SCMD / MOSI: 	pin U4:17  [K1]
## DAT[3] / CS: 	pin U4:19  [K2]
## DAT[2]:	    	pin U4:21  [M1]
## SPI mode
set_property -dict {PACKAGE_PIN G1 IOSTANDARD LVCMOS33} [get_ports sd_miso]
set_property -dict {PACKAGE_PIN K2 IOSTANDARD LVCMOS33} [get_ports sd_cs_n]
set_property -dict {PACKAGE_PIN H2 IOSTANDARD LVCMOS33} [get_ports sd_clk]
set_property -dict {PACKAGE_PIN K1 IOSTANDARD LVCMOS33} [get_ports sd_mosi]
## Regular mode
## set_property -dict {PACKAGE_PIN G1 IOSTANDARD LVCMOS33} [get_ports {spi_dat[0]}]
## set_property -dict {PACKAGE_PIN E2 IOSTANDARD LVCMOS33} [get_ports {spi_dat[1]}]
## set_property -dict {PACKAGE_PIN M1 IOSTANDARD LVCMOS33} [get_ports {spi_dat[2]}]
## set_property -dict {PACKAGE_PIN K2 IOSTANDARD LVCMOS33} [get_ports {spi_dat[3]}]
## set_property -dict {PACKAGE_PIN H2 IOSTANDARD LVCMOS33} [get_ports spi_clk]
## set_property -dict {PACKAGE_PIN K1 IOSTANDARD LVCMOS33} [get_ports spi_cmd]

## DVI -> HDMI
## hdmi_tx_clk_p pin U4:41 [T1]
## hdmi_tx_clk_n pin U4:42 [U1]
## hdmi_tx_p[0]  pin U4:47 [AB3]
## hdmi_tx_p[1]  pin U4:43 [W1]
## hdmi_tx_p[2]  pin U4:45 [AA1]
## hdmi_tx_n[0]  pin U4:48 [AB2]
## hdmi_tx_n[1]  pin U4:44 [Y1]
## hdmi_tx_n[2]  pin U4:46 [AB1]
set_property -dict {PACKAGE_PIN T1 IOSTANDARD TMDS_33} [get_ports hdmi_tx_clk_p]
set_property -dict {PACKAGE_PIN U1 IOSTANDARD TMDS_33} [get_ports hdmi_tx_clk_n]
set_property -dict {PACKAGE_PIN AB3 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_p[0]}]
set_property -dict {PACKAGE_PIN W1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_p[1]}]
set_property -dict {PACKAGE_PIN AA1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_p[2]}]
set_property -dict {PACKAGE_PIN AB2 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_n[0]}]
set_property -dict {PACKAGE_PIN Y1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_n[1]}]
set_property -dict {PACKAGE_PIN AB1 IOSTANDARD TMDS_33} [get_ports {hdmi_tx_n[2]}]

## Analog inputs routed to XADC
## xa_p[0] pin U2:7  [H22]
## xa_p[1] pin U2:9  [H18]
## xa_p[2] pin U2:11 [K22]
## xa_p[3] pin U2:13 [G20]
## xa_n[0] pin U2:8  [J22]
## xa_n[1] pin U2:10 [H17]
## xa_n[2] pin U2:12 [K21]
## xa_n[3] pin U2:14 [H20]
set_property -dict {PACKAGE_PIN H22 IOSTANDARD TMDS_33} [get_ports {xa_p[0]}]
set_property -dict {PACKAGE_PIN H18 IOSTANDARD TMDS_33} [get_ports {xa_p[1]}]
set_property -dict {PACKAGE_PIN K22 IOSTANDARD TMDS_33} [get_ports {xa_p[2]}]
set_property -dict {PACKAGE_PIN G20 IOSTANDARD TMDS_33} [get_ports {xa_p[3]}]
set_property -dict {PACKAGE_PIN J22 IOSTANDARD TMDS_33} [get_ports {xa_n[0]}]
set_property -dict {PACKAGE_PIN H17 IOSTANDARD TMDS_33} [get_ports {xa_n[1]}]
set_property -dict {PACKAGE_PIN K21 IOSTANDARD TMDS_33} [get_ports {xa_n[2]}]
set_property -dict {PACKAGE_PIN H20 IOSTANDARD TMDS_33} [get_ports {xa_n[3]}]

## DDR3 SDRAM MT41K128M16XX-15E
## width: 16, period: 2500, mask: 1
##set_property {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports init_calib_complete]
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

##set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets {clockandresetinst/sys_clock_i}]

## Timing
set_false_path -to [get_ports {leds[*]}]

## Programming
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]

set_clock_groups -name asyncA -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT1]]
set_clock_groups -name asyncB -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT2]]
set_clock_groups -name asyncC -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT3]]
set_clock_groups -name asyncD -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]]
set_clock_groups -name asyncE -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT4]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT5]]
set_clock_groups -name asyncF -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT6]]
set_clock_groups -name asyncG -asynchronous -group [get_clocks -of_objects [get_pins clockandresetinst/centralclockinst/inst/mmcm_adv_inst/CLKOUT0]] -group [get_clocks -of_objects [get_pins socinstance/axi4ddr3sdraminst/ddr3instance/u_mig_7series_0_mig/u_ddr3_infrastructure/gen_mmcm.mmcm_i/CLKFBOUT]]
