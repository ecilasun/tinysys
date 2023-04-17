`timescale 1ns / 1ps
`default_nettype none

module tophat(
	// Board clock and reset
    input wire sys_clk
    ,input wire sys_rst_n
    // Debug LEDs
    ,output wire [3:0] leds
    // UART - USB-c module
    ,output wire uart_rxd_out
	,input wire uart_txd_in
	// USB RX/TX
	,inout wire usb_d_p
	,inout wire usb_d_n
	// DDR3 SDRAM
	,output wire ddr3_reset_n
	,output wire [0:0] ddr3_cke
	,output wire [0:0] ddr3_ck_p 
	,output wire [0:0] ddr3_ck_n
	,output wire ddr3_ras_n 
	,output wire ddr3_cas_n 
	,output wire ddr3_we_n
	,output wire [2:0] ddr3_ba
	,output wire [13:0] ddr3_addr
	,output wire [0:0] ddr3_odt
	,output wire [1:0] ddr3_dm
	,inout wire [1:0] ddr3_dqs_p
	,inout wire [1:0] ddr3_dqs_n
	,inout wire [15:0] ddr3_dq
	// DVI -> HDMI
	,output wire [2:0] hdmi_tx_p
	,output wire [2:0] hdmi_tx_n
	,output wire hdmi_tx_clk_p
	,output wire hdmi_tx_clk_n
	// Micro SD Card
	,input wire spi_miso
	,output wire spi_cs_n
	,output wire spi_clk
	,output wire spi_mosi
	,input wire spi_swtch
	// ADC
	,output wire adclk
	,input wire addout
	,output wire addin
	,output wire adcs
	// Audio out
	,output wire au_sdin
	,output wire au_sclk
	,output wire au_lrclk
	,output wire au_mclk);

// --------------------------------------------------
// Clock and reset generator
// --------------------------------------------------

wire aresetn, preresetn;
wire init_calib_complete;
wire aclk, clk10, clk25, clk100, clk125, clk166, clk200;

// Clock and reset generator
clockandreset clockandresetinst(
	.sys_clock_i(sys_clk),
	.sys_rst_n(sys_rst_n),
	.init_calib_complete(init_calib_complete),
	.aclk(aclk),
	.clk10(clk10),
	.clk25(clk25),
	.clk100(clk100),
	.clk125(clk125),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	.preresetn(preresetn));

// --------------------------------------------------
// DDR3 SDRAM wires
// --------------------------------------------------

ddr3sdramwires ddr3conn(
	.ddr3_reset_n(ddr3_reset_n),
	.ddr3_cke(ddr3_cke),
	.ddr3_ck_p(ddr3_ck_p), 
	.ddr3_ck_n(ddr3_ck_n),
	.ddr3_ras_n(ddr3_ras_n), 
	.ddr3_cas_n(ddr3_cas_n), 
	.ddr3_we_n(ddr3_we_n),
	.ddr3_ba(ddr3_ba),
	.ddr3_addr(ddr3_addr),
	.ddr3_odt(ddr3_odt),
	.ddr3_dm(ddr3_dm),
	.ddr3_dqs_p(ddr3_dqs_p),
	.ddr3_dqs_n(ddr3_dqs_n),
	.ddr3_dq(ddr3_dq),
	.init_calib_complete(init_calib_complete) );

// --------------------------------------------------
// Video wires
// --------------------------------------------------

gpuwires gpuvideoout(
	.tmdsp(hdmi_tx_p),
	.tmdsn(hdmi_tx_n),
	.tmdsclkp(hdmi_tx_clk_p ),
	.tmdsclkn(hdmi_tx_clk_n) );

// --------------------------------------------------
// SPI wires
// --------------------------------------------------

spiwires sdconn(
	.spi_miso(spi_miso),
	.spi_cs_n(spi_cs_n),
	.spi_clk(spi_clk),
	.spi_mosi(spi_mosi),
	.spi_swtch(spi_swtch) );

// --------------------------------------------------
// ADC wires
// --------------------------------------------------

adcwires adcconn(
	.adclk(adclk),
	.addout(addout),
	.addin(addin),
	.adcs(adcs));

// --------------------------------------------------
// Audio wires
// --------------------------------------------------

audiowires i2sconn(
	.sdin(au_sdin),
	.sclk(au_sclk),
	.lrclk(au_lrclk),
	.mclk(au_mclk));

// --------------------------------------------------
// SoC device
// --------------------------------------------------

tinysoc #(.RESETVECTOR(32'h0FFE0000)) socinstance(
	.aclk(aclk),
	.clk10(clk10),
	.clk25(clk25),
	.clk100(clk100),
	.clk125(clk125),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	.preresetn(preresetn),
	.uart_rxd_out(uart_rxd_out),
	.uart_txd_in(uart_txd_in),
	.usb_d_p(usb_d_p),
	.usb_d_n(usb_d_n),
	.leds(leds),
	.ddr3conn(ddr3conn),
	.i2sconn(i2sconn),
	.gpuvideoout(gpuvideoout),
	.sdconn(sdconn),
	.adcconn(adcconn));

endmodule
