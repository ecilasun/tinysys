`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	input wire sys_resetn,
	output wire aclk,
	output wire clk10,
	input wire calib_done,
	output wire preresetn,
	output wire aresetn );

// calib_done
(* async_reg = "true" *) logic calibA = 1'b1;
(* async_reg = "true" *) logic calibB = 1'b1;

always @(posedge aclk) begin
	calibA <= calib_done;
	calibB <= calibA;
end

// sys_resetn
(* async_reg = "true" *) logic rstnA = 1'b1;
(* async_reg = "true" *) logic rstnB = 1'b1;

always @(posedge aclk) begin
	rstnA <= sys_resetn;
	rstnB <= rstnA;
end

// Final reset state
(* async_reg = "true" *) logic regaresetn = 1'b0;
assign preresetn = regaresetn;
assign aresetn = regaresetn && calibB;

// Gen
wire centralclocklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.aclk(aclk),
	.clk10(clk10),
	.locked(centralclocklocked) );

// Hold reset until both clocks are locked
wire internalreset = ~(centralclocklocked /* && peripheralclocklocked && ddr3clocklocked*/);
(* async_reg = "true" *) logic internalresetA = 1'b1;
(* async_reg = "true" *) logic internalresetB = 1'b1;

// Delayed reset post-clock-lock
logic [15:0] resetcountdown = 16'h0001;

always @(posedge aclk) begin
	if (internalresetB || ~rstnB) begin
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
