set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports sys_clk]
create_clock -period 20.000 -name sys_clk_pin -waveform {0.000 10.000} -add [get_ports sys_clk]

## Buttons
set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]

## LEDs
set_property -dict {PACKAGE_PIN F3 IOSTANDARD LVCMOS33} [get_ports {leds[0]}]
set_property -dict {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports {leds[1]}]

## UART
## VCC: pin3/pin4 U4
## GND: pin1/2/5/6 U4
## RXD_OUT: pin7 U4
## TDX_IN: pin8 U4
set_property -dict {PACKAGE_PIN B1 IOSTANDARD LVCMOS33} [get_ports uart_rxd_out]
set_property -dict {PACKAGE_PIN A1 IOSTANDARD LVCMOS33} [get_ports uart_txd_in]

## Timing
set_false_path -to [get_ports {leds[*]}]

## Programming
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
