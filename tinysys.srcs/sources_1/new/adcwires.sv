interface adcwires (
	output wire adclk,
	input wire addout,
	output wire addin,
	output wire adcs);

	modport def (
		input addout,
		output adclk, addin, adcs );

endinterface
