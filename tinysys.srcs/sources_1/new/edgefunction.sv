`timescale 1ns / 1ps

module edgefunction(
	input wire aclk,
	input wire aresetn,
	// At point
	input wire signed [17:0] px,
	input wire signed [17:0] py,
	// Edge
	input wire signed [17:0] x0,
	input wire signed [17:0] y0,
	input wire signed [17:0] x1,
	input wire signed [17:0] y1,
	// Edge function coefficients
	output wire signed [17:0] A_out,
	output wire signed [17:0] B_out,
	output wire signed [17:0] W_out );

logic signed [17:0] A;
logic signed [17:0] B;
logic signed [17:0] C;

logic signed [17:0] W;

always @(posedge aclk) begin
	if (~aresetn) begin
		A <= 18'd0;
		B <= 18'd0;
		C <= 18'd0;
		W <= 18'd0;
	end else begin
		// clk1
		A <= y0 - y1;
		B <= x1 - x0;
		C <= x0*y1 - y0*x0;
		// clk2
		W <= A*px + B*py + C;
	end
end

assign A_out = A;
assign B_out = B;
assign W_out = W;

endmodule
