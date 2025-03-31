`timescale 1ns / 1ps
`default_nettype none

`include "shared.vh"

module fetchunit #(
	parameter int RESETVECTOR = 32'd0
) (
	input wire aclk,
	input wire aresetn,
	// Stall control
	input wire branchresolved,
	input wire [31:0] branchtarget,
	// Output FIFO control
	output wire ififoempty,
	output wire ififovalid,
	output wire [131:0] ififodout,
	input wire ififord_en,
	// IRQ lines from CSR unit
	input wire [1:0] irqReq,
	input wire [31:0] mepc,
	input wire [31:0] mtvec,
	input wire sie,
	// CPU reset
	input wire cpuresetreq,
	// ROM copy done
	input wire romReady,
	// To system bus
	axi4if.master m_axi );

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// Internal states
// --------------------------------------------------

bit fetchena;
bit [3:0] wficount;
bit [31:0] prevPC;
bit [31:0] PC;
bit [31:0] emitPC;
bit [31:0] IR;
wire rready;
wire [31:0] instruction;
bit icacheflush;

// --------------------------------------------------
// Instruction cache
// --------------------------------------------------

instructioncache instructioncacheinst(
	.aclk(aclk),
	.aresetn(delayedresetn),
	.addr(PC),
	.icacheflush(icacheflush),
	.dout(instruction),
	.ren(fetchena),
	.rready(rready),
	.m_axi(m_axi) );

// --------------------------------------------------
// Pre-decoder
// --------------------------------------------------

wire [15:0] instrOneHotOut;
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
	.instrOneHotOut(instrOneHotOut),			// 16	+
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
	.selectimmedasrval2(selectimmedasrval2) );	// 1	+ -> 18+4+3+3+7+12+5+5+5+5+12+32+1+PC[31:1]+1(stepsize) = 144 bits (Exact *8 of 18)

// --------------------------------------------------
// Instruction injection address table
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

/*
For now, a software routine handles this
logic [31:0] mcause;
always @(*) begin
	case (1'b1)
		irqReq[0]				: begin mcause <= 32'h80000007; end  // timer
		irqReq[1]				: begin mcause <= 32'h8000000B; end  // hardware
		isecall					: begin mcause <= 32'h0000000B; end  // software - ecall
		isebreak				: begin mcause <= 32'h00000003; end  // software - ebreak
		isillegalinstruction	: begin mcause <= 32'h00000002; end  // software - illegal
		default 				: begin mcause <= 32'h00000000; end  // none
	endcase
end*/

bit [6:0] enterStartStop[0:11];
bit [6:0] exitStartStop[0:11];

initial begin
	$readmemh("entryoffsets.mem", enterStartStop);
	$readmemh("exitoffsets.mem", exitStartStop);
end

// --------------------------------------------------
// Instruction injection ROM
// --------------------------------------------------

bit [6:0] injectAddr;	// Instruction injection start and read address
bit [6:0] injectStop;	// Instruction injection stop address
bit [4:0] entryState;	// State at entry time

bit [31:0] injectionROM [0:100];

initial begin
	$readmemh("microcoderom.mem", injectionROM);
end

wire [31:0] injectInstruction = injectionROM[injectAddr];

// --------------------------------------------------
// Instruction output FIFO
// --------------------------------------------------

bit [131:0] ififodin;
bit ififowr_en = 1'b0;
wire ififofull;

instructinfifo instructionfifoinst (
  .clk(aclk),
  .rst(~delayedresetn || cpuresetreq),
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
wire isjal = instrOneHotOut[`O_H_JAL];
wire isbranch = instrOneHotOut[`O_H_JALR] || instrOneHotOut[`O_H_BRANCH];
wire isfence = instrOneHotOut[`O_H_FENCE];
wire isdiscard = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_CDISCARD);
wire isflush = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_CFLUSH);
wire ismret = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_MRET);
wire iswfi = instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_WFI);
wire isebreak = sie && instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_EBREAK);
wire isecall = sie && instrOneHotOut[`O_H_SYSTEM] && (func12 == `F12_ECALL);
wire isillegalinstruction = sie && ~(|instrOneHotOut);

wire [1:0] sysop = {
	{func3, func12} == {3'b000, `F12_CDISCARD} ? 1'b1 : 1'b0,
	{func3, func12} == {3'b000, `F12_CFLUSH} ? 1'b1 : 1'b0 };

// --------------------------------------------------
// Fetch state machine
// --------------------------------------------------

typedef enum logic [3:0] {
	INIT,										// Startup
	FETCH, STREAMOUT,							// Instuction fetch + stream loop
	WAITNEWBRANCHTARGET, WAITIFENCE,			// Branch and fence handling
	ENTERISR, EXITISR,							// ISR handling
	STARTINJECT, INJECT, POSTENTER, POSTEXIT,	// ISR entry/exit instruction injection
	WFI											// Wait state for IRQ
} fetchstate;

fetchstate fetchmode = INIT;
fetchstate postInject = FETCH;	// Where to go after injection ends

logic [4:0] exceptioncode;

always @(posedge aclk) begin
	if (~delayedresetn || cpuresetreq) begin
		PC <= 32'd0;
		prevPC <= 32'd0;
		emitPC <= 32'd0;
		injectAddr <= 7'd0;
		injectStop <= 7'd0;
		entryState <= 5'd0;
		wficount <= 4'd0;
		exceptioncode <= 5'd0;
		fetchmode <= INIT;
		IR <= 32'd0;
	end else begin
		fetchena <= 1'b0;
		ififowr_en <= 1'b0;
		icacheflush <= 1'b0;

		unique case(fetchmode)
			INIT: begin
				PC <= RESETVECTOR;
				// We do not proceed until the instruction fifo has been drained (soft reboot)
				// or the ROM has been copied over (hard reboot)
				fetchena <= romReady;
				fetchmode <= romReady ? FETCH : INIT;
			end

			FETCH: begin
				// Stalls when instruction fifo is full (unlikely but possible)
				fetchmode <= (rready && ~ififofull) ? STREAMOUT : FETCH;
				IR <= instruction;
				prevPC <= PC;
			end

			STREAMOUT: begin
				// Emit decoded instruction except:
				// - FENCE.I does not need to go to execute unit
				// - HWIRQs will ignore the current instruction and branch to ISR here after injection of a prologue
				// - EBREAK will repeatedly jump back to the same instruction site until the EBREAK is replaced

				// General interrupt handling rule is that interrupts must be processed before branches to avoid deadlocks in 1-instruction loops (jump-to-self)

				ififowr_en <= (~isfence) && (~irqReq[0]) && (~irqReq[1]) && (~isillegalinstruction) && (~isecall) && (~isebreak) && (~ismret) && (~iswfi);
				ififodin <= {
					csroffset,
					sysop, func3, func7,
					selectimmedasrval2,
					instrOneHotOut,
					bluop, aluop,
					rs1, rs2, rs3, rd,
					immed, prevPC};

				unique case (1'b1)
					// IRQ/EBREAK/ILLEGAL don't step the PC (since we need the PC intact during those operations)
					irqReq[0],
					irqReq[1],
					isebreak,
					isillegalinstruction:					PC <= prevPC;
					// Rest of the instructions will step to the adjacent address depending on instruction length
					// NOTE: ECALL will need next PC so that MRET can act like a return to the following instruction
					default:								PC <= isjal ? (prevPC + immed) : (prevPC + 32'd4);
				endcase

				// Flush I$ if we have an IFENCE instruction and go to wait
				icacheflush <= isfence;

				// Go to appropriate wait mode or resume FETCH
				// Stop fetching if we need to halt, running IFENCE, or entering/exiting an ISR
				priority case (1'b1)
					irqReq[0],
					irqReq[1],
					isecall,
					isebreak,
					isillegalinstruction:	begin fetchmode <= ENTERISR;			fetchena <= 1'b0; end
					isfence:				begin fetchmode <= WAITIFENCE;			fetchena <= 1'b0; end
					ismret:					begin fetchmode <= EXITISR;				fetchena <= 1'b0; end
					iswfi:					begin fetchmode <= WFI;					fetchena <= 1'b0; end
					isjal:					begin fetchmode <= FETCH;				fetchena <= 1'b1; end
					isbranch,
					isdiscard, 
					isflush:				begin fetchmode <= WAITNEWBRANCHTARGET;	fetchena <= 1'b0; end
					default:				begin fetchmode <= FETCH;				fetchena <= 1'b1; end
				endcase
			end

			WAITNEWBRANCHTARGET: begin
				// HAZARD#4: Wait for branch target resolve
				// Resume fetch when branch address is resolved
				fetchena <= branchresolved;
				// New PC to resume fetch at
				PC <= branchtarget;
				fetchmode <= branchresolved ? FETCH : WAITNEWBRANCHTARGET;
			end

			WAITIFENCE: begin
				// Resume fetch when I$ signals ready for pending flush
				fetchena <= rready;
				fetchmode <= rready ? FETCH : WAITIFENCE;
			end

			ENTERISR: begin
				// Inject entry instruction sequence (see table at microcode ROM section)
				priority case (1'b1)
					irqReq[0]: begin			// Interrupt: Timer
						injectAddr <= enterStartStop[0];
						injectStop <= enterStartStop[1];
						exceptioncode <= 5'h17;
					end
					irqReq[1]: begin			// Interrupt: Ext (UART and other hardware)
						injectAddr <= enterStartStop[2];
						injectStop <= enterStartStop[3];
						exceptioncode <= 5'h1B;
					end
					isecall: begin				// Exception: Environment call
						injectAddr <= enterStartStop[4];
						injectStop <= enterStartStop[5];
						exceptioncode <= 5'h0B;
					end
					isebreak: begin				// Exception: Debug breakpoint
						injectAddr <= enterStartStop[6];
						injectStop <= enterStartStop[7];
						exceptioncode <= 5'h03;
					end
					isillegalinstruction: begin	// Exception: Illegal instruction
						injectAddr <= enterStartStop[8];
						injectStop <= enterStartStop[9];
						exceptioncode <= 5'h02;
					end
					default: begin // Reserved
						injectAddr <= enterStartStop[10];
						injectStop <= enterStartStop[11];
						exceptioncode <= 5'h00;
					end
				endcase

				// Save states for exit time
				entryState <= {irqReq[0], irqReq[1], isecall, isebreak, isillegalinstruction};

				emitPC <= PC;

				fetchmode <= STARTINJECT;
				postInject <= POSTENTER;
			end

			EXITISR: begin
				// Inject exit instruction sequence (see table at microcode ROM section)
				priority case (1'b1)
					entryState[4]: begin		// Interrupt: Timer
						injectAddr <= exitStartStop[0];
						injectStop <= exitStartStop[1];
					end
					entryState[3]: begin		// Interrupt: Ext(UART)
						injectAddr <= exitStartStop[2];
						injectStop <= exitStartStop[3];
					end
					entryState[2]: begin		// Exception: Environment call
						injectAddr <= exitStartStop[4];
						injectStop <= exitStartStop[5];
					end
					entryState[1]: begin		// Exception: Debug breakpoint
						injectAddr <= exitStartStop[6];
						injectStop <= exitStartStop[7];
					end
					entryState[0]: begin		// Exception: Illegal instruction
						injectAddr <= exitStartStop[8];
						injectStop <= exitStartStop[9];
					end
					default: begin
						injectAddr <= exitStartStop[10];
						injectStop <= exitStartStop[11];
					end
				endcase

				emitPC <= PC;

				fetchmode <= STARTINJECT;
				postInject <= POSTEXIT;
			end

			STARTINJECT: begin
				// NOTE: This destroys decoded values from the actual instruction
				IR <= injectInstruction;
				fetchmode <= (injectAddr == injectStop) ? postInject : INJECT;
			end

			// WARNING: Injection ignores all instruction handling and never advances the PC
			// Care must be taken to not use any fetch-handled instructions in the 'fetch' ROM
			INJECT: begin
				ififowr_en <= ~ififofull;
				ififodin <= {
					csroffset,
					sysop, func3, func7,
					selectimmedasrval2,
					instrOneHotOut,
					bluop, aluop,
					rs1, rs2, rs3, rd,
					immed, emitPC};

				injectAddr <= injectAddr + 1;
				fetchmode <= ~ififofull ? STARTINJECT : INJECT;
			end

			POSTENTER: begin
				// TODO: Support for vectored jump using address encoding:
				// BASE[31:2] MODE[1:0] (modes-> 00:direct, 01:vectored, 10/11:reserved)
				// where exceptions jump to BASE, interrupts jump to BASE+exceptioncode*4
				// NOTE: OS has to hold a vectored interrupt table at the BASE address (set by mtvec)

				// Set up a jump to ISR (NOTE: MTVEC has to have been set in last execute pass) 
				if (mtvec[1:0] == 2'b00) begin	// Direct call
					PC <= {mtvec[31:2], 2'b00};
				end else begin					// Vectored call (assuming 01 used, treating 10 and 11 the same for now)
					PC <= {mtvec[31:2], 2'b00} + {exceptioncode, 2'b00}; // +exceptioncode*4
				end

				// Wait until control unit has executed everything in the FIFO
				// This ensures that the entry routine turns off global interrupts
				// so that the next time around we don't get irqReq, and also any
				// change to mtvec to be reflected properly
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTENTER;
			end

			POSTEXIT: begin
				// Restore PC to MEPC via CSR
				// NOTE: control unit possibly hasn't set mepc yet
				PC <= mepc;

				// Wait until control unit has executed everything in the FIFO
				// This ensures that the entry routine turns on global interrupts
				// so that the next time around we get irqReq, and also that a
				// change to mepc by a task manager is reflected properly
				fetchena <= ififoempty;
				fetchmode <= ififoempty ? FETCH : POSTEXIT;
			end

			WFI: begin
				// Wait for 16 clocks at most, as this is a hint rather than a blocking call
				wficount <= wficount + 4'd1;
				fetchena <= ((|irqReq) || wficount == 4'd0);
				fetchmode <= ((|irqReq) || wficount == 4'd0) ? FETCH : WFI;
			end
		endcase
	end
end

endmodule
