`timescale 1ns / 1ps
`default_nettype none

module tophat(
	// Board clock and reset
    input wire sys_clk
    //,input wire sysresetn
    // Debug LEDs
    ,output wire [3:0] leds
	// DDR3 SDRAM
	,output wire ddr3_reset_n
	,output wire [0:0] ddr3_cke
	,output wire [0:0] ddr3_ck_p 
	,output wire [0:0] ddr3_ck_n
	,output wire ddr3_ras_n 
	,output wire ddr3_cas_n 
	,output wire ddr3_we_n
	,output wire [2:0] ddr3_ba
	,output wire [13:0] ddr3_addr
	,output wire [0:0] ddr3_odt
	,output wire [1:0] ddr3_dm
	,inout wire [1:0] ddr3_dqs_p
	,inout wire [1:0] ddr3_dqs_n
	,inout wire [15:0] ddr3_dq
	// To video scanout chip
	,output wire vvsync
	,output wire vhsync
	,output wire vclk
	,output wire vde
	,output wire [11:0] vdat
	// Micro SD Card
	,input wire sdcard_miso
	,output wire sdcard_cs_n
	,output wire sdcard_clk
	,output wire sdcard_mosi
	,input wire sdcard_swtch
	// USB-C via MAX4320
	,input wire usbc_miso
	,output wire usbc_ss_n
	,output wire usbc_clk
	,output wire usbc_mosi
	,output wire usbc_resn
	,input wire usbc_int
	,input wire usbc_gpx
	// USB-A via MAX4321
	,input wire usba_miso
	,output wire usba_ss_n
	,output wire usba_clk
	,output wire usba_mosi
	,output wire usba_resn
	,input wire usba_int
	// DEBUG UART
	//,input wire debugrx
	//,output wire debugtx
	// Audio out
	,output wire au_sdin
	,output wire au_sclk
	,output wire au_lrclk
	,output wire au_mclk);

// --------------------------------------------------
// Clock and reset generator
// --------------------------------------------------

wire aresetn, preresetn;
wire init_calib_complete;
wire clk10, clkaudio, clk25, clk50, clk100, clkbus, clk166, clk200;

// Clock and reset generator
clockandreset clockandresetinst(
	.sys_clock_i(sys_clk),
	.init_calib_complete(init_calib_complete),
	.clk10(clk10),
	.clkaudio(clkaudio),
	.clk25(clk25),
	.clk50(clk50),
	.clk100(clk100),
	.clkbus(clkbus),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	.preresetn(preresetn));

// --------------------------------------------------
// DDR3 SDRAM wires
// --------------------------------------------------

ddr3sdramwires ddr3conn(
	.ddr3_reset_n(ddr3_reset_n),
	.ddr3_cke(ddr3_cke),
	.ddr3_ck_p(ddr3_ck_p), 
	.ddr3_ck_n(ddr3_ck_n),
	.ddr3_ras_n(ddr3_ras_n), 
	.ddr3_cas_n(ddr3_cas_n), 
	.ddr3_we_n(ddr3_we_n),
	.ddr3_ba(ddr3_ba),
	.ddr3_addr(ddr3_addr),
	.ddr3_odt(ddr3_odt),
	.ddr3_dm(ddr3_dm),
	.ddr3_dqs_p(ddr3_dqs_p),
	.ddr3_dqs_n(ddr3_dqs_n),
	.ddr3_dq(ddr3_dq),
	.init_calib_complete(init_calib_complete) );

// --------------------------------------------------
// SPI wires
// --------------------------------------------------

sdcardwires sdconn(
	.miso(sdcard_miso),
	.cs_n(sdcard_cs_n),
	.clk(sdcard_clk),
	.mosi(sdcard_mosi),
	.swtch(sdcard_swtch) );

max3420wires usbcconn(
	.miso(usbc_miso),
	.cs_n(usbc_ss_n),
	.clk(usbc_clk),
	.mosi(usbc_mosi),
	.resn(usbc_resn),
	.irq(usbc_int),
	.gpx(usbc_gpx));

max3420wires usbaconn(
	.miso(usba_miso),
	.cs_n(usba_ss_n),
	.clk(usba_clk),
	.mosi(usba_mosi),
	.resn(usba_resn),
	.irq(usba_int),
	.gpx(1'b0));

// --------------------------------------------------
// Audio wires
// --------------------------------------------------

audiowires i2sconn(
	.sdin(au_sdin),
	.sclk(au_sclk),
	.lrclk(au_lrclk),
	.mclk(au_mclk));

// --------------------------------------------------
// Hardware debug device
// --------------------------------------------------

/*debugbusif dbgbus();

hwdebug hwdebuginst(
	.aresetn(aresetn),
	.aclk(clkbus),
	.clk100(clk100),
	.m_dbg(dbgbus),
	.rx(debugrx),
	.tx(debugtx) );*/

// --------------------------------------------------
// SoC device
// --------------------------------------------------

tinysoc #(.RESETVECTOR(32'h0FFE0000)) socinstance(
	.aclk(clkbus),
	.clk10(clk10),
	.clkaudio(clkaudio),
	.clk25(clk25),
	.clk50(clk50),
	.clk100(clk100),
	.clk166(clk166),
	.clk200(clk200),
	.aresetn(aresetn),
	//.sysresetn(sysresetn), // interrupt
	.preresetn(preresetn),
	// Debug bus
	//.s_dbg(dbgbus),
	// Device wires
	.leds(leds),
	.ddr3conn(ddr3conn),
	.i2sconn(i2sconn),
	.vvsync(vvsync),
	.vhsync(vhsync),
	.vclk(vclk),
	.vde(vde),
	.vdat(vdat),
	.sdconn(sdconn),
	.usbcconn(usbcconn),
	.usbaconn(usbaconn) );

endmodule
