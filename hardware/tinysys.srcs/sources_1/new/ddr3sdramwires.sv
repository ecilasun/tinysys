interface ddr3sdramwires (
		output wire ddr3_reset_n,
		output wire [0:0] ddr3_cke,
		output wire [0:0] ddr3_ck_p, 
		output wire [0:0] ddr3_ck_n,
		output wire ddr3_ras_n, 
		output wire ddr3_cas_n, 
		output wire ddr3_we_n,
		output wire [2:0] ddr3_ba,
		output wire [13:0] ddr3_addr,
		output wire [0:0] ddr3_odt,
		output wire [1:0] ddr3_dm,
		inout wire [1:0] ddr3_dqs_p,
		inout wire [1:0] ddr3_dqs_n,
		inout wire [15:0] ddr3_dq,
		output wire init_calib_complete );

	modport def (
		output ddr3_reset_n,
		output ddr3_cke,
		output ddr3_ck_p, 
		output ddr3_ck_n,
		output ddr3_ras_n, 
		output ddr3_cas_n, 
		output ddr3_we_n,
		output ddr3_ba,
		output ddr3_addr,
		output ddr3_odt,
		output ddr3_dm,
		inout ddr3_dqs_p,
		inout ddr3_dqs_n,
		inout ddr3_dq,
		output init_calib_complete );

endinterface
