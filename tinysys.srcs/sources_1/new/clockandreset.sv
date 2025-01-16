`timescale 1ns / 1ps
`default_nettype none

module clockandreset(
	input wire sys_clock_i,
	input wire init_calib_complete,
	output wire clk10,
	output wire clkaudio,
	output wire clk25,
	output wire clk100,
	output wire clk125,
	output wire clkbus,
	output wire clk166,
	output wire clk200,
	output wire aresetn,
	output wire rst10n,
	output wire rst25n,
	output wire rst100n,
	output wire rstaudion,
	output wire preresetn);

// --------------------------------------------------
// PLLs / MMCMs
// --------------------------------------------------

wire centralclocklocked, peripheralclocklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.clk100(clk100),
	.clk10(clk10),
	.clk25(clk25),
	.clk166(clk166),
	.clk200(clk200),
	.locked(centralclocklocked) );

peripheralclocks peripheralclkinst(
	.clk_in1(sys_clock_i),
	.clkaudio(clkaudio),
	.clkbus(clkbus),
	.clkvidx5(clk125),
	.locked(peripheralclocklocked) );


wire clocksready = centralclocklocked && peripheralclocklocked;

// --------------------------------------------------
// DDR3 SDRAM cabilration complete
// --------------------------------------------------

logic [1:0] ddr3ready = 2'b00;
always @(posedge clkbus) begin
	ddr3ready <= {ddr3ready[0], init_calib_complete};
end

// --------------------------------------------------
// Clock domain crossing PLL/MMCM ready line
// --------------------------------------------------

(* async_reg = "true" *) logic clkRdyA = 1'b0;
(* async_reg = "true" *) logic clkRdyB = 1'b0;

always @(posedge clkbus) begin
	clkRdyA <= clocksready;
	clkRdyB <= clkRdyA;
end

// --------------------------------------------------
// Outside facing delayed reset
// --------------------------------------------------

logic [31:0] resetcountdown = 32'd0;
logic regaresetn;

always @(posedge clkbus) begin
	resetcountdown <= {resetcountdown[30:0], clkRdyB};
	regaresetn <= resetcountdown[31] && ddr3ready[1];
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

(* async_reg = "true" *) logic rstn10A = 1'b1;
(* async_reg = "true" *) logic rstn10B = 1'b1;
always @(posedge clk25) begin
	rstn10A <= regaresetn;
	rstn10B <= rstn10A;
end

(* async_reg = "true" *) logic rstn25A = 1'b1;
(* async_reg = "true" *) logic rstn25B = 1'b1;
always @(posedge clk25) begin
	rstn25A <= regaresetn;
	rstn25B <= rstn25A;
end

(* async_reg = "true" *) logic rstn100A = 1'b1;
(* async_reg = "true" *) logic rstn100B = 1'b1;
always @(posedge clk100) begin
	rstn100A <= regaresetn;
	rstn100B <= rstn100A;
end

(* async_reg = "true" *) logic rstaudionA = 1'b1;
(* async_reg = "true" *) logic rstaudionB = 1'b1;
always @(posedge clkaudio) begin
	rstaudionA <= regaresetn;
	rstaudionB <= rstaudionA;
end

assign rst10n = rstn10B;
assign rst25n = rstn25B;
assign rst100n = rstn100B;
assign rstaudion = rstaudionB;
assign aresetn = regaresetn;
assign preresetn = regpreresetn;

endmodule
