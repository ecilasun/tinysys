interface sdcardwires (
	input wire miso,
	output wire cs_n,
	output wire clk,
	output wire mosi,
	input wire swtch);

	modport def (
		input miso, swtch,
		output cs_n, clk, mosi );

endinterface

interface max3420wires (
	input wire miso,
	output wire cs_n,
	output wire clk,
	output wire mosi,
	output wire resn,
	input wire irq,
	input wire gpx);

	modport def (
		input miso, irq, gpx,
		output cs_n, clk, mosi, resn );

endinterface