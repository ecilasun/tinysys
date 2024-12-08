`timescale 1ns / 1ps
`default_nettype none

module axi4led(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	output wire [3:0] led );

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

logic ledwe = 1'b0;
logic [3:0] ledstate = 4'd0;

logic [3:0] ledbits = 4'd0;
always @(posedge aclk) begin
	if (ledwe)
		ledbits <= ledstate;
end

assign led = ledbits;

always @(posedge aclk) begin
	if (~delayedresetn) begin
		s_axi.awready <= 1'b0;
		s_axi.arready <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
		s_axi.bresp <= 2'b00;
		ledwe <= 1'b0;
	end else begin
		s_axi.awready <= s_axi.awvalid;
		s_axi.arready <= s_axi.arvalid;
		s_axi.bvalid <= s_axi.bready;
		s_axi.rvalid <= s_axi.rready;
		s_axi.wready <= s_axi.wvalid;
		ledwe <= 1'b0;
	
		if (s_axi.rready) begin
			s_axi.rdata[31:0] <= {28'd0, ledstate};
			s_axi.rlast <= 1'b1;
		end
	
		if (s_axi.wvalid) begin
			ledwe <= 1'b1;
			ledstate <= s_axi.wdata[3:0];
		end
	end
end

endmodule
