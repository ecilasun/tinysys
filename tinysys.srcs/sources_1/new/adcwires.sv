interface adcwires (
	input wire [3:0] xa_p,
	input wire [3:0] xa_n);

	modport def (
		input xa_p, xa_n );

endinterface
