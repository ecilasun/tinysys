`timescale 1ns / 1ps

module clockandreset(
	input wire sys_clock_i,
	input wire init_calib_complete,
	output wire clk10,
	output wire clkaudio,
	output wire clk25,
	output wire clk50,
	output wire clkbus,
	output wire clk166,
	output wire clk200,
	output wire aresetn,
	output wire rst25n,
	output wire rst50n,
	output wire rstaudion,
	output wire preresetn);

// --------------------------------------------------
// PLLs / MMCMs
// --------------------------------------------------

wire centralclocklocked, peripheralclocklocked;

centralclock centralclockinst(
	.clk_in1(sys_clock_i),
	.clk100(),
	.clk10(clk10),
	.clk25(clk25),
	.clk50(clk50),
	.clk166(clk166),
	.clk200(clk200),
	.locked(centralclocklocked) );

peripheralclocks peripheralclkinst(
	.clk_in1(sys_clock_i),
	.clkaudio(clkaudio),
	.clkbus(clkbus),
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

(* async_reg = "true" *) logic rstn25A = 1'b1;
(* async_reg = "true" *) logic rstn25B = 1'b1;
always @(posedge clk25) begin
	rstn25A <= regaresetn;
	rstn25B <= rstn25A;
end

(* async_reg = "true" *) logic rstn50A = 1'b1;
(* async_reg = "true" *) logic rstn50B = 1'b1;
always @(posedge clk50) begin
	rstn50A <= regaresetn;
	rstn50B <= rstn50A;
end

(* async_reg = "true" *) logic rstaudionA = 1'b1;
(* async_reg = "true" *) logic rstaudionB = 1'b1;
always @(posedge clkaudio) begin
	rstaudionA <= regaresetn;
	rstaudionB <= rstaudionA;
end

assign rst25n = rstn25B;
assign rst50n = rstn50B;
assign rstaudion = rstaudionB;
assign aresetn = regaresetn;
assign preresetn = regpreresetn;

endmodule
