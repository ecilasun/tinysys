`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	output wire aclk,
	output wire clk10,
	output wire clk166,
	output wire clk200,
	input wire calib_done,
	output wire preresetn,
	output wire aresetn );

wire centralclocklocked, ddr3clklocked;

(* async_reg = "true" *) logic calibA = 1'b0;
(* async_reg = "true" *) logic calibB = 1'b0;
(* async_reg = "true" *) logic regaresetn = 1'b0;

assign aresetn = regaresetn ? calibB : 1'b0;
assign preresetn = regaresetn;

// CdC from uiclk to aclk
always @(posedge aclk) begin
	if (~regaresetn) begin
		calibA <= 1'b0;
		calibB <= 1'b0;
	end else begin
		calibA <= calib_done;
		calibB <= calibA;
	end
end

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

// Hold reset until clocks are locked
wire internalreset = ~(centralclocklocked & ddr3clklocked);
(* async_reg = "true" *) logic resettrigA = 1'b1;
(* async_reg = "true" *) logic resettrigB = 1'b1;

// Delayed reset post-clock-lock
logic [15:0] resetcountdown = 16'h0001;

always @(posedge aclk) begin
	if (resettrigB) begin
		resetcountdown <= 16'h0001;
		regaresetn <= 1'b0;
	end else begin
		resetcountdown <= {resetcountdown[14:0], 1'b1};
		regaresetn <= resetcountdown[15];
	end
	// DC
	resettrigA <= internalreset;
	resettrigB <= resettrigA;
end

endmodule
