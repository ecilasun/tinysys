`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	input wire sys_rst_n,
	output wire aclk,
	output wire clk10,
	output wire clk166,
	output wire clk200,
	output wire aresetn );

// --------------------------------------------------
// PLLs / MMCMs
// --------------------------------------------------

wire centralclocklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.aclk(aclk),
	.clk10(clk10),
	.clk166(clk166),
	.clk200(clk200),
	.locked(centralclocklocked) );

wire clocksready = centralclocklocked; ///& ddr3clklocked;

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

always @(posedge aclk or negedge sys_rst_n) begin
	if (~sys_rst_n) begin
		resetcountdown <= 16'h0001;
		regaresetn <= 1'b0;
	end else begin
		resetcountdown <= {resetcountdown[14:0], 1'b1};
		regaresetn <= resetcountdown[15] & clkRdyB;
	end
end

assign aresetn = regaresetn;

endmodule
