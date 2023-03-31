`timescale 1ns / 1ps

`include "shared.vh"

module dataunit(
	input wire aclk,
	input wire aresetn,
	// From control unit
	ibusif.slave s_ibus,
	// To memory
	axi4if.master databus,
	// To memory mapped devices
	axi4if.master devicebus);

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

typedef enum logic [1:0] {FETCH, READ, CACHEOP, WRITE} dataunitmode;
dataunitmode datamode = FETCH;

always @(posedge aclk) begin
	if (~aresetn) begin
		datamode <= FETCH;
	end else begin
	
		s_ibus.rdone <= 1'b0;
		s_ibus.wdone <= 1'b0;

		datare <= 1'b0;
		datawe <= 1'b0;
		dcacheop <= 2'b00;

		unique case(datamode)
			FETCH: begin
				addrs <= s_ibus.rstrobe ? s_ibus.raddr : s_ibus.waddr;
				datain <= s_ibus.wdata;
				datare <= s_ibus.rstrobe;
				datawe <= s_ibus.wstrobe;
				s_ibus.cdone <= 1'b0;
				dcacheop <= s_ibus.cstrobe ? s_ibus.dcacheop : 2'b00;
				datamode <= s_ibus.cstrobe ? CACHEOP : (s_ibus.rstrobe ? READ : (s_ibus.wstrobe ? WRITE : FETCH));
			end
			READ: begin
				s_ibus.rdone <= rready;
				s_ibus.rdata <= dataout;
				datamode <= rready ? FETCH : READ;
			end
			WRITE: begin
				s_ibus.wdone <= wready;
				datamode <= wready ? FETCH : WRITE;
			end
			CACHEOP: begin
				s_ibus.cdone <= wready;
				datamode <= wready ? FETCH : CACHEOP;
			end
		endcase

	end
end

endmodule
