interface sdwires (
	input wire sd_miso,
	output wire sd_cs_n,
	output wire sd_clk,
	output wire sd_mosi );

	modport def (
		input sd_miso,
		output sd_cs_n, sd_clk, sd_mosi );

endinterface
