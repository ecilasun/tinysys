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
	input wire [133:0] ififodout,
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
logic [31:0] PCincrement;
logic [31:0] immed;
logic [31:0] csrprevval;
logic [17:0] instrOneHotOut;
logic [1:0] sysop;
logic [11:0] csroffset;
logic [4:0] rs1;
logic [4:0] rs2;
logic [4:0] rs3;
logic [4:0] rd;
logic [3:0] aluop;
logic [2:0] bluop;
logic [2:0] func3;
logic [6:0] func7;
logic [2:0] rfunc3;
logic selectimmedasrval2;
logic stepsize;

logic btready;
logic [31:0] btarget;

assign branchresolved = btready;
assign branchtarget = btarget;

// Operands for exec
logic [31:0] A; // rval1
logic [31:0] B; // rval2
logic [31:0] C; // rval2 : immed
logic [31:0] D; // immed
logic [31:0] fA; // frval1
logic [31:0] fB; // frval2
logic [31:0] fC; // frval3

// Writeback data
logic [31:0] wbdin;
logic [4:0] wbdest;
logic wback;
logic [31:0] fwbdin;
logic fwback;

// --------------------------------------------------
// Integer register file
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
// Float register file
// --------------------------------------------------

wire [31:0] frval1;
wire [31:0] frval2;
wire [31:0] frval3;
logic [31:0] frdin;
logic frwen = 1'b0;

floatregisterfile FGPR(
	.clock(aclk),
	.rs1(rs1),
	.rs2(rs2),
	.rs3(rs3),
	.rd(wbdest),
	.wren(frwen),
	.datain(frdin),
	.rval1(frval1),
	.rval2(frval2),
	.rval3(frval3) );

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

// ------------------------------------------------------------------------------------
// Floating point math
// ------------------------------------------------------------------------------------

logic fmaddstrobe = 1'b0;
logic fmsubstrobe = 1'b0;
logic fnmsubstrobe = 1'b0;
logic fnmaddstrobe = 1'b0;
logic faddstrobe = 1'b0;
logic fsubstrobe = 1'b0;
logic fmulstrobe = 1'b0;
logic fdivstrobe = 1'b0;
logic fi2fstrobe = 1'b0;
logic fui2fstrobe = 1'b0;
logic ff2istrobe = 1'b0;
logic ff2uistrobe = 1'b0;
logic ff2ui4satstrobe = 1'b0;
logic fsqrtstrobe = 1'b0;
logic feqstrobe = 1'b0;
logic fltstrobe = 1'b0;
logic flestrobe = 1'b0;

wire fpuresultvalid;
wire [31:0] fpuresult;

floatingpointunit FPU(
	.clock(aclk),
	.aresetn(aresetn),

	// inputs
	.frval1(fA),
	.frval2(fB),
	.frval3(fC),
	.rval1(A), // i2f input

	// operation select strobe
	.fmaddstrobe(fmaddstrobe),
	.fmsubstrobe(fmsubstrobe),
	.fnmsubstrobe(fnmsubstrobe),
	.fnmaddstrobe(fnmaddstrobe),
	.faddstrobe(faddstrobe),
	.fsubstrobe(fsubstrobe),
	.fmulstrobe(fmulstrobe),
	.fdivstrobe(fdivstrobe),
	.fi2fstrobe(fi2fstrobe),
	.fui2fstrobe(fui2fstrobe),
	.ff2istrobe(ff2istrobe),
	.ff2uistrobe(ff2uistrobe),
	.ff2ui4satstrobe(ff2ui4satstrobe),
	.fsqrtstrobe(fsqrtstrobe),
	.feqstrobe(feqstrobe),
	.fltstrobe(fltstrobe),
	.flestrobe(flestrobe),

	// output
	.resultvalid(fpuresultvalid),
	.result(fpuresult) );

// --------------------------------------------------
// Core logic
// --------------------------------------------------

