set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## Buttons
set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

## LEDs
set_property -dict {PACKAGE_PIN F3 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]
set_property -dict {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]

## UART
## VCC: pin U4:{3,4}
## GND: pin U4:{1,2,5,6}
## RXD_OUT: pin U4:8 [A1]
## TDX_IN: pin U4:10 [B2]
set_property -dict {PACKAGE_PIN A1 IOSTANDARD LVCMOS33} [get_ports uart_rxd_out]
set_property -dict {PACKAGE_PIN B2 IOSTANDARD LVCMOS33} [get_ports uart_txd_in]

## DDR3 SDRAM MT41K128M16XX-15E
## width: 16, period: 2500, mask: 1
## set_property {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports init_calib_complete]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_p[0]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_n[0]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_p[1]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dqs_n[1]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[0]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[1]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[2]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[3]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[4]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[5]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[6]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[7]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[8]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[9]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[10]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[11]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[12]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[13]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[14]}]
## set_property IBUF_LOW_PWR FALSE [get_ports {ddr3_dq[15]}]
## set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets {clk_wiz_0_inst/inst/clk_in1_clk_wiz_0}]

## Timing
set_false_path -to [get_ports {leds[*]}]

## Programming
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]
