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
	// IRQ lines from CSR unit
	output wire irqHold,
	input wire irqReq,
	input wire [31:0] mepc,
	input wire [31:0] mtvec,
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
logic icacheflushpending = 1'b0;
logic irqpending = 1'b0;
logic processingIRQ = 1'b0;

assign irqHold = processingIRQ;

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
// Instruction injection ROM
// --------------------------------------------------

logic [3:0] injectAddr = 0;
logic [3:0] injectCount = 0;

logic [31:0] injectionROM [0:15];

initial begin
	// enterTimerISR
	injectionROM[0] = 32'hffc10113;
	injectionROM[1] = 32'h00f12023;
	injectionROM[2] = 32'h00000797;
	injectionROM[3] = 32'h341797f3;
	injectionROM[4] = 32'h08000793;
	injectionROM[5] = 32'h3047b7f3;
	injectionROM[6] = 32'h3007a7f3;
	injectionROM[7] = 32'h00800793;
	injectionROM[8] = 32'h3047b7f3;

	// leaveTimerISR
	injectionROM[9]  = 32'h08000793;
	injectionROM[10] = 32'h3007b7f3;
	injectionROM[11] = 32'h3007a7f3;
	injectionROM[12] = 32'h00800793;
	injectionROM[13] = 32'h3047a7f3;
	injectionROM[14] = 32'h00012783;
	injectionROM[15] = 32'h00410113;
end

wire [31:0] injectInstruction = injectionROM[injectAddr];

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
wire ismret = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_MRET);
wire isflush = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_CFLUSH);
wire needsToStall = isbranch || isfence || isdiscard;

// --------------------------------------------------
// Fetch logic
// --------------------------------------------------

typedef enum logic [3:0] {
	INIT,
	FETCH, STREAMOUT,
	WAITNEWBRANCHTARGET, WAITIFENCE,
	ENTERISR, EXITISR,
	STARTINJECT, INJECT, POSTENTER, POSTEXIT } fetchstate;

fetchstate fetchmode = INIT;
fetchstate injectEnd = FETCH;	// Where to go after injection ends

always @(posedge aclk) begin
	if (~aresetn) begin
		fetchmode <= INIT;
		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
		processingIRQ <= 1'b0;
	end else begin

		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
		icacheflush <= 1'b0;

		unique case(fetchmode)
			INIT: begin
				fetchena <= 1'b1;
				fetchmode <= FETCH;
			end

			FETCH: begin
				fetchmode <= rready ? STREAMOUT : FETCH;
				IR <= instruction;
				icacheflushpending <= isfence;
				irqpending <= irqReq;
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

				// Flush I$ if we have an IFENCE instruction and go to wait
				icacheflush <= icacheflushpending;

				// Go to appropriate wait mode or resume FETCH
				// Stop fetching if we need to halt, running IFENCE, or entering/exiting an ISR
				priority case (1'b1)
					icacheflushpending:	begin fetchmode <= WAITIFENCE;			fetchena <= 1'b0; end
					needsToStall:		begin fetchmode <= WAITNEWBRANCHTARGET;	fetchena <= 1'b0; end
					ismret:				begin fetchmode <= EXITISR;				fetchena <= 1'b0; end
					irqpending:			begin fetchmode <= ENTERISR;			fetchena <= 1'b0; end
					default:			begin fetchmode <= FETCH;				fetchena <= 1'b1; end
				endcase
			end

			WAITNEWBRANCHTARGET: begin
				// Branch resolve and cache operation completion will signal branch address resolved
				fetchena <= branchresolved;
				PC <= branchresolved ? branchtarget : PC;
				fetchmode <= branchresolved ? FETCH : WAITNEWBRANCHTARGET;
			end

			WAITIFENCE: begin
				// Resume fetch when I$ signals ready for pending flush
				fetchena <= rready;
				fetchmode <= rready ? FETCH : WAITIFENCE;
			end

			ENTERISR: begin
				// Hold further IRQ requests on this line until we encounter an mret
				processingIRQ <= 1'b1;

				// Inject entry instruction sequence
				injectAddr <= 0;
				injectCount <= 9;
				fetchmode <= STARTINJECT;
				injectEnd <= POSTENTER;
			end

			EXITISR: begin
				// Inject exit instruction sequence
				injectAddr <= 9;
				injectCount <= 7;
				fetchmode <= STARTINJECT;
				injectEnd <= POSTEXIT;

				// Release our hold so we can receive further interrupts
				processingIRQ <= 1'b0;
			end

			STARTINJECT: begin
				IR <= injectInstruction;
				injectAddr <= injectAddr + 1;
				injectCount <= injectCount - 1;
				fetchmode <= INJECT;
			end

			INJECT: begin
				ififowr_en <= 1'b1;
				ififodin <= {
					rs3, func7, csroffset,
					func12, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					PC, immed}; // Using the same PC+32'd4 from the time of IRQ

				fetchmode <= injectCount == 0 ? injectEnd : STARTINJECT;
			end

			POSTENTER: begin
				// Set up a jump to ISR
				PC <= mtvec;
				// Wait for fifo to flush completely
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTENTER;
			end

			POSTEXIT: begin
				// Restore PC to MEPC via CSR
				PC <= mepc;
				// Resume
				fetchena <= 1'b1;
				fetchmode <= FETCH;
			end

		endcase

	end
end

endmodule
