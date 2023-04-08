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
	Name			   Offset      End
	enterTimerISR      0           11
	leaveTimerISR      12          18
	enterHWISR         19          32
	leaveHWISR         33          40
	enterEcallISR      41          52
	leaveEcallISR      53          60
	enterEbreakISR     61          72
	leaveEbreakISR     73          80
	enterIllegalISR    81          92
	leaveIllegalISR    93          100
*/

logic [6:0] injectAddr = 7'd0;
logic [6:0] injectStop = 7'd0;

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
fetchstate postInject = FETCH;	// Where to go after injection ends

always @(posedge aclk) begin
	if (~aresetn) begin
		fetchmode <= INIT;
		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
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
				ififowr_en <= (~isfence) && ~(|irqReq);
				ififodin <= {
					rs3, func7, csroffset,
					func12, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					PC, immed};

				unique case (1'b1)
				    // IRQ/EBREAK/ILLEGAL don't step the PC
					((|irqReq) || isebreak || isillegalinstruction):	PC <= PC + 32'd0;
					// No compressed instruction support:
					default:											PC <= PC + 32'd4;
					// Compressed instruction support:
					//default:											PC <= PC + IR[1:0]==2'b11 ? 32'd4 : 32'd2;
				endcase

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
					ismret:					begin fetchmode <= EXITISR;				fetchena <= 1'b0; end
					iswfi:					begin fetchmode <= WFI;					fetchena <= 1'b0; end
					isbranch:				begin fetchmode <= WAITNEWBRANCHTARGET;	fetchena <= 1'b0; end
					isdiscard:				begin fetchmode <= WAITNEWBRANCHTARGET;	fetchena <= 1'b0; end
					isflush:				begin fetchmode <= WAITNEWBRANCHTARGET;	fetchena <= 1'b0; end
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
				// Inject entry instruction sequence (see table at microcode ROM section)
				unique case (1'b1)
					irqReq[0]: begin // Interrupt:Timer
						injectAddr <= 7'd0;
						injectStop <= 7'd12;
					end
					irqReq[1]: begin // Interrupt:Ext
						injectAddr <= 7'd19;
						injectStop <= 7'd33;
					end
					isecall: begin // Exception: Environment call
						injectAddr <= 7'd41;
						injectStop <= 7'd53;
					end
					isebreak: begin // Exception: Debug breakpoint
						injectAddr <= 7'd61;
						injectStop <= 7'd73;
					end
				    isillegalinstruction: begin // Exception: Illegal instruction
						injectAddr <= 7'd81;
						injectStop <= 7'd93;
					end
				endcase

				fetchmode <= STARTINJECT;
				postInject <= POSTENTER;
			end

			EXITISR: begin
				// Inject exit instruction sequence (see table at microcode ROM section)
				unique case (1'b1)
					irqReq[0]: begin // Interrupt:Timer
						injectAddr <= 7'd12;
						injectStop <= 7'd19;
					end
					irqReq[1]: begin // Interrupt:Ext
						injectAddr <= 7'd33;
						injectStop <= 7'd41;
					end
					isecall: begin // Exception: Environment call
						injectAddr <= 7'd53;
						injectStop <= 7'd61;
					end
					isebreak: begin // Exception: Debug breakpoint
						injectAddr <= 7'd73;
						injectStop <= 7'd81;
					end
				    isillegalinstruction: begin // Exception: Illegal instruction
						injectAddr <= 7'd93;
						injectStop <= 7'd101;
					end
				endcase

				fetchmode <= STARTINJECT;
				postInject <= POSTEXIT;
			end

			STARTINJECT: begin
				// NOTE: This destroys decoded values from the actual instruction
				IR <= injectInstruction;
				injectAddr <= injectAddr + 1;
				fetchmode <= injectAddr == injectStop ? postInject : INJECT;
			end

			INJECT: begin
				ififowr_en <= 1'b1;
				ififodin <= {
					rs3, func7, csroffset,
					func12, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					PC, immed};

				// WARNING: Injection ignores all instruction handling and never advances the PC
				fetchmode <= STARTINJECT;
			end

			POSTENTER: begin
				// TODO: Support for vectored jump using address encoding:
				// BASE[31:2] MODE[1:0] (modes-> 00:direct, 01:vectored, 10/11:reserved)
				// where exceptions jump to BASE, interrupts jump to BASE+exceptioncode*4
				// NOTE: OS has to hold a vectored interrupt table at the BASE address

				// Set up a jump to ISR
				PC <= mtvec;

				// This ensures that the entry routine turns off global interrupts
				// so that the next time around we don't get irqReq, and also any
				// change to mtvec to be reflected properly
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTENTER;
			end

			POSTEXIT: begin
				// Restore PC to MEPC via CSR
				PC <= mepc;

				// This ensures that the entry routine turns on global interrupts
				// so that the next time around we get irqReq, and also that a
				// change to mepc by a task manager is reflected properly
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTEXIT;
			end

			WFI: begin
				// NOTE: When global interrupts or machine external interrupts are disabled,
				// this will end up holding the core in a suspended state.
				fetchena <= (|irqReq);
				fetchmode <= (|irqReq) ? FETCH : WFI;
			end

		endcase

	end
end

endmodule
