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

	modport master (
		output raddr, rstrobe, input rdata, rdone,
		output waddr, wdata, wstrobe, input wdone
	);

	modport slave (
		input raddr, rstrobe, output rdata, rdone,
		input waddr, wdata, wstrobe, output wdone
	);

endinterface
