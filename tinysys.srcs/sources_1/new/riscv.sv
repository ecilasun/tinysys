module riscv #(
	parameter int CSRBASE = 16'h800A,
	parameter int RESETVECTOR = 32'd0
) (
	input wire aclk,
	input wire aresetn,
	input wire romReady,
	input wire sie,
	input wire cpuresetreq,
	input wire [1:0] irqReq,
	input wire [31:0] mepc,
	input wire [31:0] mtvec,
	output wire [31:0] pc_out,
	output wire [63:0] cpuclocktime,
	output wire [63:0] retired,
	axi4if.master instructionbus,
	axi4if.master databus,
	axi4if.master devicebus );

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// Fetch unit
// --------------------------------------------------

wire branchresolved;
wire [31:0] branchtarget;
wire ififoempty;
wire ififovalid;
wire [131:0] ififodout;
wire ififord_en;

// Reset vector at last 64K of DDR3 SDRAM
fetchunit #(.RESETVECTOR(RESETVECTOR)) fetchdecodeinst (
	.aclk(aclk),
	.aresetn(delayedresetn),
	.branchresolved(branchresolved),
	.branchtarget(branchtarget),
	.ififoempty(ififoempty),
	.ififovalid(ififovalid),
	.ififodout(ififodout),
	.ififord_en(ififord_en),
	.irqReq(irqReq),
	.mepc(mepc),
	.mtvec(mtvec),
	.sie(sie),
	.cpuresetreq(cpuresetreq),
	.romReady(romReady),
	.m_axi(instructionbus) );

// --------------------------------------------------
// Data unit
// --------------------------------------------------

ibusif internaldatabus();

dataunit dataunitinst (
	.aclk(aclk),
	.aresetn(delayedresetn),
	.s_ibus(internaldatabus), 	// CPU access for r/w
	.databus(databus),			// Access to memory
	.devicebus(devicebus) );	// Access to devices

// --------------------------------------------------
// Control unit
// --------------------------------------------------

controlunit #(
	.CSRBASE(CSRBASE)) controlunitinst (
	.aclk(aclk),
	.aresetn(delayedresetn),
	.branchresolved(branchresolved),
	.branchtarget(branchtarget),
	.ififoempty(ififoempty),
	.ififovalid(ififovalid),
	.ififodout(ififodout),
	.ififord_en(ififord_en), 
	.cpuclocktime(cpuclocktime),
	.retired(retired),
	.pc_out(pc_out),
	.m_ibus(internaldatabus));

endmodule