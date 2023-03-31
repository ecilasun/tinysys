interface gpuwires(
	// DVI
	output [2:0] tmdsp,
	output [2:0] tmdsn,
	output tmdsclkp,
	output tmdsclkn );

	modport def (
		output tmdsp,
		output tmdsn,
		output tmdsclkp,
		output tmdsclkn );

endinterface