typedef enum logic [4:0] {
	INIT,
	READINSTR, READREG,
	STORE, LOAD, DISPATCH,
	FPUOP, FUSEDMATHSTALL, FLOATMATHSTALL,
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
logic pendingmul = 1'b0;
logic pendingdiv = 1'b0;
logic pendingload = 1'b0;
logic pendingflwd = 1'b0;
logic pendingwrite = 1'b0;
wire pendingwback = rwen || frwen;

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
				frdin = m_ibus.rdata;
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
		frdin = fwbdin;
	end
	rwen = (pendingload && m_ibus.rdone) || mulready || divready || divuready || wback;
	frwen = (pendingflwd && m_ibus.rdone) || fwback;
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
		pendingflwd <= 1'b0;
		pendingwrite <= 1'b0;
		ctlmode <= INIT;
	end else begin
		btready <= 1'b0;	// Stop branch target ready strobe
		ififore <= 1'b0;	// Stop instruction fifo read enable strobe
		wback <= 1'b0;		// Stop register writeback shadow strobe
		fwback <= 1'b0;

		m_ibus.rstrobe <= 1'b0;	// Stop data read strobe
		m_ibus.wstrobe <= 4'h0;	// Stop data write strobe
		m_ibus.cstrobe <= 1'b0;	// Stop data cache strobe

		mulstrobe <= 1'b0;	// Stop integer mul strobe 
		divstrobe <= 1'b0;	// Stop integer div/rem strobe

		fmaddstrobe <= 1'b0;	// Stop floating point strobe
		fmsubstrobe <= 1'b0;
		fnmsubstrobe <= 1'b0;
		fnmaddstrobe <= 1'b0;
		faddstrobe <= 1'b0;
		fsubstrobe <= 1'b0;
		fmulstrobe <= 1'b0;
		fdivstrobe <= 1'b0;
		fi2fstrobe <= 1'b0;
		fui2fstrobe <= 1'b0;
		ff2istrobe <= 1'b0;
		ff2uistrobe <= 1'b0;
		ff2ui4satstrobe <= 1'b0;
		fsqrtstrobe <= 1'b0;
		feqstrobe <= 1'b0;
		fltstrobe <= 1'b0;
		flestrobe <= 1'b0;

		retiredstrobe <= 1'b0;

		if (m_ibus.wdone) pendingwrite <= 1'b0;
		if (m_ibus.rdone) pendingload <= 1'b0;
		if (m_ibus.rdone) pendingflwd <= 1'b0;
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
					sysop, func3, func7,
					selectimmedasrval2,
					instrOneHotOut,
					bluop, aluop,
					rs1, rs2, rs3, rd,
					immed, PC[31:1], stepsize} <= ififodout;

				unique case (ififodout[0])
					1'b0: PCincrement <= 32'd2;
					1'b1: PCincrement <= 32'd4;
				endcase

				// NOTE: Since we don't do 8 bit addressing (only 32 or 16), lowest bit is always set to zero
				PC[0] <= 1'b0;

				// HAZARD#0: Wait for fetch fifo to populate
				ififore <= (ififovalid && ~ififoempty);
				retiredstrobe <= (ififovalid && ~ififoempty);
				ctlmode <= (ififovalid && ~ififoempty) ? READREG : READINSTR;
			end

			READREG: begin
				if (pendingwback || pendingmul || pendingdiv || ((pendingload||pendingflwd) && ~m_ibus.rdone)) begin
					// HAZARD#1: Wait for pending register writeback
					// HAZARD#2: Wait for pending memory load
					ctlmode <= READREG;
				end else begin
					// Set up inputs to math/branch units, addresses, and any math strobes required
					A <= rval1;
					B <= rval2;
					C <= selectimmedasrval2 ? immed : rval2;
					fA <= frval1;
					fB <= frval2;
					fC <= frval3;
					D <= immed;
					wbdest <= rd;
					rwaddress <= rval1 + immed;
					offsetPC <= PC + immed;
					adjacentPC <= PC + PCincrement;

					unique case (1'b1)
						instrOneHotOut[`O_H_FLOAT_MADD],
						instrOneHotOut[`O_H_FLOAT_MSUB],
						instrOneHotOut[`O_H_FLOAT_NMSUB],
						instrOneHotOut[`O_H_FLOAT_NMADD]: begin
							fmaddstrobe <= instrOneHotOut[`O_H_FLOAT_MADD];
							fmsubstrobe <= instrOneHotOut[`O_H_FLOAT_MSUB];
							fnmsubstrobe <= instrOneHotOut[`O_H_FLOAT_NMSUB];
							fnmaddstrobe <= instrOneHotOut[`O_H_FLOAT_NMADD];
							ctlmode <= FUSEDMATHSTALL;
						end
						instrOneHotOut[`O_H_FLOAT_OP]: begin
							ctlmode <= FPUOP;
						end
						instrOneHotOut[`O_H_STORE],
						instrOneHotOut[`O_H_FLOAT_STW]: begin
							ctlmode <= STORE;
						end
						instrOneHotOut[`O_H_LOAD],
						instrOneHotOut[`O_H_FLOAT_LDW]: begin
							ctlmode <= LOAD;
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

			FPUOP: begin
				case (func7)
					`F7_FSGNJ: begin
						fwback <= 1'b1;
						case(func3)
							3'b000: fwbdin <= {fB[31], fA[30:0]}; // fsgnj
							3'b001: fwbdin <= {~fB[31], fA[30:0]}; // fsgnjn
							3'b010: fwbdin <= {fA[31]^fB[31], fA[30:0]}; // fsgnjx
						endcase
						ctlmode <= READINSTR;
					end
					`F7_FMVXW: begin
						wback <= 1'b1;
						if (func3 == 3'b000)
							wbdin <= fA; // fmvxw
						else
							wbdin <= 32'd0; // fclass todo: classify the float
						ctlmode <= READINSTR;
					end
					`F7_FMVWX: begin
						fwback <= 1'b1;
						fwbdin <= rval1;
						ctlmode <= READINSTR;
					end
					default: begin
						faddstrobe <= (func7 == `F7_FADD);
						fsubstrobe <= (func7 == `F7_FSUB);
						fmulstrobe <= (func7 == `F7_FMUL);
						fdivstrobe <= (func7 == `F7_FDIV);
						fi2fstrobe <= (func7 == `F7_FCVTSW) & (rs2==5'b00000); // Signed
						fui2fstrobe <= (func7 == `F7_FCVTSW) & (rs2==5'b00001); // Unsigned
						ff2istrobe <= (func7 == `F7_FCVTWS) & (rs2==5'b00000); // Signed
						ff2uistrobe <= (func7 == `F7_FCVTWS) & (rs2==5'b00001); // Unsigned
						ff2ui4satstrobe <= (func7 == `F7_FCVTSWU5SAT);
						fsqrtstrobe <= (func7 == `F7_FSQRT);
						feqstrobe <= (func7==`F7_FEQ) & (func3==`F3_FEQ);
						fltstrobe <= ((func7==`F7_FEQ) & (func3==`F3_FLT)) | (func7==`F7_FMAX); // min/max same as flt
						flestrobe <= (func7==`F7_FEQ) & (func3==`F3_FLE);
						ctlmode <= FLOATMATHSTALL;
					end
				endcase
			end

			FUSEDMATHSTALL: begin
				if (fpuresultvalid) begin
					fwbdin <= fpuresult;
					fwback <= 1'b1;
					ctlmode <= READINSTR;
				end else begin
					ctlmode <= FUSEDMATHSTALL;
				end
			end

			FLOATMATHSTALL: begin
				if (fpuresultvalid) begin
					case (func7)
						`F7_FCVTWS: begin
							wbdin <= fpuresult;
							wback <= 1'b1;
						end
						`F7_FEQ: begin
							wbdin <= {31'd0, fpuresult[0]};
							wback <= 1'b1;
						end
						`F7_FMAX: begin
							fwback <= 1'b1;
							if (func3==3'b000) // fmin
								fwbdin <= fpuresult[0] ? fA : fB;
							else // fmax
								fwbdin <= fpuresult[0] ? fB : fA;
						end
						default: begin // add/sub/mul/div/sqrt/cvtsw
							fwback <= 1'b1;
							fwbdin <= fpuresult;
						end
					endcase
					ctlmode <= READINSTR;
				end else begin
					ctlmode <= FLOATMATHSTALL;
				end
			end

			STORE: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.waddr <= rwaddress;
					pendingwrite <= 1'b1;
					unique case(func3)
						3'b000:  m_ibus.wdata <= {B[7:0], B[7:0], B[7:0], B[7:0]};
						3'b001:  m_ibus.wdata <= {B[15:0], B[15:0]};
						default: m_ibus.wdata <= instrOneHotOut[`O_H_STORE] ? B : fB;
					endcase
					unique case(func3)
						3'b000:  m_ibus.wstrobe <= {rwaddress[1], rwaddress[1], ~rwaddress[1], ~rwaddress[1]} & {rwaddress[0], ~rwaddress[0], rwaddress[0], ~rwaddress[0]};
						3'b001:  m_ibus.wstrobe <= {rwaddress[1], rwaddress[1], ~rwaddress[1], ~rwaddress[1]};
						default: m_ibus.wstrobe <= 4'b1111;
					endcase
					ctlmode <= READINSTR;
				end else begin
					// HAZARD#3: Wait for pending write before read
					ctlmode <= STORE;
				end
			end
	
			LOAD: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.raddr <= rwaddress;
					m_ibus.rstrobe <= 1'b1;
					rfunc3 <= func3;
					pendingload <= instrOneHotOut[`O_H_LOAD];
					pendingflwd <= instrOneHotOut[`O_H_FLOAT_LDW];
					ctlmode <= READINSTR;
				end else begin
					// HAZARD#3: Wait for pending write before write
					ctlmode <= LOAD;
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
