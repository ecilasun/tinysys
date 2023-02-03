`timescale 1ns / 1ps

module tophat(
    input sys_clk,
    input sys_rst_n,
	output wire uart_rxd_out,
	input wire uart_txd_in,
    output wire [1:0] leds);

// --------------------------------------------------
// Clock and reset generator
// --------------------------------------------------

wire aresetn;
wire preresetn, calib_done;
wire aclk, clk15;

// Clock and reset generator
clockandreset clockandresetinst(
	.sys_clock_i(sys_clk),
	.sys_resetn(sys_rst_n),
	.aclk(aclk),
	.clk15(clk15),
	.calib_done(calib_done),
	.preresetn(preresetn),	// TODO: Use as reset signal for devices that we need initialized before the CPU/GPU such as SDRAM
	.aresetn(aresetn));

// TODO: In normal operation,t this will be set high by the SDRAM controller when it's done calibrating
// Here we set it high manually, so that aresetn can assert in the absence of SDRAM controller.
assign calib_done = 1'b1;

// --------------------------------------------------
// Test unit
// --------------------------------------------------

typedef enum logic [1:0] { INIT, EXEC } machinestate;
machinestate mstat = INIT;
machinestate nextmstat = INIT;

logic [31:0] counter = 32'd0;
always @(posedge aclk or negedge aresetn) begin
	if (~aresetn) begin
		mstat <= INIT;
	end else begin
		mstat <= nextmstat;
	end
end

always_comb begin
	unique case (mstat)
		INIT: begin counter = 32'd0; nextmstat = EXEC; end
		EXEC: begin counter = counter + 32'd1; nextmstat = EXEC; end
	endcase
end

assign leds[0] = counter[25];
assign leds[1] = counter[24];

endmodule
