`timescale 1ns / 1ps

module tophat(
	// Board clock and reset
    input sys_clk,
    input sys_rst_n,
    // Debug LEDs
    output wire [1:0] leds,
    // UART - USB-c module
    output wire uart_rxd_out,
	input wire uart_txd_in,
	// DDR3 SDRAM
	output ddr3_reset_n,
	output wire [0:0] ddr3_cke,
	output wire [0:0] ddr3_ck_p, 
	output wire [0:0] ddr3_ck_n,
	output wire ddr3_ras_n, 
	output wire ddr3_cas_n, 
	output wire ddr3_we_n,
	output wire [2:0] ddr3_ba,
	output wire [13:0] ddr3_addr,
	output wire [0:0] ddr3_odt,
	output wire [1:0] ddr3_dm,
	inout wire [1:0] ddr3_dqs_p,
	inout wire [1:0] ddr3_dqs_n,
	inout wire [15:0] ddr3_dq );

// --------------------------------------------------
// Clock and reset generator
// --------------------------------------------------

wire aresetn;
wire preresetn, init_calib_complete;
wire aclk, clk10, clk166, clk200;

// Clock and reset generator
clockandreset clockandresetinst(
	.sys_clock_i(sys_clk),
	.sys_rst_n(sys_rst_n),
	.aclk(aclk),
	.clk10(clk10),
	.clk166(clk166),
	.clk200(clk200),
	.calib_done(init_calib_complete),
	.preresetn(preresetn),	// TODO: Use as reset signal for devices that we need initialized before the CPU/GPU such as SDRAM
	.aresetn(aresetn));

// --------------------------------------------------
// DDR3 SDRAM wires
// --------------------------------------------------

ddr3sdramwires ddr3wires(
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
// SoC device
// --------------------------------------------------

tinysoc socinstance(
	.aclk(aclk),
	.clk10(clk10),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	.preresetn(preresetn),
	.uart_rxd_out(uart_rxd_out),
	.uart_txd_in(uart_txd_in),
	.leds(leds),
	.ddr3wires(ddr3wires) );

endmodule
