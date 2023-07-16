`timescale 1ns / 1ps

module axi4uart (
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	output wire uart_rxd_out,
	input wire uart_txd_in,
	output wire uartirq );

axi_uartlite_0 uartliteinstance  (
  .s_axi_aclk(aclk),
  .s_axi_aresetn(aresetn),
  .interrupt(uartirq),
  .s_axi_awaddr(s_axi.awaddr[3:0]),
  .s_axi_awvalid(s_axi.awvalid),
  .s_axi_awready(s_axi.awready),
  .s_axi_wdata(s_axi.wdata[31:0]),
  .s_axi_wstrb(s_axi.wstrb[3:0]),
  .s_axi_wvalid(s_axi.wvalid),
  .s_axi_wready(s_axi.wready),
  .s_axi_bresp(s_axi.bresp[1:0]),
  .s_axi_bvalid(s_axi.bvalid),
  .s_axi_bready(s_axi.bready),
  .s_axi_araddr(s_axi.araddr[3:0]),
  .s_axi_arvalid(s_axi.arvalid),
  .s_axi_arready(s_axi.arready),
  .s_axi_rdata(s_axi.rdata[31:0]),
  .s_axi_rresp(s_axi.rresp[1:0]),
  .s_axi_rvalid(s_axi.rvalid),
  .s_axi_rready(s_axi.rready),
  .rx(uart_txd_in),
  .tx(uart_rxd_out)
);

endmodule