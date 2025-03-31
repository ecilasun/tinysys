`timescale 1ns / 1ps
`default_nettype none

module axi4register(
	input wire aclk,
	input wire clk10,
	input wire aresetn,
	input wire rst10n,
	axi4if.slave s_axi,
	output wire [5:0] regio );

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// Register control
// --------------------------------------------------

logic regwe;
logic [5:0] regin;

logic [5:0] regstate;

initial begin
	regstate = 6'd0;
end

always @(posedge aclk) begin
	if (regwe)
		regstate <= regin;
end

(* async_reg = "true" *) logic [5:0] regstateA;
(* async_reg = "true" *) logic [5:0] regstateB;

always @(posedge clk10) begin
	if (~rst10n) begin
		regstateA <= 6'd0;
		regstateB <= 6'd0;
	end else begin
		regstateA <= regstate;
		regstateB <= regstateA;
	end
end

assign regio = regstateB;

always @(posedge aclk) begin
	if (~delayedresetn) begin
		s_axi.awready <= 1'b0;
		s_axi.arready <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
		s_axi.bresp <= 2'b00;
		regwe <= 1'b0;
		regin <= 6'd0;
	end else begin
		s_axi.awready <= s_axi.awvalid;
		s_axi.arready <= s_axi.arvalid;
		s_axi.bvalid <= s_axi.bready;
		s_axi.rvalid <= s_axi.rready;
		s_axi.wready <= s_axi.wvalid;
		regwe <= 1'b0;

		if (s_axi.rready) begin
			s_axi.rdata[31:0] <= {26'd0, regstate};
			s_axi.rlast <= 1'b1;
		end

		if (s_axi.wvalid) begin
			regwe <= 1'b1;
			regin <= s_axi.wdata[5:0];
		end
	end
end

endmodule
