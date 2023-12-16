`timescale 1ns / 1ps

module tinysoc #(
	parameter int RESETVECTOR = 32'd0
) (
	input wire aclk,
	input wire clk10,
	input wire clkaudio,
	input wire clk25,
	input wire clk50,
	input wire clk100,
	input wire clk166,
	input wire clk200,
	input wire aresetn,
	input wire sysresetn,
	input wire preresetn,
	output wire [3:0] leds,
	//debugbusif.slave s_dbg,
	output wire vvsync,
	output wire vhsync,
	output wire vclk,
	output wire vde,
	output wire [11:0] vdat,
	ddr3sdramwires.def ddr3conn,
	audiowires.def i2sconn,
	sdcardwires.def sdconn,
	max3420wires.def usbcconn,
	max3420wires.def usbaconn );

// --------------------------------------------------
// Bus lines
// --------------------------------------------------

axi4if instructionbusHart0();	// Fetch bus for HART#0
axi4if databusHart0();			// Data bus for HART#0
axi4if devicebusHart0();		// Direct access to devices from data unit of HART#0

axi4if videobus();				// Video output unit bus
axi4if dmabus();				// Direct memory access bus
axi4if romcopybus();			// Bus for boot ROM copy (TODO: switch between ROM types?)
axi4if audiobus();				// Bus for audio device output

axi4if memorybus();				// Arbitrated access to main memory

axi4if ledif();					// Sub bus: LED contol device
axi4if vpucmdif();				// Sub bus: VPU command fifo
axi4if spiif();					// Sub bus: SPI control device
axi4if csrif();					// Sub bus: CSR file device
axi4if xadcif();				// Sub bus: ADC controller
axi4if dmaif();					// Sub bus: DMA controller
axi4if usbcif();				// Sub bus: USB-C controller (device)
axi4if audioif();				// Sub bus: APU i2s audio output unit
axi4if opl2if();				// Sub bus: OPL2 interface
axi4if usbaif();				// Sub bus: USB-A controller (host)

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
	wallclocktimeA <= wallclockcounter;
	wallclocktime <= wallclocktimeA;
end

// --------------------------------------------------
// HART#0
// --------------------------------------------------

wire romReady, sieHart0;
wire [63:0] cpuclocktimeHart0;
wire [63:0] retiredHart0;
wire [31:0] mepcHart0;
wire [31:0] mtvecHart0;
wire [1:0] irqReqHart0;

riscv #(.RESETVECTOR(RESETVECTOR)) hart0(
	.aclk(aclk),
	.aresetn(aresetn),
	.romReady(romReady),
	.sie(sieHart0),
	.mepc(mepcHart0),
	.mtvec(mtvecHart0),
	.irqReq(irqReqHart0),
	.instructionbus(instructionbusHart0),
	.databus(databusHart0),
	.devicebus(devicebusHart0),
	.cpuclocktime(cpuclocktimeHart0),
	.retired(retiredHart0));

// --------------------------------------------------
// Graphics unit
// --------------------------------------------------

wire vpufifoempty;
wire [31:0] vpufifodout;
wire vpufifore;
wire vpufifovalid;
wire [31:0] vpustate;
videocore VPU(
	.aclk(aclk),
	.clk25(clk25),
	.aresetn(aresetn),
	.m_axi(videobus),
	.vvsync(vvsync),
	.vhsync(vhsync),
	.vclk(vclk),
	.vde(vde),
	.vdat(vdat),
	.vpufifoempty(vpufifoempty),
	.vpufifodout(vpufifodout),
	.vpufifore(vpufifore),
	.vpufifovalid(vpufifovalid),
	.vpustate(vpustate));

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
	.dmabusy(dmabusy) );

// --------------------------------------------------
// Boot ROM copy unit
// --------------------------------------------------

axi4romcopy #(.RESETVECTOR(RESETVECTOR)) COPYROM(
	.aclk(aclk),
	.aresetn(aresetn),
	.m_axi(romcopybus),
	.romReady(romReady));

// --------------------------------------------------
// Audio processing unit
// --------------------------------------------------

wire audiofifoempty;
wire [31:0] audiofifodout;
wire audiofifore;
wire audiofifovalid;
wire [31:0] audiobufferswapcount;
wire [15:0] opl2sampleout;

axi4i2saudio APU(
	.aclk(aclk),				// Bus clock
	.aresetn(aresetn),
    .audioclock(clkaudio),		// 22.591MHz master clock

	.m_axi(audiobus),			// Memory access

	.abempty(audiofifoempty),	// APU command FIFO
	.abvalid(audiofifovalid),
	.audiore(audiofifore),
    .audiodin(audiofifodout),
    .swapcount(audiobufferswapcount),
    
    .mixinput(opl2sampleout),

    .tx_mclk(i2sconn.mclk),
    .tx_lrck(i2sconn.lrclk),
    .tx_sclk(i2sconn.sclk),
    .tx_sdout(i2sconn.sdin) );

// --------------------------------------------------
// Traffic arbiter between master units and memory
// --------------------------------------------------

arbiter arbiter6x1instSDRAM(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s({romcopybus, audiobus, dmabus, videobus, databusHart0, instructionbusHart0}),
	.axi_m(memorybus) );

// --------------------------------------------------
// RAM
// --------------------------------------------------

wire [11:0] device_temp;

// dev   start     end      size
// DDR3: 00000000  0FFFFFFF (256 Mbytes, 16 byte r/w only)

axi4ddr3sdram axi4ddr3sdraminst(
	.aclk(aclk),
	.aresetn(aresetn),
	.preresetn(preresetn),
	.clk_sys_i(clk166),
	.clk_ref_i(clk200),
	.m_axi(memorybus),
	.ddr3conn(ddr3conn),
	.device_temp(device_temp) );

// --------------------------------------------------
// Memory mapped device router
// --------------------------------------------------

// NOTE: CSR offsets are word offsets therefore the
// address space is for 4Kbytes of data yet we can
// access 16KBytes worth of data.

// 12bit (4K) address space reserved for each memory mapped device
// dev   start     end       addrs[30:12]                 size  notes
// ----: 80000000  80000FFF  19'b000_0000_0000_0000_0000  4KB
// LEDS: 80001000  80001FFF  19'b000_0000_0000_0000_0001  4KB
// VPUC: 80002000  80002FFF  19'b000_0000_0000_0000_0010  4KB
// SDCC: 80003000  80003FFF  19'b000_0000_0000_0000_0011  4KB
// CSRF: 80004000  80004FFF  19'b000_0000_0000_0000_0100  4KB	HART#0
// XADC: 80005000  80005FFF  19'b000_0000_0000_0000_0101  4KB
// DMAC: 80006000  80006FFF  19'b000_0000_0000_0000_0110  4KB
// USBC: 80007000  80007FFF  19'b000_0000_0000_0000_0111  4KB
// APUC: 80008000  80008FFF  19'b000_0000_0000_0000_1000  4KB
// OPL2: 80009000  80009FFF  19'b000_0000_0000_0000_1001  4KB
// USBA: 8000A000  8000AFFF  19'b000_0000_0000_0000_1010  4KB
// ----: 8000B000  8000BFFF  19'b000_0000_0000_0000_1011  4KB
// ----: 8000C000  8000CFFF  19'b000_0000_0000_0000_1100  4KB
// ----: 8000D000  8000DFFF  19'b000_0000_0000_0000_1101  4KB
// ----: 8000E000  8000EFFF  19'b000_0000_0000_0000_1110  4KB
// ----: 8000F000  8000FFFF  19'b000_0000_0000_0000_1111  4KB

devicerouter devicerouterinst(
	.aclk(aclk),
	.aresetn(aresetn),
    .axi_s(devicebusHart0),				// TODO: Will need this to come from a bus arbiter
    .addressmask({
    	19'b000_0000_0000_0000_1010,	// USBA USB-A access via SPI
    	19'b000_0000_0000_0000_1001,	// OPL2	OPL2(YM8312) Command and Register Ports
    	19'b000_0000_0000_0000_1000,	// APUC	Audio Processing Unit Command Fifo
        19'b000_0000_0000_0000_0111,	// USBC USB-C access via SPI
    	19'b000_0000_0000_0000_0110,	// DMAC DMA Command Fifo
    	19'b000_0000_0000_0000_0101,	// XADC Analog / Digital Converter Interface
		19'b000_0000_0000_0000_0100,	// CSRF Control and Status Register File for HART#0
		19'b000_0000_0000_0000_0011,	// SDCC SDCard access via SPI
		19'b000_0000_0000_0000_0010,	// VPUC Graphics Processing Unit Command Fifo
		19'b000_0000_0000_0000_0001}),	// LEDS Debug / Status LED interface
    .axi_m({usbaif, opl2if, audioif, usbcif, dmaif, xadcif, csrif, spiif, vpucmdif, ledif}));

// --------------------------------------------------
// Interrupt wires
// --------------------------------------------------

wire keyfifoempty;
wire usbcirq, usbairq;

// --------------------------------------------------
// Memory mapped devices
// --------------------------------------------------

axi4led leddevice(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(ledif),
	.led(leds) );

commandqueue vpucmdinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(vpucmdif),
	.fifoempty(vpufifoempty),
	.fifodout(vpufifodout),
	.fifore(vpufifore),
	.fifovalid(vpufifovalid),
	.devicestate(vpustate));

axi4sdcard sdcardinst(
	.aclk(aclk),
	.clk10(clk10),
	.aresetn(aresetn),
	.spibaseclock(clk50),
	.sdconn(sdconn),
	.keyfifoempty(keyfifoempty),
	.s_axi(spiif));

axi4usbc usbserialport(
	.aclk(aclk),
	.aresetn(aresetn),
	.spibaseclock(clk50),
	.usbcconn(usbcconn),
	.usbirq(usbcirq),
	.s_axi(usbcif));

axi4usbc usbhostport(
	.aclk(aclk),
	.aresetn(aresetn),
	.spibaseclock(clk50),
	.usbcconn(usbaconn),
	.usbirq(usbairq),
	.s_axi(usbaif));

// CSR file acts as a region of uncached memory
// Access to register indices get mapped to LOAD/STORE
// and addresses get mapped starting at 0x80004000 + csroffset
// CSR module also acts as the interrupt generator
axi4CSRFile csrfileinstHart0(
	.aclk(aclk),
	.aresetn(aresetn),
	.cpuclocktime(cpuclocktimeHart0),
	.wallclocktime(wallclocktime),
	.retired(retiredHart0),
	// IRQ tracking
	.irqReq(irqReqHart0),
	// External interrupt wires
	.keyfifoempty(keyfifoempty),
	.usbirq({usbairq, usbcirq}),
	// Soft reset
	.sysresetn(sysresetn),
	// Shadow registers
	.mepc(mepcHart0),
	.mtvec(mtvecHart0),
	.sie(sieHart0),
	// Bus
	.s_axi(csrif) );

// XADC
axi4xadc xadcinst(
	.aclk(aclk),
	.clk10(clk10),
	.aresetn(aresetn),
	.s_axi(xadcif),
	.device_temp(device_temp) );

// DMA command queue
commandqueue dmacmdinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(dmaif),
	// Internal comms channel for DMA
	.fifoempty(dmafifoempty),
	.fifodout(dmafifodout),
	.fifore(dmafifore),
	.fifovalid(dmafifovalid),
    .devicestate({31'd0,dmabusy}));

// Audio command queue
commandqueue audiocmdinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(audioif),
	// Internal comms channel for APU
	.fifoempty(audiofifoempty),
	.fifodout(audiofifodout),
	.fifore(audiofifore),
	.fifovalid(audiofifovalid),
    .devicestate(audiobufferswapcount));

axi4opl2 opl2inst(
	.aclk(aclk),
	.aresetn(aresetn),
	.audioclock(clk50),
	.sampleout(opl2sampleout),
	.s_axi(opl2if) );

endmodule
