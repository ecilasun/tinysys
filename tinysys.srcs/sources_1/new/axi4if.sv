`timescale 1ns / 1ps

import axi4pkg::*;

interface axi4if;

	// Read Address Channel
	addr_t araddr;
	logic arvalid;
	logic arready;
	len_t arlen;
	size_t arsize;
	burst_t arburst;

	// Read Data Channel
	data_t rdata;
	resp_t rresp;
	logic rvalid;
	logic rready;
	logic rlast;

	// Write Address Channel
	addr_t awaddr;
	logic awvalid;
	logic awready;
	len_t awlen;
	size_t awsize;
	burst_t awburst;

	// Write Data Channel
	data_t wdata;
	strb_t wstrb;
	logic wvalid;
	logic wready;
	logic wlast;

	// Write Response Channel
	resp_t bresp;
	logic bvalid;
	logic bready;


	modport master (
		output araddr, arvalid, input arready, output arlen, arsize, arburst,
		input rdata, rresp, rvalid, output rready, input rlast,
		output awaddr, awvalid, input awready, output awlen, awsize, awburst,
		output 	wdata, wstrb, wvalid, input wready, output wlast,
		input bresp, bvalid, output bready
	);

	modport slave (
		input araddr, arvalid, output arready, input arlen, arsize, arburst,
		output rdata, rresp, rvalid, input rready, output rlast,
		input awaddr, awvalid, output awready, input awlen, awsize, awburst,
		input 	wdata, wstrb, wvalid, output wready, input wlast,
		output bresp, bvalid, input bready
	);

endinterface
