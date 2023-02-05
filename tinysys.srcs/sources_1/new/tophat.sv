`timescale 1ns / 1ps

module tophat(
    input sys_clk,
    input sys_rst_n,
    output wire [1:0] leds,
    output wire uart_rxd_out,
	input wire uart_txd_in);

// --------------------------------------------------
// Clock and reset generator
// --------------------------------------------------

wire aresetn;
wire preresetn, calib_done;
wire aclk, clk10;

// Clock and reset generator
clockandreset clockandresetinst(
	.sys_clock_i(sys_clk),
	.sys_resetn(sys_rst_n),
	.aclk(aclk),
	.clk10(clk10),
	.calib_done(calib_done),
	.preresetn(preresetn),	// TODO: Use as reset signal for devices that we need initialized before the CPU/GPU such as SDRAM
	.aresetn(aresetn));

// TODO: In normal operation,t this will be set high by the SDRAM controller when it's done calibrating
// Here we set it high manually, so that aresetn can assert in the absence of SDRAM controller.
assign calib_done = 1'b1;

// --------------------------------------------------
// SoC device
// --------------------------------------------------

tinysoc socinstance(
	.aclk(aclk),
	.clk10(clk10),
	.aresetn(aresetn),
	.uart_rxd_out(uart_rxd_out),
	.uart_txd_in(uart_txd_in),
	.leds(leds));

endmodule
