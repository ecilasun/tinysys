`timescale 1ns / 1ps

`include "shared.vh"

module controlunit #(
	parameter int CSRBASE = 16'h800A
) (
	input wire aclk,
	input wire aresetn,
	output wire branchresolved,
	output wire [31:0] branchtarget,
	// Instruction FIFO control
	input wire ififoempty,
	input wire ififovalid,
	input wire [131:0] ififodout,
	output wire ififord_en,
	// CPU cycle / retired instruction counts
	output wire [63:0] cpuclocktime,
	output wire [63:0] retired,
	// Internal bus to data unit
	ibusif.master m_ibus);

logic ififore;
assign ififord_en = ififore;

logic [63:0] cyclecount;
logic [63:0] retiredcount;
assign cpuclocktime = cyclecount;
assign retired = retiredcount;

logic [31:0] PC;
logic [31:0] immed;
logic [31:0] csrprevval;
logic [15:0] instrOneHotOut;
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

logic btready;
logic [31:0] btarget;

assign branchresolved = btready;
assign branchtarget = btarget;

// Operands for exec
logic [31:0] A;	// rval1
logic [31:0] B;	// rval2
logic [31:0] D;	// immed
logic [31:0] E;	// rval3
logic F;		// selectimmedasrval2

// Writeback data
logic [31:0] wbdin;
logic [4:0] wbdest;
logic wback;

// --------------------------------------------------
// Integer register file
// --------------------------------------------------

wire [31:0] rval1;
wire [31:0] rval2;
wire [31:0] rval3;
logic [31:0] rdin;
logic rwen;
integerregisterfile GPR(
	.clock(aclk),
	.rs1(rs1),
	.rs2(rs2),
	.rs3(rs3),
	.rd(wbdest),
	.wren(rwen),
	.din(rdin),
	.rval1(rval1),
	.rval2(rval2),
	.rval3(rval3));

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
	.aclk(aclk),
	.aresetn(aresetn),
	.aluout(aluout),
	.val1(A),
	.val2(F ? D : B),
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

logic fmaddstrobe;
logic fmsubstrobe;
logic fnmsubstrobe;
logic fnmaddstrobe;
logic faddstrobe;
logic fsubstrobe;
logic fmulstrobe;
logic fdivstrobe;
logic fi2fstrobe;
logic fui2fstrobe;
logic ff2istrobe;
logic ff2uistrobe;
logic ff2ui4satstrobe;
logic fsqrtstrobe;
logic feqstrobe;
logic fltstrobe;
logic flestrobe;

wire fpuresultvalid;
wire [31:0] fpuresult;

floatingpointunit FPU(
	.clock(aclk),
	.aresetn(aresetn),

	// inputs
	.frval1(A),
	.frval2(B),
	.frval3(E),

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
	CSROPS, SYSCDISCARD, SYSCFLUSH,
	WCSROP, WCACHE} controlunitmode;

controlunitmode ctlmode = INIT;
controlunitmode sysmode = INIT;

//logic cpurunning;
always @(posedge aclk) begin
	if (~aresetn) begin
		cyclecount <= 64'd0;
	end else begin
		// TODO: Stop this if CPU's halted for debug i.e. + {63'd0, cpurunning}
		cyclecount <= cyclecount + 64'd1;
	end
end

