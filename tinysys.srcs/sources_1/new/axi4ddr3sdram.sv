`timescale 1ns / 1ps

module axi4ddr3sdram(
	input wire aclk,
	input wire aresetn,
	input wire clk_sys_i,
	input wire clk_ref_i,
	axi4if.slave m_axi,
	ddr3sdramwires.def ddr3wires );

wire ui_clk;
wire ui_clk_sync_rst;
wire mmcm_locked;

//assign calib_done = mmcm_locked & init_calib_complete;

axi4if s_axi();

wire saresetn, maresetn;

axi_interconnect_0 axi4interconnectinst(
  .INTERCONNECT_ACLK(aclk),
  .INTERCONNECT_ARESETN(aresetn),

  .S00_AXI_ARESET_OUT_N(saresetn),
  .S00_AXI_ACLK(aclk),
  .S00_AXI_AWID(    1'b0/*m_axi.awid*/),
  .S00_AXI_AWADDR(  m_axi.awaddr),
  .S00_AXI_AWLEN(   m_axi.awlen),
  .S00_AXI_AWSIZE(  m_axi.awsize),
  .S00_AXI_AWBURST( m_axi.awburst),
  .S00_AXI_AWLOCK(  1'b0/*m_axi.awlock*/),
  .S00_AXI_AWCACHE( 4'b0011/*m_axi.awcache*/),
  .S00_AXI_AWPROT(  3'b000/*m_axi.awprot*/),
  .S00_AXI_AWQOS(   4'h0/*m_axi.awqos*/),
  .S00_AXI_AWVALID( m_axi.awvalid),
  .S00_AXI_AWREADY( m_axi.awready),
  .S00_AXI_WDATA(   m_axi.wdata),
  .S00_AXI_WSTRB(   m_axi.wstrb),
  .S00_AXI_WLAST(   m_axi.wlast),
  .S00_AXI_WVALID(  m_axi.wvalid),
  .S00_AXI_WREADY(  m_axi.wready),
  .S00_AXI_BID(     /*m_axi.bid*/),
  .S00_AXI_BRESP(   m_axi.bresp),
  .S00_AXI_BVALID(  m_axi.bvalid),
  .S00_AXI_BREADY(  m_axi.bready),
  .S00_AXI_ARID(    1'b0/*m_axi.arid*/),
  .S00_AXI_ARADDR(  m_axi.araddr),
  .S00_AXI_ARLEN(   m_axi.arlen),
  .S00_AXI_ARSIZE(  m_axi.arsize),
  .S00_AXI_ARBURST( m_axi.arburst),
  .S00_AXI_ARLOCK(  1'b0/*m_axi.arlock*/),
  .S00_AXI_ARCACHE( 4'b0011/*m_axi.arcache*/),
  .S00_AXI_ARPROT(  3'b000/*m_axi.arprot*/),
  .S00_AXI_ARQOS(   4'h0/*m_axi.arqos*/),
  .S00_AXI_ARVALID( m_axi.arvalid),
  .S00_AXI_ARREADY( m_axi.arready),
  .S00_AXI_RID(     /*m_axi.rid*/),
  .S00_AXI_RDATA(   m_axi.rdata),
  .S00_AXI_RRESP(   m_axi.rresp),
  .S00_AXI_RLAST(   m_axi.rlast),
  .S00_AXI_RVALID(  m_axi.rvalid),
  .S00_AXI_RREADY(  m_axi.rready),

  .M00_AXI_ARESET_OUT_N(maresetn),
  .M00_AXI_ACLK(    ui_clk),
  .M00_AXI_AWID(    /*s_axi.awid*/),
  .M00_AXI_AWADDR(  s_axi.awaddr),
  .M00_AXI_AWLEN(   s_axi.awlen),
  .M00_AXI_AWSIZE(  s_axi.awsize),
  .M00_AXI_AWBURST( s_axi.awburst),
  .M00_AXI_AWLOCK(  /*s_axi.awlock*/),
  .M00_AXI_AWCACHE( /*s_axi.awcache*/),
  .M00_AXI_AWPROT(  /*s_axi.awprot*/),
  .M00_AXI_AWQOS(   /*s_axi.awqos*/),
  .M00_AXI_AWVALID( s_axi.awvalid),
  .M00_AXI_AWREADY( s_axi.awready),
  .M00_AXI_WDATA(   s_axi.wdata),
  .M00_AXI_WSTRB(   s_axi.wstrb),
  .M00_AXI_WLAST(   s_axi.wlast),
  .M00_AXI_WVALID(  s_axi.wvalid),
  .M00_AXI_WREADY(  s_axi.wready),
  .M00_AXI_BID(     0/*s_axi.bid*/),
  .M00_AXI_BRESP(   s_axi.bresp),
  .M00_AXI_BVALID(  s_axi.bvalid),
  .M00_AXI_BREADY(  s_axi.bready),
  .M00_AXI_ARID(    /*s_axi.arid*/),
  .M00_AXI_ARADDR(  s_axi.araddr),
  .M00_AXI_ARLEN(   s_axi.arlen),
  .M00_AXI_ARSIZE(  s_axi.arsize),
  .M00_AXI_ARBURST( s_axi.arburst),
  .M00_AXI_ARLOCK(  /*s_axi.arlock*/),
  .M00_AXI_ARCACHE( /*s_axi.arcache*/),
  .M00_AXI_ARPROT(  /*s_axi.arprot*/),
  .M00_AXI_ARQOS(   /*s_axi.arqos*/),
  .M00_AXI_ARVALID( s_axi.arvalid),
  .M00_AXI_ARREADY( s_axi.arready),
  .M00_AXI_RID(     0/*s_axi.rid*/),
  .M00_AXI_RDATA(   s_axi.rdata),
  .M00_AXI_RRESP(   s_axi.rresp),
  .M00_AXI_RLAST(   s_axi.rlast),
  .M00_AXI_RVALID(  s_axi.rvalid),
  .M00_AXI_RREADY(  s_axi.rready) );

mig_7series_0 ddr3instance (
    // memory interface ports
    .ddr3_addr                      (ddr3wires.ddr3_addr),
    .ddr3_ba                        (ddr3wires.ddr3_ba),
    .ddr3_cas_n                     (ddr3wires.ddr3_cas_n),
    .ddr3_ck_n                      (ddr3wires.ddr3_ck_n),
    .ddr3_ck_p                      (ddr3wires.ddr3_ck_p),
    .ddr3_cke                       (ddr3wires.ddr3_cke),
    .ddr3_ras_n                     (ddr3wires.ddr3_ras_n),
    .ddr3_reset_n                   (ddr3wires.ddr3_reset_n),
    .ddr3_we_n                      (ddr3wires.ddr3_we_n),
    .ddr3_dq                        (ddr3wires.ddr3_dq),
    .ddr3_dqs_n                     (ddr3wires.ddr3_dqs_n),
    .ddr3_dqs_p                     (ddr3wires.ddr3_dqs_p),
    .ddr3_dm                        (ddr3wires.ddr3_dm),
    .ddr3_odt                       (ddr3wires.ddr3_odt),

    // application interface ports
    .ui_clk                         (ui_clk),          // feeds back into axi4if.aclk to drive the entire bus
    .ui_clk_sync_rst                (ui_clk_sync_rst),
    .init_calib_complete            (ddr3wires.init_calib_complete),
    .device_temp					(), // unused

    .mmcm_locked                    (mmcm_locked), // unused
    .aresetn                        (maresetn),

    .app_sr_req                     (1'b0), // unused
    .app_ref_req                    (1'b0), // unused
    .app_zq_req                     (1'b0), // unused
    .app_sr_active                  (), // unused
    .app_ref_ack                    (), // unused
    .app_zq_ack                     (), // unused

    // slave interface write address ports
    .s_axi_awid                     (4'h0),
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

    // slave interface write data ports
    .s_axi_wdata                    (s_axi.wdata),
    .s_axi_wstrb                    (s_axi.wstrb),
    .s_axi_wlast                    (s_axi.wlast),
    .s_axi_wvalid                   (s_axi.wvalid),
    .s_axi_wready                   (s_axi.wready),

    // slave interface write response ports
    .s_axi_bid                      (), // unused
    .s_axi_bresp                    (s_axi.bresp),
    .s_axi_bvalid                   (s_axi.bvalid),
    .s_axi_bready                   (s_axi.bready),

    // slave interface read address ports
    .s_axi_arid                     (4'h0),
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

    // slave interface read data ports
    .s_axi_rid                      (), // unused
    .s_axi_rdata                    (s_axi.rdata),
    .s_axi_rresp                    (s_axi.rresp),
    .s_axi_rlast                    (s_axi.rlast),
    .s_axi_rvalid                   (s_axi.rvalid),
    .s_axi_rready                   (s_axi.rready),
    // system clock ports
    .sys_clk_i                      (clk_sys_i), // 100mhz
    // reference clock ports
    .clk_ref_i                      (clk_ref_i), // 200mhz
    .sys_rst                        (aresetn) );

endmodule
