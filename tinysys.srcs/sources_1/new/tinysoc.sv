`timescale 1ns / 1ps

module tinysoc(
	input wire aclk,
	input wire clk10,
	input wire clk25,
	input wire clk50,
	input wire clk166,
	input wire clk200,
	input wire clk250,
	input wire aresetn,
	output wire [1:0] leds,
	output wire uart_rxd_out,
	gpuwires.def gpuvideoout,
	input wire uart_txd_in,
	ddr3sdramwires.def ddr3wires,
	sdwires.def sdconn);

// --------------------------------------------------
// Bus lines
// --------------------------------------------------

axi4if instructionbus();	// Fetch unit bus
axi4if databus();			// Data unit bus
axi4if gpubus();			// Graphics unit bus

axi4if memorybus();			// Arbitrated, to memory
axi4if bramif();			// Sub bus: BRAM
axi4if ddr3sdramif();		// Sub bus: DDR3 SDRAM

axi4if devicebus();			// Arbitrated, to devices
axi4if uartif();			// Sub bus: UART device
axi4if ledif();				// Sub bus: LED contol device
axi4if gpucmdif();			// Sub bus: GPU command fifo
axi4if spiif();				// Sub bus: SPI control device

ibusif ibus();				// Internal bus between units

// --------------------------------------------------
// Fetch unit
// --------------------------------------------------

wire branchresolved;
wire [31:0] branchtarget;
wire ififoempty;
wire ififovalid;
wire [107:0] ififodout;
wire ififord_en;

// Reset vector is at top of BRAM
fetchunit #(.RESETVECTOR(32'h10000000)) instructionfetch (
	.aclk(aclk),
	.aresetn(aresetn),
	.branchresolved(branchresolved),
	.branchtarget(branchtarget),
	.ififoempty(ififoempty),
	.ififovalid(ififovalid),
	.ififodout(ififodout),
	.ififord_en(ififord_en),
	.m_axi(instructionbus) );

// --------------------------------------------------
// Data unit
// --------------------------------------------------

dataunit dataunitints (
	.aclk(aclk),
	.aresetn(aresetn),
	.s_ibus(ibus),
	.databus(databus),
	.devicebus(devicebus) );

// --------------------------------------------------
// Control unit
// --------------------------------------------------

controlunit #(.CID(32'h00000000)) controlunitinst (
	.aclk(aclk),
	.aresetn(aresetn),
	.branchresolved(branchresolved),
	.branchtarget(branchtarget),
	.ififoempty(ififoempty),
	.ififovalid(ififovalid),
	.ififodout(ififodout),
	.ififord_en(ififord_en), 
	.m_ibus(ibus));

// --------------------------------------------------
// Graphics unit
// --------------------------------------------------

wire gpufifoempty;
wire [31:0] gpufifodout;
wire gpufifore;
wire gpufifovalid;
wire [31:0] vblankcount;
gpucore GPU(
	.aclk(aclk),
	.clk25(clk25),
	.clk250(clk250),
	.aresetn(aresetn),
	.m_axi(gpubus),
	.gpuvideoout(gpuvideoout),
	.gpufifoempty(gpufifoempty),
	.gpufifodout(gpufifodout),
	.gpufifore(gpufifore),
	.gpufifovalid(gpufifovalid),
	.vblankcount(vblankcount));

// --------------------------------------------------
// DMA unit
// --------------------------------------------------

/*dummymaster #(.MASTERID(32'h00000003)) fakeDMA (
	.aclk(aclk),
	.aresetn(aresetn),
	.m_axi(dmabus) );*/

// --------------------------------------------------
// Traffic arbiter between master units and memory
// --------------------------------------------------

arbiter arbiter3x1inst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s({gpubus, databus, instructionbus}),
	.axi_m(memorybus) );

// --------------------------------------------------
// Memory region router
// --------------------------------------------------

// dev   start     end       addrs[30:28]  size
// DDR3: 00000000  0FFFFFFF  3'b000        256MB
// BRAM: 10000000  1000FFFF  3'b001        64KB

memoryrouter memoryrouterinst(
	.aclk(aclk),
	.aresetn(aresetn),
    .axi_s(memorybus),
    .addressmask({3'b000, 3'b001}),
    .axi_m({ddr3sdramif, bramif}));

// --------------------------------------------------
// RAM
// --------------------------------------------------

bootmem bootmeminst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s(bramif) );

axi4ddr3sdram axi4ddr3sdraminst(
	.aclk(aclk),
	.aresetn(aresetn),
	.clk_sys_i(clk166),
	.clk_ref_i(clk200),
	.m_axi(ddr3sdramif),
	.ddr3wires(ddr3wires) );

// --------------------------------------------------
// Memory mapped device router
// --------------------------------------------------

// 12bit (4K) address space reserved for each memory mapped device
// dev   start     end       addrs[30:12]                 size
// UART: 80000000  80000FFF  19'b000_0000_0000_0000_0000  4KB
// LEDS: 80001000  80001FFF  19'b000_0000_0000_0000_0001  4KB
// GPUC: 80002000  80002FFF  19'b000_0000_0000_0000_0010  4KB
// SPIC: 80003000  80003FFF  19'b000_0000_0000_0000_0011  4KB
// ----: 80004000  80005FFF  19'b000_0000_0000_0000_0100  4KB
// ----: 80005000  80006FFF  19'b000_0000_0000_0000_0101  4KB
// ----: 80006000  80007FFF  19'b000_0000_0000_0000_0110  4KB
// ----: 80007000  80008FFF  19'b000_0000_0000_0000_0111  4KB

devicerouter devicerouterinst(
	.aclk(aclk),
	.aresetn(aresetn),
    .axi_s(devicebus),
    .addressmask({19'b000_0000_0000_0000_0011, 19'b000_0000_0000_0000_0010, 19'b000_0000_0000_0000_0001, 19'b000_0000_0000_0000_0000}),
    .axi_m({spiif, gpucmdif, ledif, uartif}));

// --------------------------------------------------
// Memory mapped devices
// --------------------------------------------------

// TODO: Use ~uartrcvempty as UART IRQ
wire uartrcvempty;

axi4uart uartdevice(
	.aclk(aclk),
	.clk10(clk10),
	.aresetn(aresetn),
	.s_axi(uartif),
	.uart_rxd_out(uart_rxd_out),
	.uart_txd_in(uart_txd_in),
	.uartrcvempty(uartrcvempty));

axi4led leddevice(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(ledif),
	.led(leds) );

// TODO: Could wire a vblankcount!=targetframe to trigger a vblank IRQ
gpucommanddevice gpucmdinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(gpucmdif),
	.fifoempty(gpufifoempty),
	.fifodout(gpufifodout),
	.fifore(gpufifore),
	.fifovalid(gpufifovalid),
	.vblankcount(vblankcount));

// TODO: Could wire SDCard insertion signal to trigger an IRQ
axi4spi spictlinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.spibaseclock(clk50),
	.sdconn(sdconn),
	.s_axi(spiif));
	
endmodule
