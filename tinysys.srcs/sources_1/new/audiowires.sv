interface audiowires (
	output cs_n,
	output sclk,
	output sdin);

	modport def (
		output cs_n, sclk, sdin );

endinterface
