`timescale 1ns / 1ps

module bootmem(
	input wire aclk,
	input wire aresetn,
	axi4if.slave axi_s);

wire rsta_busy;
wire rstb_busy;
bootmem64k bootmem64kinst (
  .rsta_busy(rsta_busy),
  .rstb_busy(rstb_busy),
  .s_aclk(aclk),
  .s_aresetn(aresetn),
  .s_axi_awid(4'h0),
  .s_axi_awaddr(axi_s.awaddr),
  .s_axi_awlen(axi_s.awlen),
  .s_axi_awsize(axi_s.awsize),
  .s_axi_awburst(axi_s.awburst),
  .s_axi_awvalid(axi_s.awvalid),
  .s_axi_awready(axi_s.awready),
  .s_axi_wdata(axi_s.wdata),
  .s_axi_wstrb(axi_s.wstrb),
  .s_axi_wlast(axi_s.wlast),
  .s_axi_wvalid(axi_s.wvalid),
  .s_axi_wready(axi_s.wready),
  .s_axi_bid(),
  .s_axi_bresp(axi_s.bresp),
  .s_axi_bvalid(axi_s.bvalid),
  .s_axi_bready(axi_s.bready),
  .s_axi_arid(4'h0),
  .s_axi_araddr(axi_s.araddr),
  .s_axi_arlen(axi_s.arlen),
  .s_axi_arsize(axi_s.arsize),
  .s_axi_arburst(axi_s.arburst),
  .s_axi_arvalid(axi_s.arvalid),
  .s_axi_arready(axi_s.arready),
  .s_axi_rid(),
  .s_axi_rdata(axi_s.rdata),
  .s_axi_rresp(axi_s.rresp),
  .s_axi_rlast(axi_s.rlast),
  .s_axi_rvalid(axi_s.rvalid),
  .s_axi_rready(axi_s.rready) );

endmodule
