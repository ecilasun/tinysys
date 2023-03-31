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
	input wire [1:0] irqReq,
	input wire [31:0] mepc,
	input wire [31:0] mtvec,
	input wire sie,
	// ROM copy done
	input wire romReady,
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
// Microcode ROM
// NOTE: Not yet 'microcode' but eventually will be
// --------------------------------------------------

/*
	Table of injected function offsets and lengths
	Name			   Offset      Length
	enterTimerISR      0           12
	leaveTimerISR      12          7
	enterHWISR         19          14
	leaveHWISR         33          8
	enterEcallISR      41          12
	leaveEcallISR      53          8
	enterEbreakISR     61          12
	leaveEbreakISR     73          8
	enterIllegalISR    81          12
	leaveIllegalISR    93          8
*/

logic [6:0] injectAddr = 0;
logic [6:0] injectCount = 0;
logic [6:0] exitAddr = 0;
logic [6:0] exitCount = 0;

logic [31:0] injectionROM [0:100];

initial begin
	$readmemh("microcoderom.mem", injectionROM);
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
wire isflush = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_CFLUSH);
wire ismret = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_MRET);
wire iswfi = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_WFI);
wire isebreak = sie && instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_EBREAK);
wire isecall = sie && instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_ECALL);
wire isillegalinstruction = sie && ~(|instrOneHotOut);
wire needsToStall = isbranch || isdiscard || isflush;

// --------------------------------------------------
// Fetch logic
// --------------------------------------------------

typedef enum logic [3:0] {
	INIT,										// Startup
	FETCH, STREAMOUT,							// Regular instuction fetch + stream loop
	WAITNEWBRANCHTARGET, WAITIFENCE,			// Branch and fence handling
	ENTERISR, EXITISR,							// IRQ handling
	STARTINJECT, INJECT, POSTENTER, POSTEXIT,	// IRQ and post-IRQ maintenance
	WFI											// Wait state for IRQ
} fetchstate;

fetchstate fetchmode = INIT;
fetchstate injectEnd = FETCH;	// Where to go after injection ends

always @(posedge aclk) begin
	if (~aresetn) begin
		fetchmode <= INIT;
		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
		processingIRQ <= 1'b0;
		IR <= 32'd0;
	end else begin

		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
		icacheflush <= 1'b0;

		unique case(fetchmode)
			INIT: begin
				fetchena <= romReady;
				fetchmode <= romReady ? FETCH : INIT;
			end

			FETCH: begin
				IR <= instruction;
				fetchmode <= rready ? STREAMOUT : FETCH;
			end

			STREAMOUT: begin
				// Emit decoded instruction
				// - FENCE.I does not need to go to execute unit
				// - IRQ will ignore the current instruction and keep PC the same to process it after ISR
				// - EBREAK will repeatedly jump back to the same instruction site until the EBREAK is replaced
				ififowr_en <= ~isfence && ~(|irqReq);
				ififodin <= {
					rs3, func7, csroffset,
					func12, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					PC, immed};

				// No compressed instruction support:
				PC <= PC + ((|irqReq || isebreak || isillegalinstruction) ? 32'd0 : 32'd4);

				// Compressed instruction support:
				// Step 2 bytes for compressed instruction
				//PC <= PC + IR[1:0]==2'b11 ? 32'd4 : 32'd2;

				// Flush I$ if we have an IFENCE instruction and go to wait
				icacheflush <= isfence;

				// Go to appropriate wait mode or resume FETCH
				// Stop fetching if we need to halt, running IFENCE, or entering/exiting an ISR
				priority case (1'b1)
					(|irqReq):				begin fetchmode <= ENTERISR;			fetchena <= 1'b0; end
					isillegalinstruction:	begin fetchmode <= ENTERISR;			fetchena <= 1'b0; end
					isecall:				begin fetchmode <= ENTERISR;			fetchena <= 1'b0; end
					isebreak:				begin fetchmode <= ENTERISR;			fetchena <= 1'b0; end
					isfence:				begin fetchmode <= WAITIFENCE;			fetchena <= 1'b0; end
					needsToStall:			begin fetchmode <= WAITNEWBRANCHTARGET;	fetchena <= 1'b0; end
					ismret:					begin fetchmode <= EXITISR;				fetchena <= 1'b0; end
					iswfi:					begin fetchmode <= WFI;					fetchena <= 1'b0; end
					default:				begin fetchmode <= FETCH;				fetchena <= 1'b1; end
				endcase
			end

			WAITNEWBRANCHTARGET: begin
				// Resume fetch when branch address is resolved
				fetchena <= branchresolved;
				// New PC to resume fetch at
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

				// Inject entry instruction sequence (see table at microcode ROM section)
				unique case (1'b1)
				    isillegalinstruction:	begin injectAddr <= 7'd81; injectCount <= 7'd12; exitAddr <= 7'd93; exitCount <= 7'd8; end	// Software: illegal instruction
					isebreak:				begin injectAddr <= 7'd61; injectCount <= 7'd12; exitAddr <= 7'd73; exitCount <= 7'd8; end	// Software: debug breakpoint
					isecall:				begin injectAddr <= 7'd41; injectCount <= 7'd12; exitAddr <= 7'd53; exitCount <= 7'd8; end	// Software: environment call
					irqReq[1]:				begin injectAddr <= 7'd19; injectCount <= 7'd14; exitAddr <= 7'd33; exitCount <= 7'd8; end	// Ext
					irqReq[0]:				begin injectAddr <= 7'd0;  injectCount <= 7'd12; exitAddr <= 7'd12; exitCount <= 7'd7; end	// Timer
				endcase

				fetchmode <= STARTINJECT;
				injectEnd <= POSTENTER;
			end

			EXITISR: begin
				// Inject exit instruction sequence (see table at microcode ROM section)
				injectAddr <= exitAddr;
				injectCount <= exitCount;

				fetchmode <= STARTINJECT;
				injectEnd <= POSTEXIT;
			end

			STARTINJECT: begin
				// NOTE: This destroys decoded values from the actual instruction
				IR <= injectInstruction;
				injectAddr <= injectAddr + 1;
				injectCount <= injectCount - 1;
				fetchmode <= INJECT;
			end

			INJECT: begin
				// Ignore ifence, interrupts and never advance the PC
				ififowr_en <= 1'b1;
				ififodin <= {
					rs3, func7, csroffset,
					func12, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					PC, immed};

				fetchmode <= injectCount == 0 ? injectEnd : STARTINJECT;
			end

			POSTENTER: begin
				// Set up a jump to ISR
				// TODO: Support for vectored jump using address encoding:
				// BASE[31:2] MODE[1:0] (modes-> 00:direct, 01:vectored, 10/11:reserved)
				// where exceptions jump to BASE, interrupts jump to BASE+exceptioncode*4
				// NOTE: OS has to hold a vectored interrupt table at the BASE address
				PC <= mtvec;

				// We need to see the correct MTVEC so we wait for all pending
				// instructions to get executed first
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTENTER;
			end

			POSTEXIT: begin
				// Release our hold so we can receive further interrupts
				processingIRQ <= ~ififoempty;

				// Restore PC to MEPC via CSR
				PC <= mepc;

				// A task manager may have shuffled the mepc register
				// so we need to wait for instructions to fall through
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTEXIT;
			end

			WFI: begin
				fetchena <= (|irqReq);
				fetchmode <= (|irqReq) ? FETCH : WFI;
			end

		endcase

	end
end

endmodule