logic retiredstrobe;
always @(posedge aclk) begin
	if (~aresetn) begin
		retiredcount <= 64'd0;
	end else begin
		retiredcount <= retiredcount + {63'd0, retiredstrobe};
	end
end

// --------------------------------------------------
// Writeback
// --------------------------------------------------

logic [31:0] rwaddress;
logic pendingmul = 1'b0;
logic pendingdiv = 1'b0;
logic pendingload = 1'b0;
logic pendingwrite = 1'b0;
wire pendingwback = rwen;

always_comb begin
	if (~aresetn) begin
		rwen = 1'b0;
	end else begin
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
				default:	rdin = remainderu; // `F3_REMU
			endcase
		end else begin
			rdin = wbdin;
		end
		rwen = (pendingload && m_ibus.rdone) || mulready || divready || divuready || wback;
	end
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
		wbdin <= 32'd0;
		A <= 32'd0;
		B <= 32'd0;
		D <= 32'd0;
		E <= 32'd0;
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
		ctlmode <= INIT;
		sysmode <= INIT;
	end else begin
		btready <= 1'b0;	// Stop branch target ready strobe
		ififore <= 1'b0;	// Stop instruction fifo read enable strobe
		wback <= 1'b0;		// Stop register writeback shadow strobe

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
		if (mulready) pendingmul <= 1'b0;
		if (divready || divuready) pendingdiv <= 1'b0;

		unique case(ctlmode)
			INIT: begin
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
					immed, PC} <= ififodout;

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
					D <= immed;
					E <= rval3;
					F <= selectimmedasrval2;
					wbdest <= rd;
					rwaddress <= rval1 + immed;
					offsetPC <= PC + immed;
					adjacentPC <= PC + 32'd4;

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
						instrOneHotOut[`O_H_STORE]: begin
							ctlmode <= STORE;
						end
						instrOneHotOut[`O_H_LOAD]: begin
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
				unique case (func7)
					`F7_FSGNJ: begin
						wback <= 1'b1;
						case(func3)
							3'b000: wbdin <= {B[31], A[30:0]}; // fsgnj
							3'b001: wbdin <= {~B[31], A[30:0]}; // fsgnjn
							3'b010: wbdin <= {A[31]^B[31], A[30:0]}; // fsgnjx
						endcase
						ctlmode <= READINSTR;
					end
					`F7_FMVXW: begin
						wback <= 1'b1;
						wbdin <= A; // fmvxw (TODO: fclass: classify the float)
						// wbdin <= func3 == 3'b000 ? fA : ?????; // fmvxw
						ctlmode <= READINSTR;
					end
					`F7_FMVWX: begin
						wback <= 1'b1;
						wbdin <= rval1;
						ctlmode <= READINSTR;
					end
					`F7_FMAX: begin
						fltstrobe <= 1'b1; // min/max uses same hardware as flt
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FEQ: begin
						feqstrobe <= (func3==`F3_FEQ);
						fltstrobe <= (func3==`F3_FLT); // shares same hardware with min/max
						flestrobe <= (func3==`F3_FLE);
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FCVTSW: begin
						fi2fstrobe <= (rs2==5'b00000); // Signed
						fui2fstrobe <= (rs2!=5'b00000); // Unsigned (5'b00001)
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FCVTWS: begin
						ff2istrobe <= (rs2==5'b00000); // Signed
						ff2uistrobe <= (rs2!=5'b00000); // Unsigned (5'b00001)
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FADD: begin
						faddstrobe <= 1'b1;
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FSUB: begin
						fsubstrobe <= 1'b1;
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FMUL: begin
						fmulstrobe <= 1'b1;
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FDIV: begin
						fdivstrobe <= 1'b1;
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FCVTSWU4SAT: begin
						ff2ui4satstrobe <= 1'b1;
						ctlmode <= FLOATMATHSTALL;
					end
					`F7_FSQRT: begin
						fsqrtstrobe <= 1'b1;
						ctlmode <= FLOATMATHSTALL;
					end
					default: begin
						ctlmode <= FLOATMATHSTALL;
					end
				endcase
			end

			FUSEDMATHSTALL: begin
				if (fpuresultvalid) begin
					wbdin <= fpuresult;
					wback <= 1'b1;
					ctlmode <= READINSTR;
				end else begin
					ctlmode <= FUSEDMATHSTALL;
				end
			end

			FLOATMATHSTALL: begin
				if (fpuresultvalid) begin
					unique case (func7)
						`F7_FCVTWS,
						`F7_FCVTSWU4SAT: begin
							wbdin <= fpuresult;
							wback <= 1'b1;
						end
						`F7_FEQ: begin
							wbdin <= {31'd0, fpuresult[0]};
							wback <= 1'b1;
						end
						`F7_FMAX: begin
							if (func3==3'b000) // fmin
								wbdin <= fpuresult[0] ? A : B;
							else // fmax
								wbdin <= fpuresult[0] ? B : A;
							wback <= 1'b1;
						end
						default: begin // add/sub/mul/div/sqrt/cvtsw
							wbdin <= fpuresult;
							wback <= 1'b1;
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
					ctlmode <= STORE;
				end
			end
	
			LOAD: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.raddr <= rwaddress;
					m_ibus.rstrobe <= 1'b1;
					rfunc3 <= func3;
					pendingload <= instrOneHotOut[`O_H_LOAD];
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
					2'b10 : begin ctlmode <= SYSCDISCARD;	sysmode <= WCACHE; end
					2'b01 : begin ctlmode <= SYSCFLUSH;		sysmode <= WCACHE; end
					default : begin ctlmode <= CSROPS;		sysmode <= WCSROP; end
				endcase
			end
	
			SYSCDISCARD: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.dcacheop <= 2'b01; // {nowb,iscachecmd}
					m_ibus.cstrobe <= 1'b1;
					ctlmode <= sysmode;
				end else begin
					// HAZARD#3: Wait for pending read or write before cache discard
					ctlmode <= SYSCDISCARD;
				end
			end
	
			SYSCFLUSH: begin
				if (~pendingwrite || m_ibus.wdone) begin
					m_ibus.dcacheop <= 2'b11; // {wb,iscachecmd}
					m_ibus.cstrobe <= 1'b1;
					ctlmode <= sysmode;
				end else begin
					// HAZARD#3: Wait for pending read or write before cache flush
					ctlmode <= SYSCFLUSH;
				end
			end
	
			CSROPS: begin
				if (~pendingwrite || m_ibus.wdone) begin
					// 4 byte aligned
					m_ibus.raddr <= {CSRBASE, 2'b00, csroffset, 2'b00};
					m_ibus.rstrobe <= 1'b1;
					ctlmode <= sysmode;
				end else begin
					// HAZARD#3: Wait for pending write before CSR read
					ctlmode <= CSROPS;
				end
			end

			WCACHE: begin
				// Wait for pending cache operation to complete and unblock fetch unit
				btarget <= adjacentPC;
				btready <= m_ibus.cdone;
				ctlmode <= m_ibus.cdone ? READINSTR : WCACHE;
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
					// 4 byte aligned
					m_ibus.waddr <= {CSRBASE, 2'b00, csroffset, 2'b00};
					m_ibus.wstrobe <= 4'b1111;
					pendingwrite <= 1'b1;
					// Update CSR register with result of the operation
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
