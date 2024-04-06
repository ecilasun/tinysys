`timescale 1ns / 1ps

module axi4uart(
	input wire aclk,
	input wire aresetn,
	input wire uartrx,
	output wire uarttx,
	axi4if.slave s_axi,
	output wire uartinterrupt );

axiuartlite uartinternal (
  .s_axi_aclk(aclk),
  .s_axi_aresetn(aresetn),
  .interrupt(uartinterrupt),
  .s_axi_awaddr(s_axi.awaddr),
  .s_axi_awvalid(s_axi.awvalid),
  .s_axi_awready(s_axi.awready),
  .s_axi_wdata(s_axi.wdata),
  .s_axi_wstrb(s_axi.wstrb),
  .s_axi_wvalid(s_axi.wvalid),
  .s_axi_wready(s_axi.wready),
  .s_axi_bresp(s_axi.bresp),
  .s_axi_bvalid(s_axi.bvalid),
  .s_axi_bready(s_axi.bready),
  .s_axi_araddr(s_axi.araddr),
  .s_axi_arvalid(s_axi.arvalid),
  .s_axi_arready(s_axi.arready),
  .s_axi_rdata(s_axi.rdata),
  .s_axi_rresp(s_axi.rresp),
  .s_axi_rvalid(s_axi.rvalid),
  .s_axi_rready(s_axi.rready),
  .rx(uartrx),
  .tx(uarttx) );

endmodule
