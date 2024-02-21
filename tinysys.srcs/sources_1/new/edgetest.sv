`timescale 1ns / 1ps

module edgetest(
	input wire aclk,
	input wire aresetn,
	input wire signed [17:0] N,
	input wire signed [17:0] sw0,
	input wire signed [17:0] sw1,
	input wire signed [17:0] sw2,
	input wire signed [17:0] sA12,
	input wire signed [17:0] sA20,
	input wire signed [17:0] sA01, 
	output wire testOut );

logic signed [17:0] E0;
logic signed [17:0] E1;
logic signed [17:0] E2;

logic edgeResult;
always @(posedge aclk) begin
	if (~aresetn)
		edgeResult <= 1'b0;
	else begin
		// clock1
		E0 <= sw0 + N*sA12;
		E1 <= sw1 + N*sA20;
		E2 <= sw2 + N*sA01;
		// clock2
		edgeResult <= E0[17] & E1[17] & E2[17];
	end
end

assign testOut = edgeResult;

endmodule
