interface sramwires (
	output reg [17:0] sram_addr,
	inout wire [15:0] sram_data_inout,
	output reg sram_oe,
	output reg sram_cen,
	output reg sram_we,
	output reg sram_ub,
	output reg sram_lb);

	modport def (
		output sram_addr, sram_oe, sram_cen, sram_we, sram_ub, sram_lb,
		input sram_data_inout );

endinterface
