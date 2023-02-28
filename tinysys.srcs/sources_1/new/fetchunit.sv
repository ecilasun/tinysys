`timescale 1ns / 1ps

`include "shared.vh"

module fetchunit #(
	parameter int RESETVECTOR = 32'h20000000
) (
	input wire aclk,
	input wire aresetn,
	// Stall control
	input wire branchresolved,
	input wire [31:0] branchtarget,
	// Output FIFO control
	output wire ififoempty,
	output wire ififovalid,
	output wire [143:0] ififodout,
	input wire ififord_en, 
	// To system bus
	axi4if.master m_axi );

// --------------------------------------------------
// Internal states
// --------------------------------------------------

logic fetchena = 1'b0;
logic [31:0] PC = RESETVECTOR;
logic [31:0] IR;
wire rready;
wire [31:0] instruction;
logic icacheflush = 1'b0;

// --------------------------------------------------
// Instruction cache
// --------------------------------------------------

instructioncache instructioncacheinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.addr(PC),
	.icacheflush(icacheflush),
	.dout(instruction),
	.ren(fetchena),
	.rready(rready),
	.m_axi(m_axi) );

// --------------------------------------------------
// Pre-decoder
// --------------------------------------------------

wire [17:0] instrOneHotOut;
wire [3:0] aluop;
wire [2:0] bluop;
wire [2:0] func3;
wire [6:0] func7;
wire [11:0] func12;
wire [4:0] rs1, rs2, rs3, rd;
wire [11:0] csroffset;
wire [31:0] immed;
wire selectimmedasrval2;

decoder decoderinst(
	.instruction(IR),
	.instrOneHotOut(instrOneHotOut),			// 18	+
	.aluop(aluop),								// 4	+
	.bluop(bluop),								// 3	+
	.func3(func3),								// 3	+
	.func7(func7),								// 7	+
	.func12(func12),							// 12	+
	.rs1(rs1),									// 5	+
	.rs2(rs2),									// 5	+
	.rs3(rs3),									// 5	+
	.rd(rd),									// 5	+
	.csroffset(csroffset),						// 12	+
	.immed(immed),								// 32	+
	.selectimmedasrval2(selectimmedasrval2) );	// 1	+ -> 18+4+3+3+7+12+5+5+5+5+12+32+1+PC[31:0] = 144 bits (Exact *8 of 18)

// --------------------------------------------------
// Instruction output FIFO
// --------------------------------------------------

logic [143:0] ififodin;
logic ififowr_en = 1'b0;
wire ififofull;

instructinfifo instructionfifoinst (
  .clk(aclk),
  .rst(~aresetn),
  .din(ififodin),
  .wr_en(ififowr_en),
  .rd_en(ififord_en),
  .dout(ififodout),
  .full(ififofull),
  .empty(ififoempty),
  .valid(ififovalid) );

  // --------------------------------------------------
// Instruction classification and halt detection
// --------------------------------------------------

// Reasons to stall fetch activity until control unit executes current instruction and flags branchresolved
wire isbranch = instrOneHotOut[`O_H_JAL] || instrOneHotOut[`O_H_JALR] || instrOneHotOut[`O_H_BRANCH];
wire isfence = instrOneHotOut[`O_H_FENCE];
wire isdiscard = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_CDISCARD);
wire isflush = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_CFLUSH);
wire ismret = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_MRET);

wire needstohalt = isbranch || isfence || isdiscard || isflush || ismret;

// --------------------------------------------------
// Fetch logic
// --------------------------------------------------

typedef enum logic [1:0] { INIT, FETCH, STREAMOUT, WAITNEWBRANCHTARGET } fetchstate;
fetchstate fetchmode = INIT;

always @(posedge aclk) begin
	if (~aresetn) begin
		fetchmode <= INIT;
		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
	end else begin

		fetchena <= 1'b0;
		ififowr_en <= 1'b0;

		unique case(fetchmode)
			INIT: begin
				fetchena <= 1'b1;
				fetchmode <= FETCH;
			end

			FETCH: begin
				fetchmode <= rready ? STREAMOUT : FETCH;
				IR <= instruction;
			end

			STREAMOUT: begin
				// Emit decoded instruction
				ififowr_en <= 1'b1;
				ififodin <= {
					rs3, func7, csroffset,
					func12, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					PC, immed};

				// No compressed instruction support:
				PC <= PC + 32'd4;

				// Compressed instruction support:
				// Step 2 bytes for compressed instruction
				//PC <= PC + IR[1:0]==2'b11 ? 32'd4 : 32'd2;

				fetchena <= ~needstohalt;
				fetchmode <= ~needstohalt ? FETCH : WAITNEWBRANCHTARGET;
			end

			WAITNEWBRANCHTARGET: begin
				// Branch resolve and cache operation completion will signal branch address resolved
				fetchena <= branchresolved;
				PC <= branchresolved ? branchtarget : PC;
				fetchmode <= branchresolved ? FETCH : WAITNEWBRANCHTARGET;
			end

		endcase

	end
end

endmodule
