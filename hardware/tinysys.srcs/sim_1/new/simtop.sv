`timescale 1ns / 1ps

module simtop();

logic boardclock;
logic old_esp_ena;

initial begin
	old_esp_ena = 1'bz;
	boardclock = 1'bz;
	#80;
	boardclock = 1'b0;
end

wire [3:0] ledout;

wire sdcard_mosi;
wire sdcard_miso = sdcard_mosi; // nul device
wire usba_mosi;
wire usba_miso = usba_mosi; // nul device

// DDR3 simulation model
wire ddr3_reset_n;
wire [0:0]   ddr3_cke;
wire [0:0]   ddr3_ck_p; 
wire [0:0]   ddr3_ck_n;
wire ddr3_ras_n; 
wire ddr3_cas_n;
wire ddr3_we_n;
wire [2:0]   ddr3_ba;
wire [13:0]  ddr3_addr;
wire [0:0]   ddr3_odt;
wire [1:0]   ddr3_dm;
wire [1:0]   ddr3_dqs_p;
wire [1:0]   ddr3_dqs_n;
wire [15:0]  ddr3_dq;

ddr3_model ddr3simmod(
    .rst_n(ddr3_reset_n),
    .ck(ddr3_ck_p),
    .ck_n(ddr3_ck_n),
    .cke(ddr3_cke),
    .cs_n(1'b0),
    .ras_n(ddr3_ras_n),
    .cas_n(ddr3_cas_n),
    .we_n(ddr3_we_n),
    .dm_tdqs(ddr3_dm),
    .ba(ddr3_ba),
    .addr(ddr3_addr),
    .dq(ddr3_dq),
    .dqs(ddr3_dqs_p),
    .dqs_n(ddr3_dqs_n),
    .tdqs_n(), // out
    .odt(ddr3_odt) );

wire esp_ena;
tophat main(
    .sys_clk(boardclock),
    // LEDs
    .leds(ledout),
	// DDR3 SDRAM
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
	// Micro SD Card
	.sdcard_miso(sdcard_miso),
	.sdcard_cs_n(),
	.sdcard_clk(),
	.sdcard_mosi(sdcard_mosi),
	.sdcard_swtch(1'b0),
	// Coprocessor via ESP32-C6-WROOM-1-N8 (only on rev. 2E boards)
	.esp_txd1_out(),
	.esp_rxd1_in(1'b1),
	.fpga_reboot(1'b0),
	.esp_ena(esp_ena),
	// HDMI
	.HDMI_CLK_p(),
	.HDMI_CLK_n(),
	.HDMI_TMDS_p(),
	.HDMI_TMDS_n()	);

always begin
	#10
	boardclock = ~boardclock;
	if (esp_ena != old_esp_ena) begin
		old_esp_ena = esp_ena;
		$display("ESP32 new state: %h", esp_ena);
	end
end

endmodule
