`timescale 1ns / 1ps

`include "shared.vh"

module branchlogic(
	input wire aresetn,
	output logic branchout,
	input wire [31:0] val1,
	input wire [31:0] val2,
	input wire [2:0] bluop);

logic branch = 1'b0;

wire [4:0] bluonehot = {
	/*bluop == `BLU_EQ  ? 1'b1 : 1'b0,*/
	bluop == `BLU_NE  ? 1'b1 : 1'b0,
	bluop == `BLU_L   ? 1'b1 : 1'b0,
	bluop == `BLU_GE  ? 1'b1 : 1'b0,
	bluop == `BLU_LU  ? 1'b1 : 1'b0,
	bluop == `BLU_GEU ? 1'b1 : 1'b0 };

logic eq, sless, less;

always_comb begin
	eq = val1 == val2 ? 1'b1 : 1'b0;
	sless = $signed(val1) < $signed(val2) ? 1'b1 : 1'b0;
	less = val1 < val2 ? 1'b1 : 1'b0;
end

always_comb begin
	if (~aresetn) begin
		branchout = 1'b0;
	end else begin
		priority case(1'b1)
			// branch alu
			bluonehot[0]:	branchout = ~less;
			bluonehot[1]:	branchout = less;
			bluonehot[2]:	branchout = ~sless;
			bluonehot[3]:	branchout = sless;
			bluonehot[4]:	branchout = ~eq;
			default:		branchout = eq; // bluonehot[5]
		endcase
	end
end

endmodule
