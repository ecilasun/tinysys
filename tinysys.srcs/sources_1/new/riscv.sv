module riscv #(
	parameter int RESETVECTOR = 32'd0
) (
	input wire aclk,
	input wire aresetn,
	input wire romReady,
	input wire sie,
	input wire [1:0] irqReq,
	input wire [31:0] mepc,
	input wire [31:0] mtvec,
	output wire [63:0] cpuclocktime,
	output wire [63:0] retired,
	axi4if.master instructionbus,
	axi4if.master databus,
	axi4if.master devicebus );

// --------------------------------------------------
// Internal buses
// --------------------------------------------------

ibusif internaldatabus();

// --------------------------------------------------
// Fetch unit
// --------------------------------------------------

wire branchresolved;
wire [31:0] branchtarget;
wire ififoempty;
wire ififovalid;
wire [119:0] ififodout;
wire ififord_en;

// Reset vector at last 64K of DDR3 SDRAM
fetchunit #(.RESETVECTOR(RESETVECTOR)) fetchdecodeinst (
	.aclk(aclk),
	.aresetn(aresetn),
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
	.romReady(romReady),
	.m_axi(instructionbus) );

// --------------------------------------------------
// Data unit
// --------------------------------------------------

dataunit dataunitinst (
	.aclk(aclk),
	.aresetn(aresetn),
	.s_ibus(internaldatabus), 	// CPU access for r/w
	.databus(databus),			// Access to memory
	.devicebus(devicebus) );	// Access to devices

// --------------------------------------------------
// Control unit
// --------------------------------------------------

controlunit #(
	.CID(32'h00000000),
	.CSRBASE(20'h8000A)) controlunitinst (
	.aclk(aclk),
	.aresetn(aresetn),
	.branchresolved(branchresolved),
	.branchtarget(branchtarget),
	.ififoempty(ififoempty),
	.ififovalid(ififovalid),
	.ififodout(ififodout),
	.ififord_en(ififord_en), 
	.cpuclocktime(cpuclocktime),
	.retired(retired),
	.m_ibus(internaldatabus));

endmodule