`timescale 1ns / 1ps

`include "shared.vh"

module decoder(
	input wire [31:0] instruction,		// Raw input instruction
	output bit [17:0] instrOneHotOut,	// Current instruction class
	output bit [3:0] aluop,				// Current ALU op
	output bit [2:0] bluop,				// Current BLU op
	output bit [2:0] func3,				// Sub-instruction
	output bit [6:0] func7,				// Sub-instruction
	output bit [11:0] func12,			// Sub-instruction
	output bit [4:0] rs1,				// Source register one
	output bit [4:0] rs2,				// Source register two
	output bit [4:0] rs3,				// Used by fused multiplyadd/sub
	output bit [4:0] rd,				// Destination register
	output bit [11:0] csroffset,		// Index of selected CSR register
	output bit [31:0] immed,			// Unpacked immediate integer value
	output bit selectimmedasrval2		// Select rval2 or unpacked integer during EXEC
);

wire [17:0] instrOneHot = {
	instruction[6:0]==`OPCODE_OP ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_OP_IMM ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_AUIPC ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_LUI ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_STORE ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_LOAD ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_JAL ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_JALR ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_BRANCH ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FENCE ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_SYSTEM ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_OP ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_LDW ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_STW ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_MADD ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_MSUB ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_NMSUB ? 1'b1:1'b0,
	instruction[6:0]==`OPCODE_FLOAT_NMADD ? 1'b1:1'b0 };

always_comb begin
	csroffset = {instruction[31:25], instruction[24:20]};
end

// Immed vs rval2 selector
wire selector = instrOneHot[`O_H_JALR] || instrOneHot[`O_H_OP_IMM] || instrOneHot[`O_H_LOAD] || instrOneHot[`O_H_FLOAT_LDW] || instrOneHot[`O_H_FLOAT_STW] || instrOneHot[`O_H_STORE];

// Every instruction except SYS:3'b000, BRANCH, FPU ops and STORE are recoding form
// i.e. NOT (branch or store) OR (SYS AND at least one bit set)
wire isfpuopcode = 
	instrOneHot[`O_H_FLOAT_OP] ||
	instrOneHot[`O_H_FLOAT_LDW] ||
	instrOneHot[`O_H_FLOAT_STW] ||
	instrOneHot[`O_H_FLOAT_MADD] ||
	instrOneHot[`O_H_FLOAT_MSUB] ||
	instrOneHot[`O_H_FLOAT_NMSUB] ||
	instrOneHot[`O_H_FLOAT_NMADD];

// NOTE: Load _is_ recording form but it's delayed vs where we normaly flag 'recording', so it's omitted from list and handled mamually
//wire recordingform = ~(instrOneHot[`O_H_BRANCH] || instrOneHot[`O_H_LOAD] || instrOneHot[`O_H_STORE] || isfpuopcode) || (instrOneHot[`O_H_SYSTEM] & (|func3));

// Source/destination register indices
wire [4:0] src1 = instruction[19:15];
wire [4:0] src2 = instruction[24:20];
wire [4:0] src3 = instruction[31:27];
wire [4:0] dest = instruction[11:7];

// Sub-functions
wire [2:0] f3 = instruction[14:12];
wire [6:0] f7 = instruction[31:25];
wire [11:0] f12 = instruction[31:20];
wire mathopsel = instruction[30];
wire muldiv = instruction[25];

// Shift in decoded values
always_comb begin
	rs1 = src1;
	rs2 = src2;
	rs3 = src3;
	rd = dest;
	func3 = f3;
	func7 = f7;
	func12 = f12;
	instrOneHotOut = instrOneHot;
	selectimmedasrval2 = selector;	// Use rval2 or immed
end

// Work out ALU op
always_comb begin
	unique case(1'b1)
		instrOneHot[`O_H_OP]: begin
			unique case({muldiv, f3})
				4'b0_000: aluop = mathopsel ? `ALU_SUB : `ALU_ADD;
				4'b0_001: aluop = `ALU_SLL;
				4'b0_011: aluop = `ALU_SLTU;
				4'b0_010: aluop = `ALU_SLT;
				4'b0_110: aluop = `ALU_OR;
				4'b0_111: aluop = `ALU_AND;
				4'b0_101: aluop = mathopsel ? `ALU_SRA : `ALU_SRL;
				4'b0_100: aluop = `ALU_XOR;
				4'b1_000,
				4'b1_001,
				4'b1_010,
				4'b1_011: aluop = `ALU_MUL;
				4'b1_100,
				4'b1_101: aluop = `ALU_DIV;
				4'b1_110,
				4'b1_111: aluop = `ALU_REM;
				default:  aluop = `ALU_NONE;
			endcase
		end

		instrOneHot[`O_H_OP_IMM]: begin
			unique case(f3)
				3'b000: aluop = `ALU_ADD;
				3'b001: aluop = `ALU_SLL;
				3'b011: aluop = `ALU_SLTU;
				3'b010: aluop = `ALU_SLT;
				3'b110: aluop = `ALU_OR;
				3'b111: aluop = `ALU_AND;
				3'b101: aluop = mathopsel ? `ALU_SRA : `ALU_SRL;
				3'b100: aluop = `ALU_XOR;
				default: aluop = `ALU_NONE;
			endcase
		end

		default: begin
			aluop = `ALU_NONE;
		end
	endcase
end

// Work out BLU op
always_comb begin
	unique case(1'b1)
		instrOneHot[`O_H_BRANCH]: begin
			unique case(f3)
				3'b000: bluop = `BLU_EQ;
				3'b001: bluop = `BLU_NE;
				3'b011: bluop = `BLU_NONE;
				3'b010: bluop = `BLU_NONE;
				3'b110: bluop = `BLU_LU;
				3'b111: bluop = `BLU_GEU;
				3'b101: bluop = `BLU_GE;
				3'b100: bluop = `BLU_L;
			endcase
		end

		default: begin
			bluop = `BLU_NONE;
		end
	endcase
end

// Work out immediate value
always_comb begin
	unique case(1'b1)
		instrOneHot[`O_H_LUI], instrOneHot[`O_H_AUIPC]: begin	
			immed = {instruction[31], instruction[30:12], 12'd0};
		end

		instrOneHot[`O_H_FLOAT_STW], instrOneHot[`O_H_STORE]: begin
			immed = {{21{instruction[31]}}, instruction[30:25], instruction[11:7]};
		end

		instrOneHot[`O_H_JAL]: begin
			immed = {{12{instruction[31]}}, instruction[19:12], instruction[20], instruction[30:21], 1'b0};
		end

		instrOneHot[`O_H_BRANCH]: begin
			immed = {{20{instruction[31]}}, instruction[7], instruction[30:25], instruction[11:8], 1'b0};
		end

		instrOneHot[`O_H_OP_IMM], instrOneHot[`O_H_FLOAT_LDW], instrOneHot[`O_H_LOAD], instrOneHot[`O_H_JALR]: begin
			immed = {{21{instruction[31]}}, instruction[30:20]};
		end
	endcase
end

endmodule
