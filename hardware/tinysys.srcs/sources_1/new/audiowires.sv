interface audiowires (
	output sdin,
	output sclk,
	output lrclk,
	output mclk);

	modport def (
		output sdin, sclk, lrclk, mclk );

endinterface
