`timescale 1ns / 1ps

module edgefunction(
	// At point
	input wire signed [15:0] px,
	input wire signed [15:0] py,
	// Edge
	input wire signed [15:0] x0,
	input wire signed [15:0] y0,
	input wire signed [15:0] x1,
	input wire signed [15:0] y1,
	// Edge function coefficients
	output wire signed [15:0] A_out,
	output wire signed [15:0] B_out,
	output wire signed [31:0] W_out );

logic signed [15:0] A = y0 - y1;
logic signed [15:0] B = x1 - x0;
logic signed [31:0] C = x0*y1 - y0*x0;

logic signed [31:0] W = A*px + B*py + C;

assign A_out = A;
assign B_out = B;
assign W_out = W;

endmodule
