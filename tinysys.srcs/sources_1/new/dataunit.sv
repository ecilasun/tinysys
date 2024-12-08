`timescale 1ns / 1ps
`default_nettype none

`include "shared.vh"

module dataunit(
	input wire aclk,
	input wire aresetn,
	// From control unit
	ibusif.slave s_ibus,
	// To memory
	axi4if.master databus,
	// To memory mapped devices
	axi4if.master devicebus );

logic datare = 1'b0;
logic [3:0] datawe = 4'h0;
wire [31:0] dataout;
logic [31:0] addrs = 32'd0;
logic [31:0] datain = 32'd0;
logic [4:0] dreg = 5'd0;
logic [4:0] wbtype = 5'd0;
logic [1:0] dcacheop = 2'b00;

wire rready, wready;

datacache datacacheinst(
	.aclk(aclk),
	.aresetn(aresetn),
	.dcacheop(dcacheop),
	.addr(addrs),
	.din(datain),
	.dout(dataout),
	.wstrb(datawe),
	.ren(datare),
	.rready(rready),
	.wready(wready),
	.a4buscached(databus),
	.a4busuncached(devicebus) );

typedef enum logic [1:0] {WCMD, WREAD, WCACHEOP, WWRITE} dataunitmode;
dataunitmode datamode = WCMD;

always_comb begin
	s_ibus.rdone = rready;
	s_ibus.rdata = dataout;
	s_ibus.wdone = wready;
	s_ibus.cdone = wready;
end

always @(posedge aclk) begin
	if (~aresetn) begin
		datare <= 1'b0;
		datawe <= 1'b0;
		dcacheop <= 2'b00;
		datamode <= WCMD;
	end else begin
		datare <= 1'b0;
		datawe <= 1'b0;
		dcacheop <= 2'b00;
		unique case(datamode)
			WCMD: begin
				addrs <= s_ibus.rstrobe ? s_ibus.raddr : s_ibus.waddr;
				datain <= s_ibus.wdata;
				datare <= s_ibus.rstrobe;
				datawe <= s_ibus.wstrobe;
				dcacheop <= s_ibus.cstrobe ? s_ibus.dcacheop : 2'b00;
				datamode <= s_ibus.cstrobe ? WCACHEOP : (s_ibus.rstrobe ? WREAD : (s_ibus.wstrobe ? WWRITE : WCMD));
			end
			WREAD: begin
				datamode <= rready ? WCMD : WREAD;
			end
			WWRITE: begin
				datamode <= wready ? WCMD : WWRITE;
			end
			WCACHEOP: begin
				datamode <= wready ? WCMD : WCACHEOP;
			end
		endcase
	end
end

endmodule
