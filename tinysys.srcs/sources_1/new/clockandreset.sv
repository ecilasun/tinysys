`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	input wire sys_rst_n,
	input wire init_calib_complete,
	output wire aclk,
	output wire clk10,
	output wire clk25,
	output wire clk50,
	output wire clk125,
	output wire clk166,
	output wire clk200,
	output wire aresetn,
	output wire preresetn);

// --------------------------------------------------
// PLLs / MMCMs
// --------------------------------------------------

wire centralclocklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.aclk(aclk),
	.clk10(clk10),
	.clk25(clk25),
	.clk50(clk50),
	.clk125(clk125),
	.clk166(clk166),
	.clk200(clk200),
	.locked(centralclocklocked) );

wire clocksready = centralclocklocked; ///& ddr3clklocked;

// --------------------------------------------------
// DDR3 SDRAM cabilration complete
// --------------------------------------------------

logic [1:0] ddr3ready = 2'b00;
always @(posedge aclk) begin
	ddr3ready <= {ddr3ready[0], init_calib_complete};
end

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

logic [31:0] resetcountdown = 32'd0;
logic regaresetn = 1'b0;

always @(posedge aclk or negedge sys_rst_n) begin
	if (~sys_rst_n) begin
		resetcountdown <= 32'd0;
		regaresetn <= 1'b0;
	end else begin
		resetcountdown <= {resetcountdown[30:0], clkRdyB};
		regaresetn <= resetcountdown[31] && ddr3ready[1];
	end
end

// --------------------------------------------------
// Pre-reset for DDR3 SDRAM & CdC
// --------------------------------------------------

logic regpreresetn = 1'b0;

(* async_reg = "true" *) logic sysRdyA = 1'b0;
(* async_reg = "true" *) logic sysRdyB = 1'b0;

always @(posedge clk166) begin
	sysRdyA <= clocksready;
	sysRdyB <= sysRdyA;
	regpreresetn <= sysRdyB;
end

assign aresetn = regaresetn;
assign preresetn = regpreresetn;

endmodule
