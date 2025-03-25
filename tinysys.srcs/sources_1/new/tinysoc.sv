`timescale 1ns / 1ps
`default_nettype none

import axi4pkg::*;

module tinysoc #(
	parameter int RESETVECTOR = 32'd0
) (
	input wire aclk,
	input wire clk10,
	input wire clkaudio,
	input wire clk25,
	input wire clk100,
	input wire clk125,
	input wire clk166,
	input wire clk200,
	input wire aresetn,
	input wire rst10n,
	input wire rst25n,
	input wire rst100n,
	input wire rstaudion,
	input wire preresetn,
	// LEDs
	output wire [5:0] leds,
	// ESP32
	//output wire esp_txd0_out,
	//input wire esp_rxd0_in,
	output wire esp_txd1_out,
	input wire esp_rxd1_in,
	input wire fpga_reboot,
	output wire esp_ena,
	// Video output
	output wire HDMI_CLK_p,
	output wire HDMI_CLK_n,
	output wire [2:0] HDMI_TMDS_p,
	output wire [2:0] HDMI_TMDS_n,
	//input wire HDMI_CEC,
	//inout wire HDMI_SDA,
	//inout wire HDMI_SCL,
	//input wire HDMI_HPD,
	// DDR3
	ddr3sdramwires.def ddr3conn,
	// SDCard
	sdcardwires.def sdconn );

// --------------------------------------------------
// Bus lines
// --------------------------------------------------

axi4if instructionbusHart0();	// Fetch bus for HART#0
axi4if instructionbusHart1();	// Fetch bus for HART#1
axi4if databusHart0();			// Data bus for HART#0
axi4if databusHart1();			// Data bus for HART#1
axi4if devicebusHart0();		// Direct access to devices from data unit of HART#0
axi4if devicebusHart1();		// Direct access to devices from data unit of HART#1

axi4if videobus();				// Video output unit bus
axi4if romcopybus();			// Bus for boot ROM copy
axi4if audiobus();				// Bus for audio device output

axi4if memorybus();				// Arbitrated access to main memory
axi4if devicebus();				// Arbitrated access to devices

axi4if scratchif();				// Sub bus: SPAD scratchpad memory (uncached)
axi4if ledif();					// Sub bus: LED contol device (debug LEDs)
axi4if vpucmdif();				// Sub bus: VPU command fifo (video scan out logic)
axi4if spiif();					// Sub bus: SPI control device (sdcard)
axi4if csrif0();				// Sub bus: CSR#0 file device (control registers)
axi4if csrif1();				// Sub bus: CSR#1 file device (control registers)
axi4if audioif();				// Sub bus: APU i2s audio output unit (raw audio)
axi4if uartif();				// Sub bus: UART serial I/O for ESP32 communication
axi4if mailif();				// Sub bus: MAIL mailbox for HART communication

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
// Startup reset timer for ESP32
// --------------------------------------------------

logic [7:0] esp32resetshift;
assign esp_ena = esp32resetshift[7];

always @(posedge clk10) begin
	if (~rst10n) begin
		esp32resetshift <= 8'b00000000;
	end else begin
		// Holds esp reset set to ground, then to floating so that
		// the external pullup resistor can hold the enable line high
		esp32resetshift <= {esp32resetshift[6:0], 1'b1};
	end
end

// --------------------------------------------------
// Common wires
// --------------------------------------------------

wire romReady;

// --------------------------------------------------
// HART#0
// --------------------------------------------------

wire sieHart0, cpuresetreq0;
wire [63:0] cpuclocktimeHart0;
wire [63:0] retiredHart0;
wire [31:0] mepcHart0;
wire [31:0] mtvecHart0;
wire [31:0] PCHart0;
wire [1:0] irqReqHart0;

riscv #( .CSRBASE(16'h8009), .RESETVECTOR(RESETVECTOR)) hart0(
	.aclk(aclk),
	.aresetn(aresetn),
	.romReady(romReady),
	.sie(sieHart0),
	.mepc(mepcHart0),
	.mtvec(mtvecHart0),
	.pc_out(PCHart0),
	.cpuresetreq(cpuresetreq0),
	.irqReq(irqReqHart0),
	.instructionbus(instructionbusHart0),
	.databus(databusHart0),
	.devicebus(devicebusHart0),
	.cpuclocktime(cpuclocktimeHart0),
	.retired(retiredHart0));

// --------------------------------------------------
// HART#1
// --------------------------------------------------

