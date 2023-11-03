interface sramwires (
	output reg [17:0] sram_addr,
	inout wire [15:0] sram_data_inout,
	output reg sram_oe,
	output reg sram_ce,
	output reg sram_we,
	output reg sram_ub,
	output reg sram_lb);

	modport def (
		output sram_addr, sram_oe, sram_ce, sram_we, sram_ub, sram_lb,
		inout sram_data_inout );

endinterface
