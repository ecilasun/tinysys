`timescale 1ns / 1ps

module tinysoc(
	input wire aclk,
	input wire clk10,
	input wire clk25,
	input wire clk50,
	input wire clk125,
	input wire clk166,
	input wire clk200,
	input wire aresetn,
	input wire preresetn,
	output wire [1:0] leds,
	output wire uart_rxd_out,
	input wire uart_txd_in,
	output wire usb_rxd_out,
	input wire usb_txd_in,
	gpuwires.def gpuvideoout,
	ddr3sdramwires.def ddr3wires,
	sdwires.def sdconn);

// --------------------------------------------------
// Bus lines
// --------------------------------------------------

axi4if instructionbus();	// Fetch unit bus
axi4if databus();			// Data unit bus
axi4if gpubus();			// Graphics unit bus
axi4if dmabus();			// Direct memory access bus

axi4if memorybus();			// Main memory

axi4if devicebus();			// Arbitrated, to devices
axi4if uartif();			// Sub bus: UART device
axi4if ledif();				// Sub bus: LED contol device
axi4if gpucmdif();			// Sub bus: GPU command fifo
axi4if spiif();				// Sub bus: SPI control device
axi4if csrif();				// Sub bus: CSR file device
axi4if xadcif();			// Sub bus: ADC controller
axi4if dmaif();				// Sub bus: DMA controller
axi4if usbif();				// Sub bus: USB serial i/o

ibusif ibus();				// Internal bus between units

// --------------------------------------------------
// Fetch unit
// --------------------------------------------------

wire branchresolved;
wire [31:0] branchtarget;
wire ififoempty;
wire ififovalid;
wire [143:0] ififodout;
wire ififord_en;
wire [1:0] irqReq;
wire [31:0] mepc;
wire [31:0] mtvec;
wire sie;
wire romReady;

// Reset vector at last 64K of DDR3 SDRAM
fetchunit #(.RESETVECTOR(32'h0FFE0000)) fetchdecodeinst (
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
	.s_ibus(ibus),
	.databus(databus),
	.devicebus(devicebus) );

// --------------------------------------------------
// Wall clock
// --------------------------------------------------

logic [63:0] wallclockcounter = 64'd0;

always @(posedge clk10) begin
	if (~aresetn) begin
		wallclockcounter <= 64'd0;
	end else begin
		wallclockcounter <= wallclockcounter + 64'd1;
	end
end

// CDC for wall clock
(* async_reg = "true" *) logic [63:0] wallclocktimeA = 64'd0;
(* async_reg = "true" *) logic [63:0] wallclocktime = 64'd0;

always @(posedge aclk) begin
	if (~aresetn) begin
		wallclocktimeA <= 64'd0;
		wallclocktime <= 64'd0;
	end else begin
		wallclocktimeA <= wallclockcounter;
		wallclocktime <= wallclocktimeA;
	end
end

// --------------------------------------------------
// Control unit
// --------------------------------------------------

wire [63:0] cpuclocktime;
wire [63:0] retired;

controlunit #(.CID(32'h00000000)) controlunitinst (
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
	.clk125(clk125),
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

wire dmafifoempty;
wire dmafifore;
wire dmafifovalid;
wire dmabusy;
wire [31:0] dmafifodout;

axi4dma DMA(
	.aclk(aclk),
	.aresetn(aresetn),
	.m_axi(dmabus),
	.dmafifoempty(dmafifoempty),
	.dmafifodout(dmafifodout),
	.dmafifore(dmafifore),
	.dmafifovalid(dmafifovalid),
	.romReady(romReady),
	.dmabusy(dmabusy) );

// --------------------------------------------------
// Traffic arbiter between master units and memory
// --------------------------------------------------

arbiter arbiter4x1inst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s({dmabus, gpubus, databus, instructionbus}),
	.axi_m(memorybus) );

// --------------------------------------------------
// RAM
// --------------------------------------------------

wire [11:0] device_temp;

// dev   start     end
// DDR3: 00000000  0FFFFFFF

axi4ddr3sdram axi4ddr3sdraminst(
	.aclk(aclk),
	.aresetn(aresetn),
	.preresetn(preresetn),
	.clk_sys_i(clk166),
	.clk_ref_i(clk200),
	.m_axi(memorybus),
	.ddr3wires(ddr3wires),
	.device_temp(device_temp) );

// --------------------------------------------------
// Memory mapped device router
// --------------------------------------------------

// NOTE: CSR offsets are word offsets therefore the
// address space is for 4Kbytes of data yet we can
// access 16KBytes worth of data.

// 12bit (4K) address space reserved for each memory mapped device
// dev   start     end       addrs[30:12]                 size
// UART: 80000000  80000FFF  19'b000_0000_0000_0000_0000  4KB
// LEDS: 80001000  80001FFF  19'b000_0000_0000_0000_0001  4KB
// GPUC: 80002000  80002FFF  19'b000_0000_0000_0000_0010  4KB
// SPIC: 80003000  80003FFF  19'b000_0000_0000_0000_0011  4KB
// CSRF: 80004000  80004FFF  19'b000_0000_0000_0000_0100  4KB
// XADC: 80005000  80005FFF  19'b000_0000_0000_0000_0101  4KB
// DMAC: 80006000  80006FFF  19'b000_0000_0000_0000_0110  4KB
// USBH: 80007000  80007FFF  19'b000_0000_0000_0000_0111  4KB
// ----: 80008000  80008FFF  19'b000_0000_0000_0000_1000  4KB

devicerouter devicerouterinst(
	.aclk(aclk),
	.aresetn(aresetn),
    .axi_s(devicebus),
    .addressmask({
        19'b000_0000_0000_0000_0111,	// USBH
    	19'b000_0000_0000_0000_0110,	// DMAC
    	19'b000_0000_0000_0000_0101,	// XADC
		19'b000_0000_0000_0000_0100,	// CSRF
		19'b000_0000_0000_0000_0011,	// SPIC
		19'b000_0000_0000_0000_0010,	// GPUC
		19'b000_0000_0000_0000_0001,	// LEDS
		19'b000_0000_0000_0000_0000 }),	// UART
    .axi_m({usbif, dmaif, xadcif, csrif, spiif, gpucmdif, ledif, uartif}));

// --------------------------------------------------
// Memory mapped devices
// --------------------------------------------------

wire uartrcvempty;
wire usbrcvempty;

axi4uart #(.BAUDRATE(115200), .FREQ(10000000)) uartinst(
	.aclk(aclk),
	.uartclk(clk10),
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

// TODO: Wire a vblankcount!=targetframe to trigger a vblank IRQ
gpucommanddevice gpucmdinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(gpucmdif),
	.fifoempty(gpufifoempty),
	.fifodout(gpufifodout),
	.fifore(gpufifore),
	.fifovalid(gpufifovalid),
	.vblankcount(vblankcount));

// TODO: Use sdconn switch state to trigger a card detect IRQ
axi4spi spictlinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.spibaseclock(clk50),
	.sdconn(sdconn),
	.s_axi(spiif));

// CSR file acts as a region of uncached memory
// Access to register indices get mapped to LOAD/STORE
// and addresses get mapped starting at 0x80004000 + csroffset
// CSR module also acts as the interrupt generator
axi4CSRFile csrfileinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.cpuclocktime(cpuclocktime),
	.wallclocktime(wallclocktime),
	.retired(retired),
	// IRQ tracking
	.irqReq(irqReq),
	// External interrupt wires
	.uartrcvempty(uartrcvempty),
	//.usbrcvempty(usbrcvempty),	// TODO: interrupt from USB serial
	// Shadow registers
	.mepc(mepc),
	.mtvec(mtvec),
	.sie(sie),
	// Bus
	.s_axi(csrif) );

// XADC
axi4xadc xadcinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(xadcif),
	.device_temp(device_temp) );

// DMA
dmacommanddevice dmacmdinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(dmaif),
	// Internal comms channel for DMA
	.fifoempty(dmafifoempty),
	.fifodout(dmafifodout),
	.fifore(dmafifore),
	.fifovalid(dmafifovalid),
    .dmabusy(dmabusy));

axi4uart #(.BAUDRATE(400000), .FREQ(10000000)) usbhostdevice(
	.aclk(aclk),
	.uartclk(clk10),
	.aresetn(aresetn),
	.s_axi(usbif),
	.uart_rxd_out(usb_rxd_out),
	.uart_txd_in(usb_txd_in),
	.uartrcvempty(usbrcvempty));

endmodule