wire sieHart1, cpuresetreq1;
wire [63:0] cpuclocktimeHart1;
wire [63:0] retiredHart1;
wire [31:0] mepcHart1;
wire [31:0] mtvecHart1;
wire [31:0] PCHart1;
wire [1:0] irqReqHart1;

riscv #( .CSRBASE(16'h800A), .RESETVECTOR(RESETVECTOR)) hart1(
	.aclk(aclk),
	.aresetn(aresetn),
	.romReady(romReady),
	.sie(sieHart1),
	.mepc(mepcHart1),
	.mtvec(mtvecHart1),
	.pc_out(PCHart1),
	.cpuresetreq(cpuresetreq1),
	.irqReq(irqReqHart1),
	.instructionbus(instructionbusHart1),
	.databus(databusHart1),
	.devicebus(devicebusHart1),
	.cpuclocktime(cpuclocktimeHart1),
	.retired(retiredHart1));

// --------------------------------------------------
// Video output unit
// --------------------------------------------------

wire vpufifoempty;
wire [31:0] vpufifodout;
wire vpufifore;
wire vpufifovalid;
wire [31:0] vpustate;
wire [31:0] tx_sdout;
wire audiosampleclk;
videocore VPU(
	.aclk(aclk),
	.clk25(clk25),
	.clk125(clk125),
	.aresetn(aresetn),
	.rst25n(rst25n),
	.m_axi(videobus),
	.HDMI_CLK_p(HDMI_CLK_p),
	.HDMI_CLK_n(HDMI_CLK_n),
	.HDMI_TMDS_p(HDMI_TMDS_p),
	.HDMI_TMDS_n(HDMI_TMDS_n),
	//.HDMI_CEC(HDMI_CEC),
	//.HDMI_SDA(HDMI_SDA),
	//.HDMI_SCL(HDMI_SCL),
	//.HDMI_HPD(HDMI_HPD),
	.audioclock(audiosampleclk),
	.audiosampleLR(tx_sdout),
	.vpufifoempty(vpufifoempty),
	.vpufifodout(vpufifodout),
	.vpufifore(vpufifore),
	.vpufifovalid(vpufifovalid),
	.vpustate(vpustate));

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

axi4i2saudio APU(
	.aclk(aclk),				// Bus clock
	.aresetn(aresetn),
	.rstaudion(rstaudion),
    .audioclock(clkaudio),

	.m_axi(audiobus),			// Memory access

	.abempty(audiofifoempty),	// APU command FIFO
	.abvalid(audiofifovalid),
	.audiore(audiofifore),
    .audiodin(audiofifodout),
    .swapcount(audiobufferswapcount),

	.audiosampleclk(audiosampleclk),
    .tx_sdout(tx_sdout));

// --------------------------------------------------
// Traffic between master units / memory / devices
// --------------------------------------------------

arbitersmall arbitersmallinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s({devicebusHart1, devicebusHart0}),
	.axi_m(devicebus) );

arbiterlarge arbiterlargeinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.axi_s({romcopybus, audiobus, videobus, databusHart1, databusHart0, instructionbusHart1, instructionbusHart0}),
	.axi_m(memorybus) );

// --------------------------------------------------
// RAM
// --------------------------------------------------

// dev   start     end      size
// DDR3: 00000000  0FFFFFFF (256 Mbytes, 16 byte r/w only)

axi4ddr3sdram axi4ddr3sdraminst(
	.aclk(aclk),
	.aresetn(aresetn),
	.preresetn(preresetn),
	.clk_sys_i(clk166),
	.clk_ref_i(clk200),
	.m_axi(memorybus),
	.ddr3conn(ddr3conn));

// --------------------------------------------------
// Memory mapped device router
// --------------------------------------------------

// NOTE: CSR offsets are word offsets therefore the
// address space is for 4Kbytes of data yet we can
// access 16KBytes worth of data.

