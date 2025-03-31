`timescale 1ns / 1ps
`default_nettype none

module tophat(
	// Board clock and reset
    input wire sys_clk
    // Debug LEDs
    ,output wire [5:0] leds
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
	// To HDMI connector
	,output wire HDMI_CLK_p
	,output wire HDMI_CLK_n
	,output wire [2:0] HDMI_TMDS_p
	,output wire [2:0] HDMI_TMDS_n
	//,input wire HDMI_CEC
	//,inout wire HDMI_SDA
	//,inout wire HDMI_SCL
	//,input wire HDMI_HPD
  	// Micro SD Card
	,input wire sdcard_miso
	,output wire sdcard_cs_n
	,output wire sdcard_clk
	,output wire sdcard_mosi
	,input wire sdcard_swtch
	// Coprocessor via ESP32-C6-WROOM-1-N8 (only on rev. 2E boards)
	,output wire esp_txd1_out
	,input wire esp_rxd1_in
	,input wire fpga_reboot
	,output wire esp_ena );

// --------------------------------------------------
// Clock and reset generator
// --------------------------------------------------

wire aresetn, rst10n, rst25n, rst100n, rstaudion, preresetn;
wire init_calib_complete;
wire clk10, clkaudio, clk25, clk100, clk125, clkbus, clk166, clk200;

wire rebootn;
BUF (.O(rebootn), .I(~fpga_reboot));

// Clock and reset generator
clockandreset clockandresetinst(
	.sys_clock_i(sys_clk),
	.fpga_rstn(rebootn),
	.init_calib_complete(init_calib_complete),
	.clk10(clk10),
	.clkaudio(clkaudio),
	.clk25(clk25),
	.clk100(clk100),
	.clk125(clk125),
	.clkbus(clkbus),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	.rst10n(rst10n),
	.rst25n(rst25n),
	.rst100n(rst100n),
	.rstaudion(rstaudion),
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
// SPI wires
// --------------------------------------------------

sdcardwires sdconn(
	.miso(sdcard_miso),
	.cs_n(sdcard_cs_n),
	.clk(sdcard_clk),
	.mosi(sdcard_mosi),
	.swtch(sdcard_swtch) );

// --------------------------------------------------
// SoC device
// --------------------------------------------------

tinysoc #(.RESETVECTOR(32'h0FFE0000)) socinstance(
	.aclk(clkbus),
	.clk10(clk10),
	.clkaudio(clkaudio),
	.clk25(clk25),
	.clk100(clk100),
	.clk125(clk125),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	.rst10n(rst10n),
	.rst25n(rst25n),
	.rst100n(rst100n),
	.rstaudion(rstaudion),
	.preresetn(preresetn),
	// Device wires
	.leds(leds),
	.esp_rxd1_in(esp_rxd1_in),
	.esp_txd1_out(esp_txd1_out),
	.fpga_reboot(fpga_reboot),
	.esp_ena(esp_ena),
	.HDMI_CLK_p(HDMI_CLK_p),
	.HDMI_CLK_n(HDMI_CLK_n),
	.HDMI_TMDS_p(HDMI_TMDS_p),
	.HDMI_TMDS_n(HDMI_TMDS_n),
	.ddr3conn(ddr3conn),
	.sdconn(sdconn) );

endmodule
