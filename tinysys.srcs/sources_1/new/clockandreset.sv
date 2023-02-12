`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	input wire sys_rst_n,
	output wire aclk,
	output wire clk10,
	output wire clk166,
	output wire clk200,
	input wire calib_done,
	output wire preresetn,
	output wire aresetn );

// --------------------------------------------------
// PLLs / MMCMs
// --------------------------------------------------

wire centralclocklocked, ddr3clklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.aclk(aclk),
	.clk10(clk10),
	.locked(centralclocklocked) );

peripheralclocks ddr3sdramclockinst(
	.clk_in1(sys_clock_i),
	.clk166(clk166),
	.clk200(clk200),
	.locked(ddr3clklocked) );

wire clocksready = centralclocklocked & ddr3clklocked;

// --------------------------------------------------
// Pre-reset
// --------------------------------------------------

(* async_reg = "true" *) logic preresetA = 1'b0;
(* async_reg = "true" *) logic preresetB = 1'b0;

always @(posedge aclk) begin
	preresetA <= sys_rst_n;
	preresetB <= preresetA;
end

// --------------------------------------------------
// Clock domain crossing calibration done line
// --------------------------------------------------

/*(* async_reg = "true" *) logic calibA = 1'b0;
(* async_reg = "true" *) logic calibB = 1'b0;

// CdC from uiclk to aclk
always @(posedge aclk) begin
	calibA <= calib_done;
	calibB <= calibA;
end*/ // TODO:

// --------------------------------------------------
// Clock domain crossing PLL/MMCM ready line
// --------------------------------------------------

(* async_reg = "true" *) logic clkRdyA = 1'b0;
(* async_reg = "true" *) logic clkRdyB = 1'b0;

always @(posedge aclk) begin
	clkRdyA <= clocksready;
	clkRdyB <= clkRdyA;
end

// --------------------------------------------------
// Outside facing delayed reset
// --------------------------------------------------

logic [15:0] resetcountdown = 16'h0001;
logic regaresetn = 1'b0;

always @(posedge aclk) begin
	if (~clkRdyB) begin
		resetcountdown <= 16'h0001;
		regaresetn <= 1'b0;
	end else begin
		resetcountdown <= {resetcountdown[14:0], 1'b1};
		regaresetn <= resetcountdown[15];
	end
end

assign aresetn = regaresetn;
assign preresetn = preresetB;

endmodule
