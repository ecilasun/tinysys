`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	input wire sys_resetn,
	output wire aclk,
	output wire clk15,
	input wire calib_done,
	output wire preresetn,
	output wire aresetn );

(* async_reg = "true" *) logic regaresetn = 1'b0;
assign preresetn = regaresetn;
assign aresetn = regaresetn && calib_done;

wire centralclocklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.aclk(aclk),
	.clk15(clk15),
	.locked(centralclocklocked) );

// Hold reset until both clocks are locked
wire internalreset = ~(sys_resetn) && ~(centralclocklocked /* && peripheralclocklocked && ddr3clocklocked*/);
(* async_reg = "true" *) logic internalresetA = 1'b1;
(* async_reg = "true" *) logic internalresetB = 1'b1;

// Delayed reset post-clock-lock
logic [15:0] resetcountdown = 16'h0001;

always @(posedge aclk) begin
	if (internalresetB) begin
		resetcountdown <= 16'h0001;
		regaresetn <= 1'b0;
	end else begin
		resetcountdown <= {resetcountdown[14:0], 1'b1};
		regaresetn <= resetcountdown[15];
	end
	// DC
	internalresetA <= internalreset;
	internalresetB <= internalresetA;
end

endmodule
