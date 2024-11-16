module axi4scratch(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi);

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// Memory - 64Kbytes
// --------------------------------------------------

scratchram scratchraminst(
  .s_aclk(aclk),
  .s_aresetn(delayedresetn),
  .s_axi_awid(4'd0),
  .s_axi_awaddr(s_axi.awaddr),
  .s_axi_awlen(s_axi.awlen),
  .s_axi_awsize(s_axi.awsize),
  .s_axi_awburst(s_axi.awburst),
  .s_axi_awvalid(s_axi.awvalid),
  .s_axi_awready(s_axi.awready),
  .s_axi_wdata(s_axi.wdata[31:0]),
  .s_axi_wstrb(s_axi.wstrb[3:0]),
  .s_axi_wlast(s_axi.wlast),
  .s_axi_wvalid(s_axi.wvalid),
  .s_axi_wready(s_axi.wready),
  .s_axi_bid(),
  .s_axi_bresp(s_axi.bresp),
  .s_axi_bvalid(s_axi.bvalid),
  .s_axi_bready(s_axi.bready),
  .s_axi_arid(4'd0),
  .s_axi_araddr(s_axi.araddr),
  .s_axi_arlen(s_axi.arlen),
  .s_axi_arsize(s_axi.arsize),
  .s_axi_arburst(s_axi.arburst),
  .s_axi_arvalid(s_axi.arvalid),
  .s_axi_arready(s_axi.arready),
  .s_axi_rid(),
  .s_axi_rdata(s_axi.rdata[31:0]),
  .s_axi_rresp(s_axi.rresp),
  .s_axi_rlast(s_axi.rlast),
  .s_axi_rvalid(s_axi.rvalid),
  .s_axi_rready(s_axi.rready) );

endmodule
