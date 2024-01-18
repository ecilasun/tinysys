`timescale 1ns / 1ps

`include "shared.vh"

module controlunit #(
	parameter int CID = 32'h00000000,	// Corresponds to HARTID
	parameter int CSRBASE = 20'h8000A	// TODO: Add CID<<12, for instance HART#1 would be 32'h8000B
) (
	input wire aclk,
	input wire aresetn,
	output wire branchresolved,
	output wire [31:0] branchtarget,
	// Instruction FIFO control
	input wire ififoempty,
	input wire ififovalid,
	input wire [119:0] ififodout,
	output wire ififord_en,
	// CPU cycle / retired instruction counts
	output wire [63:0] cpuclocktime,
	output wire [63:0] retired,
	// Internal bus to data unit
	ibusif.master m_ibus);

logic ififore;
assign ififord_en = ififore;

(* extract_reset = "yes" *) logic [63:0] cyclecount;
(* extract_reset = "yes" *) logic [63:0] retiredcount;
assign cpuclocktime = cyclecount;
assign retired = retiredcount;

logic [31:0] PC;
logic [31:0] immed;
logic [31:0] csrprevval;
logic [17:0] instrOneHotOut;
logic [1:0] sysop;
logic [11:0] csroffset;
logic [4:0] rs1;
logic [4:0] rs2;
logic [4:0] rd;
logic [3:0] aluop;
logic [2:0] bluop;
logic [2:0] func3;
logic [2:0] rfunc3;
logic selectimmedasrval2;

logic btready;
logic [31:0] btarget;

assign branchresolved = btready;
assign branchtarget = btarget;

// Operands for exec
logic [31:0] A; // rval1
logic [31:0] B; // rval2
logic [31:0] C; // rval2 : immed
logic [31:0] D; // immed

// Writeback data
logic [31:0] wbdin;
logic [4:0] wbdest;
logic wback;

// --------------------------------------------------
// Register file
// --------------------------------------------------

wire [31:0] rval1;
wire [31:0] rval2;
logic [31:0] rdin;
logic rwen = 1'b0;
integerregisterfile GPR(
	.clock(aclk),
	.rs1(rs1),
	.rs2(rs2),
	.rd(wbdest),
	.wren(rwen),
	.din(rdin),
	.rval1(rval1),
	.rval2(rval2) );

// --------------------------------------------------
// Branch logic
// --------------------------------------------------

wire branchout;
branchlogic BLU(
	.aresetn(aresetn),
	.branchout(branchout),
	.val1(A),
	.val2(B),
	.bluop(bluop) );

// --------------------------------------------------
// Arithmetic
// --------------------------------------------------

wire [31:0] aluout;
arithmeticlogic ALU(
	.aresetn(aresetn),
	.aluout(aluout),
	.val1(A),
	.val2(C),
	.aluop(aluop) );

// --------------------------------------------------
// IMUL/IDIV
// --------------------------------------------------

logic mulstrobe = 1'b0;
logic divstrobe = 1'b0;

wire mulready;
wire [31:0] product;
logic [2:0] mfunc3;
integermultiplier IMULSU(
    .aclk(aclk),
    .aresetn(aresetn),
    .start(mulstrobe),
    .ready(mulready),
    .func3(mfunc3),
    .multiplicand(A),
    .multiplier(B),
    .product(product) );

wire divuready;
wire [31:0] quotientu, remainderu;
integerdividerunsigned IDIVU (
	.aclk(aclk),
	.aresetn(aresetn),
	.start(divstrobe),
	.ready(divuready),
	.dividend(A),
	.divisor(B),
	.quotient(quotientu),
	.remainder(remainderu) );

wire divready;
wire [31:0] quotient, remainder;
integerdividersigned IDIVS (
	.aclk(aclk),
	.aresetn(aresetn),
	.start(divstrobe),
	.ready(divready),
	.dividend(A),
	.divisor(B),
	.quotient(quotient),
	.remainder(remainder) );

// --------------------------------------------------
// Core logic
// --------------------------------------------------

typedef enum logic [3:0] {
	INIT,
	READINSTR, READREG,
	WRITE, READ, DISPATCH,
	SYSOP, SYSWBACK, SYSWAIT,
	CSROPS, WCSROP,
	SYSCDISCARD, SYSCFLUSH, WCACHE} controlunitmode;
