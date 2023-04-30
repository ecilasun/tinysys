`timescale 1ns / 1ps

`include "shared.vh"

module instructiondecompressor(
    input wire [15:0] instr_lowword,
    input wire [15:0] instr_highword,
    output reg is_compressed,
    output reg [31:0] fullinstr
);

always @ (*) begin
	if (instr_lowword[1:0] == 2'b11) begin

		// Already decompressed
		is_compressed = 1'b0;

		fullinstr = {instr_highword, instr_lowword};

	end else begin

		// Needs decompression
		is_compressed = 1'b1;

		case ({instr_lowword[15:13], instr_lowword[1:0]})
			`INSTR_CADDI4SPN: begin // rd = (zero extended nonzero immediate)*4+sp
				if (instr_lowword[12:2] != 11'h0 && instr_lowword[12:5] != 8'h0)
					fullinstr = { 2'b00, instr_lowword[10:7], instr_lowword[12:11], instr_lowword[5], instr_lowword[6], 2'b00, 5'd2, 3'b000, 2'b01, instr_lowword[4:2], `INSTR_ADDI }; // CADDI4SPN
			end

			`INSTR_CLW: begin // load word
				fullinstr = { 5'b00000, instr_lowword[5], instr_lowword[12:10], instr_lowword[6], 2'b00, 2'b01, instr_lowword[9:7], 3'b010, 2'b01, instr_lowword[4:2], `INSTR_LW }; // CLW
			end

			`INSTR_CSW: begin // store word
				fullinstr = { 5'b00000, instr_lowword[5], instr_lowword[12], 2'b01, instr_lowword[4:2], 2'b01, instr_lowword[9:7], 3'b010, instr_lowword[11:10], instr_lowword[6], 2'b00, `INSTR_SW }; // CSW
			end

			`INSTR_CNOP: begin // noop == addi x0,x0,0
				if (instr_lowword[12:2] == 11'h0)
					fullinstr = { 25'h0, `ADDI }; // CNOP
				else if (instr_lowword[12] != 1'b0 || instr_lowword[6:2] != 5'h0)
					fullinstr = { {7{instr_lowword[12]}}, instr_lowword[6:2], instr_lowword[11:7], 3'b000, instr_lowword[11:7], `INSTR_ADDI }; // CADDI
			end

			`INSTR_CJAL: begin // jump and link register
				fullinstr = { instr_lowword[12], instr_lowword[8], instr_lowword[10:9], instr_lowword[6], instr_lowword[7], instr_lowword[2], instr_lowword[11], instr_lowword[5:3], instr_lowword[12], {8{instr_lowword[12]}}, 5'd1, `INSTR_JAL }; // CJAL
			end

			`INSTR_CLI: begin // load immediate
				if (instr_lowword[11:7] != 5'd0)
					fullinstr = { {7{instr_lowword[12]}}, instr_lowword[6:2], 5'd0, 3'b000, instr_lowword[11:7], `INSTR_ADDI }; // CLI
			end

			`INSTR_CADDI16SP: begin
				if ((instr_lowword[12] != 1'b0 || instr_lowword[6:2] != 5'h0) && instr_lowword[11:7] != 5'd0) begin
					if (instr_lowword[11:7] == 5'd2)
						fullinstr = { {3{instr_lowword[12]}}, instr_lowword[4], instr_lowword[3], instr_lowword[5], instr_lowword[2], instr_lowword[6], 4'b0000, 5'd2, 3'b000, 5'd2, `INSTR_ADDI }; // CADDI16SP
					else
						fullinstr = { {15{instr_lowword[12]}}, instr_lowword[6:2], instr_lowword[11:7], 7'b0110111 }; // CLUI
				end
			end

			`INSTR_CSRLI: begin // shift right logical immediate
				if (instr_lowword[12:10] == 3'b011 && instr_lowword[6:5] == 2'b00)
					fullinstr = { 7'b0100000, 2'b01, instr_lowword[4:2], 2'b01, instr_lowword[9:7], 3'b000, 2'b01, instr_lowword[9:7], `INSTR_SUB }; // CSUB
				else if (instr_lowword[12:10] == 3'b011 && instr_lowword[6:5] == 2'b01)
					fullinstr = { 7'b0000000, 2'b01, instr_lowword[4:2], 2'b01, instr_lowword[9:7], 3'b100, 2'b01, instr_lowword[9:7], `INSTR_XOR }; // CXOR
				else if (instr_lowword[12:10] == 3'b011 && instr_lowword[6:5] == 2'b10)
					fullinstr = { 7'b0000000, 2'b01, instr_lowword[4:2], 2'b01, instr_lowword[9:7], 3'b110, 2'b01, instr_lowword[9:7], `INSTR_OR }; // COR
				else if (instr_lowword[12:10] == 3'b011 && instr_lowword[6:5] == 2'b11)
					fullinstr = { 7'b0000000, 2'b01, instr_lowword[4:2], 2'b01, instr_lowword[9:7], 3'b111, 2'b01, instr_lowword[9:7], `INSTR_AND }; // CAND
				else if (instr_lowword[11:10] == 2'b10)
					fullinstr = { {7{instr_lowword[12]}}, instr_lowword[6:2], 2'b01, instr_lowword[9:7], 3'b111, 2'b01, instr_lowword[9:7], `INSTR_ANDI }; // CANDI
				else if (instr_lowword[12] == 1'b0 && instr_lowword[6:2] == 5'h0)
					fullinstr = 32'h0; // UNDEF!
				else if (instr_lowword[11:10] == 2'b00)
					fullinstr = { 7'b0000000, instr_lowword[6:2], 2'b01, instr_lowword[9:7], 3'b101, 2'b01, instr_lowword[9:7], `INSTR_SRLI }; // CSRLI
				else if (instr_lowword[11:10] == 2'b01)
					fullinstr = { 7'b0100000, instr_lowword[6:2], 2'b01, instr_lowword[9:7], 3'b101, 2'b01, instr_lowword[9:7], `INSTR_SRAI }; // CSRAI
			end

			`INSTR_CJ: begin // jump
				fullinstr = { instr_lowword[12], instr_lowword[8], instr_lowword[10:9], instr_lowword[6], instr_lowword[7], instr_lowword[2], instr_lowword[11], instr_lowword[5:3], instr_lowword[12], {8{instr_lowword[12]}}, 5'd0, `INSTR_JAL }; // CJ
			end

			`INSTR_CBEQZ: begin // branch if equal to zero
				fullinstr = { {4{instr_lowword[12]}}, instr_lowword[6], instr_lowword[5], instr_lowword[2], 5'd0, 2'b01, instr_lowword[9:7], 3'b000, instr_lowword[11], instr_lowword[10], instr_lowword[4], instr_lowword[3], instr_lowword[12], `INSTR_BEQ }; // CBEQZ
			end

			`INSTR_CBNEZ: begin // branch if not equal to zero
				fullinstr = { {4{instr_lowword[12]}}, instr_lowword[6], instr_lowword[5], instr_lowword[2], 5'd0, 2'b01, instr_lowword[9:7], 3'b001, instr_lowword[11], instr_lowword[10], instr_lowword[4], instr_lowword[3], instr_lowword[12], `INSTR_BNE }; // CBNEZ
			end

			`INSTR_CSLLI: begin // shift left logical immediate
				if (instr_lowword[11:7] != 5'd0)
					fullinstr = { 7'b0000000, instr_lowword[6:2], instr_lowword[11:7], 3'b001, instr_lowword[11:7], `INSTR_SLLI }; // CSLLI
			end

			`INSTR_CLWSP: begin // load word relative to stack pointer
				if (instr_lowword[11:7] != 5'h0) // rd!=0
					fullinstr = { 4'b0000, instr_lowword[3:2], instr_lowword[12], instr_lowword[6:4], 2'b0, 5'd2, 3'b010, instr_lowword[11:7], `INSTR_LW }; // CLWSP
			end

			`INSTR_CSWSP: begin // store word relative to stack pointer
				fullinstr = { 4'b0000, instr_lowword[8:7], instr_lowword[12], instr_lowword[6:2], 5'd2, 3'b010, instr_lowword[11:9], 2'b00, `INSTR_SW }; // CSWSP
			end

			`INSTR_CJR: begin // jump register
				if (instr_lowword[6:2] == 5'd0) begin
					if (instr_lowword[11:7] == 5'h0) begin
						if (instr_lowword[12] == 1'b1)
							fullinstr = { 11'h0, 1'b1, 13'h0, `INSTR_EBREAK }; // CEBREAK
					end else if (instr_lowword[12])
						fullinstr = { 12'h0, instr_lowword[11:7], 3'b000, 5'd1, `INSTR_JALR }; // CJALR
				else
					fullinstr = { 12'h0, instr_lowword[11:7], 3'b000, 5'd0, `INSTR_JALR }; // CJR
				end else if (instr_lowword[11:7] != 5'h0) begin
					if (instr_lowword[12] == 1'b0)
						fullinstr = { 7'b0000000, instr_lowword[6:2], 5'd0, 3'b000, instr_lowword[11:7], `INSTR_ADD }; // CMV
					else
						fullinstr = { 7'b0000000, instr_lowword[6:2], instr_lowword[11:7], 3'b000, instr_lowword[11:7], `INSTR_ADD }; // CADD
				end
			end

			default: begin
				fullinstr = 32'd0; // UNDEF
			end
		endcase

	end
end

endmodule