// 16bit (64K) address space for up to 255 devices
// dev   start     end       addrs[19:16]  size  notes
// SPAD: 80000000  8xx0FFFF  4'b0000  64KB	 Scratchpad (16Kbytes)
// LEDS: 8xx10000  8xx1FFFF  4'b0001  64KB	 Debug LEDs
// VPUC: 8xx20000  8xx2FFFF  4'b0010  64KB	 Video Processing Unit
// SDCC: 8xx30000  8xx3FFFF  4'b0011  64KB	 SDCard SPI Unit
// ----: 8xx40000  8xx4FFFF  4'b0100  64KB	 Unused
// ----: 8xx50000  8xx5FFFF  4'b0101  64KB	 Reserved for future use
// APUC: 8xx60000  8xx6FFFF  4'b0110  64KB	 Audio Processing Unit / Mixer
// MAIL: 8xx70000  8xx7FFFF  4'b0111  64KB	 MAIL inter-HART comm (16Kbytes)
// UART: 8xx80000  8xx8FFFF  4'b1000  64KB	 UART HART <-> ESP32-C6 comm
// CSR0: 8xx90000  8xx9FFFF  4'b1001  64KB	 CSR#0
// CSR1: 8xxA0000  8xxAFFFF  4'b1010  64KB	 CSR#1
// ----: 8xxB0000  8xxBFFFF  4'b1011  64KB	 Unused
// ----: 8xxC0000  8xxCFFFF  4'b1100  64KB	 Unused
// ----: 8xxD0000  8xxDFFFF  4'b1101  64KB	 Unused
// ----: 8xxE0000  8xxEFFFF  4'b1110  64KB	 Unused
// ----: 8xxF0000  8xxFFFFF  4'b1111  64KB	 Unused

devicerouter devicerouterinst(
	.addressmask({
		4'b1010,		// CRS1 CSR file for HART#1
		4'b1001,		// CRS0 CSR file for HART#0
		4'b1000,		// UART ESP32 to RISC-V UART channel
		4'b0111,		// MAIL Mailbox for HART-to-HART commmunication
		4'b0110,		// APUC	Audio Processing Unit Command Fifo
		4'b0011,		// SDCC SDCard access via SPI
		4'b0010,		// VPUC Graphics Processing Unit Command Fifo
		4'b0001,		// LEDS Debug / Status LED interface
		4'b0000}),		// SPAD Scratchpad for inter-core data transfer
	.axi_s(devicebus),
    .axi_m({csrif1, csrif0, uartif, mailif, audioif, spiif, vpucmdif, ledif, scratchif}));

// --------------------------------------------------
// Interrupt wires
// --------------------------------------------------

wire keyirq;
wire uartirq;

// --------------------------------------------------
// Memory mapped devices
// --------------------------------------------------

axi4register leddevice(
	.aclk(aclk),
	.clk10(clk10),
	.aresetn(aresetn),
	.rst10n(rst10n),
	.s_axi(ledif),
	.regio(leds) );

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
	.rst100n(rst100n),
	.spibaseclock(clk100),
	.sdconn(sdconn),
	.keyirq(keyirq),
	.s_axi(spiif));

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

axi4CSRFile #( .HARTID(4'd0)) csrfile0 (
	.aclk(aclk),
	.aresetn(aresetn),
	.cpuclocktime(cpuclocktimeHart0),
	.wallclocktime(wallclocktime),
	.retired(retiredHart0),
	.pc_in(PCHart0),
	// IRQ state for hardware or timer
	.irqReq(irqReqHart0),
	// External hardware interrupt wires
	.keyirq(keyirq),
	.uartirq(uartirq),
	// CPU reset
	.cpuresetreq(cpuresetreq0),
	// Shadow registers
	.mepc(mepcHart0),
	.mtvec(mtvecHart0),
	.sie(sieHart0),
	// Bus
	.s_axi(csrif0) );

axi4CSRFile #( .HARTID(4'd1)) csrfile1 (
	.aclk(aclk),
	.aresetn(aresetn),
	.cpuclocktime(cpuclocktimeHart1),
	.wallclocktime(wallclocktime),
	.retired(retiredHart1),
	.pc_in(PCHart1),
	// IRQ state for hardware or timer
	.irqReq(irqReqHart1),
	// External hardware interrupt wires
	.keyirq(keyirq),
	.uartirq(uartirq),
	// CPU reset
	.cpuresetreq(cpuresetreq1),
	// Shadow registers
	.mepc(mepcHart1),
	.mtvec(mtvecHart1),
	.sie(sieHart1),
	// Bus
	.s_axi(csrif1) );

axi4uart uartinst(
	.aclk(aclk),
	.uartbaseclock(clk10),
	.aresetn(aresetn),
	.rst10n(rst10n),
	.uartrx(esp_rxd1_in),
	.uarttx(esp_txd1_out),
	.s_axi(uartif),
	.uartirq(uartirq) );

axi4mail maildevice(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(mailif));
	
axi4scratch scratchpaddevice(
	.aclk(aclk),
	.aresetn(aresetn),
	.s_axi(scratchif));

endmodule