controlunitmode ctlmode = INIT;

//logic cpurunning;
always @(posedge aclk) begin
	// TODO: Stop this if CPU's halted for debug i.e. + {63'd0, cpurunning}
	cyclecount <= cyclecount + 64'd1;

	if (~aresetn) begin
		cyclecount <= 64'd0;
	end
end

logic retiredstrobe;
always @(posedge aclk) begin
	retiredcount <= retiredcount + {63'd0, retiredstrobe};

	if (~aresetn) begin
		retiredcount <= 64'd0;
	end
end

// --------------------------------------------------
// Writeback
// --------------------------------------------------

logic [31:0] rwaddress;
logic pendingmul;
logic pendingdiv;
logic pendingload;
logic pendingwrite;
wire pendingwback = rwen;

always_comb begin
	if (m_ibus.rdone) begin
		unique case(rfunc3)
			3'b000: begin // BYTE with sign extension
				unique case(rwaddress[1:0])
					2'b11: begin rdin = {{24{m_ibus.rdata[31]}}, m_ibus.rdata[31:24]}; end
					2'b10: begin rdin = {{24{m_ibus.rdata[23]}}, m_ibus.rdata[23:16]}; end
					2'b01: begin rdin = {{24{m_ibus.rdata[15]}}, m_ibus.rdata[15:8]}; end
					2'b00: begin rdin = {{24{m_ibus.rdata[7]}}, m_ibus.rdata[7:0]}; end
				endcase
			end
			3'b001: begin // HALF with sign extension
				unique case(rwaddress[1])
					1'b1: begin rdin = {{16{m_ibus.rdata[31]}}, m_ibus.rdata[31:16]}; end
					1'b0: begin rdin = {{16{m_ibus.rdata[15]}}, m_ibus.rdata[15:0]}; end
				endcase
			end
			3'b100: begin // BYTE with zero extension
				unique case(rwaddress[1:0])
					2'b11: begin rdin = {24'd0, m_ibus.rdata[31:24]}; end
					2'b10: begin rdin = {24'd0, m_ibus.rdata[23:16]}; end
					2'b01: begin rdin = {24'd0, m_ibus.rdata[15:8]}; end
					2'b00: begin rdin = {24'd0, m_ibus.rdata[7:0]}; end 
				endcase
			end
			3'b101: begin // HALF with zero extension
				unique case(rwaddress[1])
					1'b1: begin rdin = {16'd0, m_ibus.rdata[31:16]}; end
					1'b0: begin rdin = {16'd0, m_ibus.rdata[15:0]}; end
				endcase
			end
			default: begin // WORD - 3'b010
				rdin = m_ibus.rdata;
			end
		endcase
	end else if (mulready) begin
		rdin = product;
	end else if (divready || divuready) begin
		unique case (mfunc3)
			`F3_DIV:	rdin = quotient;
			`F3_DIVU:	rdin = quotientu;
			`F3_REM:	rdin = remainder;
			`F3_REMU:	rdin = remainderu;
		endcase
	end else begin
		rdin = wbdin;
	end
	rwen = m_ibus.rdone || mulready || divready || divuready || wback;
end

// --------------------------------------------------
// Main
// --------------------------------------------------

logic [31:0] offsetPC;
logic [31:0] adjacentPC;

