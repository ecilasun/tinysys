`timescale 1ns / 1ps

interface debugbusif;

	logic [7:0] din;
	logic wen;
	logic [7:0] dout;
	logic drdy;

	modport master (
		output din, wen, input dout, drdy
	);

	modport slave (
		input din, wen, output dout, drdy
	);

endinterface
