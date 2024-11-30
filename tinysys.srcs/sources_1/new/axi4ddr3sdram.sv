`timescale 1ns / 1ps

module axi4ddr3sdram(
	input wire aclk,
	input wire aresetn,
    input wire preresetn,
	input wire clk_sys_i,
	input wire clk_ref_i,
	axi4if.slave m_axi,
	ddr3sdramwires.def ddr3conn);

// --------------------------------------------------
// AXI4 re-timer from 100MHz to DDR3 ui_clk
// --------------------------------------------------

wire aresetm;
wire ui_clk;
wire ui_clk_sync_rst;

axi4if s_axi();

axi4retimer axi4retimerinst(
	.aresetn(preresetn),
	.srcclk(aclk),
	.srcbus(m_axi),
	.destclk(ui_clk),
	.destbus(s_axi),
	.destrst(aresetm) );

// --------------------------------------------------
// MIG7 - AXI4
// --------------------------------------------------

wire init_calib_complete;
wire mmcm_locked;
assign ddr3conn.init_calib_complete = init_calib_complete & mmcm_locked;

mig_7series_0 ddr3instance (
    // memory interface ports
    .ddr3_addr                      (ddr3conn.ddr3_addr),
    .ddr3_ba                        (ddr3conn.ddr3_ba),
    .ddr3_cas_n                     (ddr3conn.ddr3_cas_n),
    .ddr3_ck_n                      (ddr3conn.ddr3_ck_n),
    .ddr3_ck_p                      (ddr3conn.ddr3_ck_p),
    .ddr3_cke                       (ddr3conn.ddr3_cke),
    .ddr3_ras_n                     (ddr3conn.ddr3_ras_n),
    .ddr3_reset_n                   (ddr3conn.ddr3_reset_n),
    .ddr3_we_n                      (ddr3conn.ddr3_we_n),
    .ddr3_dq                        (ddr3conn.ddr3_dq),
    .ddr3_dqs_n                     (ddr3conn.ddr3_dqs_n),
    .ddr3_dqs_p                     (ddr3conn.ddr3_dqs_p),
    .ddr3_dm                        (ddr3conn.ddr3_dm),
    .ddr3_odt                       (ddr3conn.ddr3_odt),
    .init_calib_complete            (init_calib_complete),

    // application interface ports
    .ui_clk                         (ui_clk),
    .ui_clk_sync_rst                (ui_clk_sync_rst),
    .device_temp					(),

    .mmcm_locked                    (mmcm_locked),
    .aresetn                        (aresetm),

    .app_sr_req                     (1'b0), // unused
    .app_ref_req                    (1'b0), // unused
    .app_zq_req                     (1'b0), // unused
    .app_sr_active                  (), // unused
    .app_ref_ack                    (), // unused
    .app_zq_ack                     (), // unused

    // write address ports
    .s_axi_awid                     (2'd0),
    .s_axi_awaddr                   (s_axi.awaddr[27:0]),
    .s_axi_awlen                    (s_axi.awlen),
    .s_axi_awsize                   (s_axi.awsize),
    .s_axi_awburst                  (s_axi.awburst),
    .s_axi_awlock                   (1'b0),
    .s_axi_awcache                  (4'b0011),
    .s_axi_awprot                   (3'b000),
    .s_axi_awqos                    (4'h0),
    .s_axi_awvalid                  (s_axi.awvalid),
    .s_axi_awready                  (s_axi.awready),

    // write data ports
    .s_axi_wdata                    (s_axi.wdata),
    .s_axi_wstrb                    (s_axi.wstrb),
    .s_axi_wlast                    (s_axi.wlast),
    .s_axi_wvalid                   (s_axi.wvalid),
    .s_axi_wready                   (s_axi.wready),

    // write response ports
    .s_axi_bid                      (), // unused
    .s_axi_bresp                    (s_axi.bresp),
    .s_axi_bvalid                   (s_axi.bvalid),
    .s_axi_bready                   (s_axi.bready),

    // read address ports
    .s_axi_arid                     (2'd0),
    .s_axi_araddr                   (s_axi.araddr[27:0]),
    .s_axi_arlen                    (s_axi.arlen),
    .s_axi_arsize                   (s_axi.arsize),
    .s_axi_arburst                  (s_axi.arburst),
    .s_axi_arlock                   (1'b0),
    .s_axi_arcache                  (4'b0011),
    .s_axi_arprot                   (3'b000),
    .s_axi_arqos                    (4'h0),
    .s_axi_arvalid                  (s_axi.arvalid),
    .s_axi_arready                  (s_axi.arready),

    // read data ports
    .s_axi_rid                      (), // unused
    .s_axi_rdata                    (s_axi.rdata),
    .s_axi_rresp                    (s_axi.rresp),
    .s_axi_rlast                    (s_axi.rlast),
    .s_axi_rvalid                   (s_axi.rvalid),
    .s_axi_rready                   (s_axi.rready),
    // system clock ports
    .sys_clk_i                      (clk_sys_i), // 166mhz
    // reference clock ports
    .clk_ref_i                      (clk_ref_i), // 200mhz
    .sys_rst                        (preresetn) );

endmodule
