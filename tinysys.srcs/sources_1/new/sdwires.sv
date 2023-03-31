interface sdwires (
	input wire spi_miso,
	output wire spi_cs_n,
	output wire spi_clk,
	output wire spi_mosi,
	input wire spi_swtch);

	modport def (
		input spi_miso, spi_swtch,
		output spi_cs_n, spi_clk, spi_mosi );

endinterface
