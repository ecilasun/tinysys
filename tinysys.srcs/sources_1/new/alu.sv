`timescale 1ns / 1ps

`include "shared.vh"

module arithmeticlogic (
	output logic [31:0] aluout,
	input wire [31:0] val1,
	input wire [31:0] val2,
	input wire [3:0] aluop );

wire [9:0] aluonehot = {
	aluop == `ALU_ADD  ? 1'b1 : 1'b0,
	aluop == `ALU_SUB  ? 1'b1 : 1'b0,
	aluop == `ALU_SLL  ? 1'b1 : 1'b0,
	aluop == `ALU_SLT  ? 1'b1 : 1'b0,
	aluop == `ALU_SLTU ? 1'b1 : 1'b0,
	aluop == `ALU_XOR  ? 1'b1 : 1'b0,
	aluop == `ALU_SRL  ? 1'b1 : 1'b0,
	aluop == `ALU_SRA  ? 1'b1 : 1'b0,
	aluop == `ALU_OR   ? 1'b1 : 1'b0,
	aluop == `ALU_AND  ? 1'b1 : 1'b0 };

logic [31:0] vsum;
logic [31:0] vdiff;
logic [31:0] vshl;
logic [31:0] vsless;
logic [31:0] vless;
logic [31:0] vxor;
logic [31:0] vshr;
logic [31:0] vsra;
logic [31:0] vor;
logic [31:0] vand;

always_comb begin
	vsum = val1 + val2;
end

always_comb begin
	vdiff = val1 + (~val2 + 32'd1); // val1 - val2;
end

always_comb begin
	vshl = val1 << val2[4:0];
end

always_comb begin
	vsless = $signed(val1) < $signed(val2) ? 32'd1 : 32'd0;
end

always_comb begin
	vless = val1 < val2 ? 32'd1 : 32'd0;
end

always_comb begin
	vxor = val1 ^ val2;
end

always_comb begin
	vshr = val1 >> val2[4:0];
end

always_comb begin
	vsra = $signed(val1) >>> val2[4:0];
end

always_comb begin
	vor = val1 | val2;
end

always_comb begin
	vand = val1 & val2;
end

always_comb begin
	priority case(1'b1)
		// integer ops
		aluonehot[0]:	aluout = vand;
		aluonehot[1]:	aluout = vor;
		aluonehot[2]:	aluout = vsra;
		aluonehot[3]:	aluout = vshr;
		aluonehot[4]:	aluout = vxor;
		aluonehot[5]:	aluout = vless;
		aluonehot[6]:	aluout = vsless;
		aluonehot[7]:	aluout = vshl;
		aluonehot[8]:	aluout = vdiff;
		aluonehot[9]:	aluout = vsum;
		default:		aluout = 32'd0;
	endcase
end

endmodule