// EXEC
always @(posedge aclk) begin
	if (~aresetn) begin
		m_ibus.raddr <= 32'd0;
		m_ibus.waddr <= 32'd0;
		m_ibus.rstrobe <= 1'b0;
		m_ibus.wstrobe <= 4'h0;
		m_ibus.cstrobe <= 1'b0;
		m_ibus.dcacheop <= 2'b0;
		btready <= 1'b0;
		ififore <= 1'b0;
		pendingmul <= 1'b0;
		pendingdiv <= 1'b0;
		pendingload <= 1'b0;
		pendingwrite <= 1'b0;
		ctlmode <= INIT;
	end else begin
		btready <= 1'b0;	// Stop branch target ready strobe
		ififore <= 1'b0;	// Stop instruction fifo read enable strobe
		wback <= 1'b0;		// Stop register writeback shadow strobe 
	
		m_ibus.rstrobe <= 1'b0;	// Stop data read strobe
		m_ibus.wstrobe <= 4'h0;	// Stop data write strobe
		m_ibus.cstrobe <= 1'b0;	// Stop data cache strobe
	
		mulstrobe <= 1'b0;	// Stop integer mul strobe 
		divstrobe <= 1'b0;	// Stop integer div/rem strobe
		
		retiredstrobe <= 1'b0;
	
		if (m_ibus.wdone) pendingwrite <= 1'b0;
		if (m_ibus.rdone) pendingload <= 1'b0;
		if (mulready) pendingmul <= 1'b0;
		if (divready || divuready) pendingdiv <= 1'b0;
	
		unique case(ctlmode)
			INIT: begin
				wbdin <= 32'd0;
				ctlmode <= READINSTR;
			end
	
			READINSTR: begin
				// Grab next decoded instruction if there's something in the FIFO
				{	csroffset,
					sysop, func3,
					instrOneHotOut, selectimmedasrval2,
					bluop, aluop,
					rs1, rs2, rd,
					immed, PC[31:2]} <= ififodout;

				PC[1:0] <= 2'b0;

				// HAZARD#0: Wait for fetch fifo to populate
				ififore <= (ififovalid && ~ififoempty);
				retiredstrobe <= (ififovalid && ~ififoempty);
				ctlmode <= (ififovalid && ~ififoempty) ? READREG : READINSTR;
			end
	
			READREG: begin
				if (pendingwback || pendingmul || pendingdiv || (pendingload && ~m_ibus.rdone)) begin
					// HAZARD#1: Wait for pending register writeback
					// HAZARD#2: Wait for pending memory load
					ctlmode <= READREG;
				end else begin
					// Set up inputs to math/branch units, addresses, and any math strobes required
					A <= rval1;
					B <= rval2;
					C <= selectimmedasrval2 ? immed : rval2;
					D <= immed;
					wbdest <= rd;
					rwaddress <= rval1 + immed;
					offsetPC <= PC + immed;
					adjacentPC <= PC + 32'd4;
	
					unique case (1'b1)
						instrOneHotOut[`O_H_STORE]: begin
							ctlmode <= WRITE;
						end
						instrOneHotOut[`O_H_LOAD]: begin
							ctlmode <= READ;
						end
						instrOneHotOut[`O_H_SYSTEM]: begin
							ctlmode <= SYSOP;
						end
						(aluop==`ALU_MUL): begin
							mfunc3 <= func3;
							mulstrobe <= 1'b1;
							pendingmul <= 1'b1;
							ctlmode <= READINSTR;
						end
						(aluop==`ALU_DIV),
						(aluop==`ALU_REM): begin
							mfunc3 <= func3;
							divstrobe <= 1'b1;
							pendingdiv <= 1'b1;
							ctlmode <= READINSTR;
						end
						default: begin
							ctlmode <= DISPATCH;
						end
					endcase
					
				end
			end
	
			WRITE: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.waddr <= rwaddress;
					pendingwrite <= 1'b1;
					unique case(func3)
						3'b000:  m_ibus.wdata <= {B[7:0], B[7:0], B[7:0], B[7:0]};
						3'b001:  m_ibus.wdata <= {B[15:0], B[15:0]};
						default: m_ibus.wdata <= B;
					endcase
					unique case(func3)
						3'b000:  m_ibus.wstrobe <= {rwaddress[1], rwaddress[1], ~rwaddress[1], ~rwaddress[1]} & {rwaddress[0], ~rwaddress[0], rwaddress[0], ~rwaddress[0]};
						3'b001:  m_ibus.wstrobe <= {rwaddress[1], rwaddress[1], ~rwaddress[1], ~rwaddress[1]};
						default: m_ibus.wstrobe <= 4'b1111;
					endcase
					ctlmode <= READINSTR;
				end else begin
					// HAZARD#3: Wait for pending write before read
					ctlmode <= WRITE;
				end
			end
	
			READ: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.raddr <= rwaddress;
					m_ibus.rstrobe <= 1'b1;
					rfunc3 <= func3;
					pendingload <= 1'b1;
					ctlmode <= READINSTR;
				end else begin
					// HAZARD#3: Wait for pending write before write
					ctlmode <= READ;
				end
			end
	
			DISPATCH: begin
				// Most instructions are done here and go directly to writeback
				btarget <= 32'd0;
				unique case(1'b1)
					instrOneHotOut[`O_H_OP],
					instrOneHotOut[`O_H_OP_IMM]: begin
						// Store ALU output in target register
						wbdin <= aluout;
						wback <= 1'b1;
					end
					instrOneHotOut[`O_H_AUIPC]: begin
						// Store PC relative offset in target register
						wbdin <= offsetPC;
						wback <= 1'b1;
					end
					instrOneHotOut[`O_H_LUI]: begin
						// Store the immed in target register
						wbdin <= D;
						wback <= 1'b1;
					end
					instrOneHotOut[`O_H_JAL]: begin
						// Save return address
						wbdin <= adjacentPC;
						wback <= 1'b1;
						// NOTE: Fetch unit takes the branch before we get here
					end
					instrOneHotOut[`O_H_JALR]: begin
						// Save return address
						wbdin <= adjacentPC;
						wback <= 1'b1;
						// Notify fetch to resume
						btarget <= rwaddress;
						btready <= 1'b1;
					end
					instrOneHotOut[`O_H_BRANCH]: begin
						// Take or skip branch and notify fetch to resume
						btarget <= branchout ? offsetPC : adjacentPC;
						btready <= 1'b1;
					end
				endcase
				
				ctlmode <= READINSTR;
			end
	
			SYSOP: begin
				unique case (sysop)
					2'b00,
					2'b11 : ctlmode <= CSROPS;
					2'b10 : ctlmode <= SYSCDISCARD;
					2'b01 : ctlmode <= SYSCFLUSH;
				endcase
			end
	
			SYSCDISCARD: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.dcacheop <= 2'b01; // {nowb,iscachecmd}
					m_ibus.cstrobe <= 1'b1;
					ctlmode <= WCACHE;
				end else begin
					// HAZARD#3: Wait for pending read or write before cache discard
					ctlmode <= SYSCDISCARD;
				end
			end
	
			SYSCFLUSH: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.dcacheop <= 2'b11; // {wb,iscachecmd}
					m_ibus.cstrobe <= 1'b1;
					ctlmode <= WCACHE;
				end else begin
					// HAZARD#3: Wait for pending read or write before cache flush
					ctlmode <= SYSCFLUSH;
				end
			end
	
			WCACHE: begin
				// Wait for pending cache operation to complete and unblock fetch unit
				btarget <= adjacentPC;
				btready <= m_ibus.cdone;
				ctlmode <= m_ibus.cdone ? READINSTR : WCACHE;
			end
	
			CSROPS: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.raddr <= {CSRBASE, csroffset};
					m_ibus.rstrobe <= 1'b1;
					ctlmode <= WCSROP;
				end else begin
					// HAZARD#3: Wait for pending write before CSR read
					ctlmode <= CSROPS;
				end
			end
	
			WCSROP: begin
				if (m_ibus.rdone) begin
					csrprevval <= m_ibus.rdata;
					wbdin <= m_ibus.rdata;
					wback <= 1'b1;
				end
				ctlmode <= m_ibus.rdone ? SYSWBACK : WCSROP;
			end
	
			SYSWBACK: begin
				if (~pendingwback && (~pendingwrite || m_ibus.wdone)) begin
					// Update CSR register with read value
					m_ibus.waddr <= {CSRBASE, csroffset};
					m_ibus.wstrobe <= 4'b1111;
					pendingwrite <= 1'b1;
					unique case (func3)
						3'b001: begin // CSRRW
							m_ibus.wdata <= A;
						end
						3'b101: begin // CSRRWI
							m_ibus.wdata <= D;
						end
						3'b010: begin // CSRRS
							m_ibus.wdata <= csrprevval | A;
						end
						3'b110: begin // CSRRSI
							m_ibus.wdata <= csrprevval | D;
						end
						3'b011: begin // CSRRC
							m_ibus.wdata <= csrprevval & (~A);
						end
						3'b111: begin // CSRRCI
							m_ibus.wdata <= csrprevval & (~D);
						end
						default: begin // Unknown - keep previous value
							m_ibus.wdata <= csrprevval;
						end
					endcase
					// Wait for CSR writeback
					ctlmode <= SYSWAIT;
				end else begin
					// HAZARD#3: Wait for pending write before CSR write
					ctlmode <= SYSWBACK;
				end
			end
	
			SYSWAIT: begin
				ctlmode <= m_ibus.wdone ? READINSTR : SYSWAIT;
			end
		endcase
	end
end

endmodule
