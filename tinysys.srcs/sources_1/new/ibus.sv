`timescale 1ns / 1ps

interface ibusif;

	// Read
	logic [31:0] raddr;
	logic rstrobe;
	logic [31:0] rdata;
	logic rdone;

	// Write
	logic [31:0] waddr;
	logic [31:0] wdata;
	logic [3:0] wstrobe;
	logic wdone;

	// Cache op
	logic [1:0] dcacheop;
	logic [1:0] icacheop;
	logic cstrobe;
	logic cdone;

	modport master (
		output raddr, rstrobe, input rdata, rdone,
		output waddr, wdata, wstrobe, input wdone,
		output dcacheop, icacheop, cstrobe, input cdone
	);

	modport slave (
		input raddr, rstrobe, output rdata, rdone,
		input waddr, wdata, wstrobe, output wdone,
		input dcacheop, icacheop, cstrobe, output cdone
	);

endinterface
