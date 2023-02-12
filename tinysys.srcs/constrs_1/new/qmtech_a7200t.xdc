## Constraints for the QMTECH A7200T board including some i/o setup for custom expansion board

set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## Buttons
#set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

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



create_pblock pblock_clockandresetinst
add_cells_to_pblock [get_pblocks pblock_clockandresetinst] [get_cells -quiet [list clockandresetinst]]
resize_pblock [get_pblocks pblock_clockandresetinst] -add {SLICE_X134Y66:SLICE_X151Y79}
resize_pblock [get_pblocks pblock_clockandresetinst] -add {DSP48_X7Y28:DSP48_X7Y31}
resize_pblock [get_pblocks pblock_clockandresetinst] -add {RAMB18_X7Y28:RAMB18_X7Y31}
resize_pblock [get_pblocks pblock_clockandresetinst] -add {RAMB36_X7Y14:RAMB36_X7Y15}
create_pblock pblock_instructionfetch
add_cells_to_pblock [get_pblocks pblock_instructionfetch] [get_cells -quiet [list socinstance/instructionfetch]]
resize_pblock [get_pblocks pblock_instructionfetch] -add {SLICE_X50Y52:SLICE_X81Y98}
resize_pblock [get_pblocks pblock_instructionfetch] -add {DSP48_X3Y22:DSP48_X4Y37}
resize_pblock [get_pblocks pblock_instructionfetch] -add {RAMB18_X3Y22:RAMB18_X4Y37}
resize_pblock [get_pblocks pblock_instructionfetch] -add {RAMB36_X3Y11:RAMB36_X4Y18}
create_pblock pblock_dataunitints
add_cells_to_pblock [get_pblocks pblock_dataunitints] [get_cells -quiet [list socinstance/dataunitints]]
resize_pblock [get_pblocks pblock_dataunitints] -add {SLICE_X4Y52:SLICE_X45Y98}
resize_pblock [get_pblocks pblock_dataunitints] -add {DSP48_X0Y22:DSP48_X1Y37}
resize_pblock [get_pblocks pblock_dataunitints] -add {RAMB18_X0Y22:RAMB18_X2Y37}
resize_pblock [get_pblocks pblock_dataunitints] -add {RAMB36_X0Y11:RAMB36_X2Y18}
create_pblock pblock_controlunitinst
add_cells_to_pblock [get_pblocks pblock_controlunitinst] [get_cells -quiet [list socinstance/controlunitinst]]
resize_pblock [get_pblocks pblock_controlunitinst] -add {SLICE_X2Y104:SLICE_X41Y143}
resize_pblock [get_pblocks pblock_controlunitinst] -add {DSP48_X0Y42:DSP48_X1Y55}
resize_pblock [get_pblocks pblock_controlunitinst] -add {RAMB18_X0Y42:RAMB18_X1Y55}
resize_pblock [get_pblocks pblock_controlunitinst] -add {RAMB36_X0Y21:RAMB36_X1Y27}
create_pblock pblock_uartdevice
add_cells_to_pblock [get_pblocks pblock_uartdevice] [get_cells -quiet [list socinstance/uartdevice]]
resize_pblock [get_pblocks pblock_uartdevice] -add {SLICE_X132Y162:SLICE_X155Y186}
resize_pblock [get_pblocks pblock_uartdevice] -add {DSP48_X7Y66:DSP48_X8Y73}
resize_pblock [get_pblocks pblock_uartdevice] -add {RAMB18_X7Y66:RAMB18_X7Y73}
resize_pblock [get_pblocks pblock_uartdevice] -add {RAMB36_X7Y33:RAMB36_X7Y36}

create_pblock pblock_ddr3instance
add_cells_to_pblock [get_pblocks pblock_ddr3instance] [get_cells -quiet [list socinstance/axi4ddr3sdraminst/ddr3instance]]
resize_pblock [get_pblocks pblock_ddr3instance] -add {SLICE_X0Y249:SLICE_X55Y184}
resize_pblock [get_pblocks pblock_ddr3instance] -add {DSP48_X0Y74:DSP48_X2Y99}
resize_pblock [get_pblocks pblock_ddr3instance] -add {RAMB18_X0Y74:RAMB18_X2Y99}
resize_pblock [get_pblocks pblock_ddr3instance] -add {RAMB36_X0Y37:RAMB36_X2Y49}
create_pblock pblock_axi4interconnectinst
add_cells_to_pblock [get_pblocks pblock_axi4interconnectinst] [get_cells -quiet [list socinstance/axi4ddr3sdraminst/axi4interconnectinst]]
resize_pblock [get_pblocks pblock_axi4interconnectinst] -add {SLICE_X42Y104:SLICE_X81Y144}
resize_pblock [get_pblocks pblock_axi4interconnectinst] -add {DSP48_X2Y42:DSP48_X4Y57}
resize_pblock [get_pblocks pblock_axi4interconnectinst] -add {RAMB18_X3Y42:RAMB18_X4Y57}
resize_pblock [get_pblocks pblock_axi4interconnectinst] -add {RAMB36_X3Y21:RAMB36_X4Y28}
