`timescale 1ns / 1ps

module tinysoc(
	input wire aclk,
	input wire clk10,
	input wire aresetn,
	output wire [1:0] leds,
	output wire uart_rxd_out,
	input wire uart_txd_in );

// --------------------------------------------------
// AXI4 bus
// --------------------------------------------------

axi4if instructionbus();	// Instruction bus
axi4if databus();			// Data bus
axi4if gpubus();			// GPU bus
//axi4if dmabus();			// DMA bus
//axi4if memmapbus();		// Memory mapped devices

axi4if memorybus();			// Arbitrated, to memory
axi4if devicebus();			// Arbitrated, to devices

// --------------------------------------------------
// Internal bus
// --------------------------------------------------

ibusif ibus();

// --------------------------------------------------
// Arbiter
// --------------------------------------------------

arbiter arbiter3x1inst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s({gpubus, databus, instructionbus}),
	.axi_m(memorybus) );

// --------------------------------------------------
// RAM
// --------------------------------------------------

bootmem bootmeminst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s(memorybus) );

// --------------------------------------------------
// Memory mapped device router
// --------------------------------------------------

axi4if uartif();
axi4if ledif();

// 12bit (4K) address space reserved for each memory mapped device
// dev   start     end       addrs[30:12]
// UART: 80000000  80000FFF  19'b000_0000_0000_0000_0000
// LEDS: 80001000  80001FFF  19'b000_0000_0000_0000_0001
// ????: 80002000  80002FFF  19'b000_0000_0000_0000_0010
// ????: 80003000  80003FFF  19'b000_0000_0000_0000_0011
// ????: 80004000  80005FFF  19'b000_0000_0000_0000_0100

devicerouter memmappeddevicerouter(
	.aclk(aclk),
	.aresetn(aresetn),
    .axi_s(devicebus),
    .addressmask({19'b000_0000_0000_0000_0001, 19'b000_0000_0000_0000_0000}),
    .axi_m({ledif, uartif}));

// --------------------------------------------------
// Memory mapped devices
// --------------------------------------------------

axi4uart uartdevice(
	.aclk(aclk),
	.clk10(clk10),
	.aresetn(aresetn),
	.s_axi(uartif),
	.uart_rxd_out(uart_rxd_out),
	.uart_txd_in(uart_txd_in),
	.uartrcvempty()); // TODO: ~uartrcvempty to IRQ line

axi4led leddevice(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(ledif),
	.led(leds) );

// --------------------------------------------------
// Fetch unit
// --------------------------------------------------

wire branchresolved;
wire [31:0] branchtarget;

// Fifo control
wire ififoempty;
wire ififovalid;
wire [107:0] ififodout;
wire ififord_en;

fetchunit #(.RESETVECTOR(32'h00000000)) instructionfetch (
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

dummymaster #(.MASTERID(32'h00000002)) fakeGPU (
	.aclk(aclk),
	.aresetn(aresetn),
	.m_axi(gpubus) );

// --------------------------------------------------
// DMA unit
// --------------------------------------------------

/*dummymaster #(.MASTERID(32'h00000003)) fakeDMA (
	.aclk(aclk),
	.aresetn(aresetn),
	.m_axi(dmabus) );*/

endmodule
