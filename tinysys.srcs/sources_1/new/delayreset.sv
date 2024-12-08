`timescale 1ns / 1ps
`default_nettype none

module delayreset(
	input wire aclk,
	input wire inputresetn,
	output wire delayedresetn );

// --------------------------------------------------
// Delayed reset
// Only to be used with negative reset signals
// --------------------------------------------------

logic subresetn;
logic resetstate;

always_ff @(posedge aclk) begin
	if (~inputresetn) begin
		resetstate <= 1'b0;
		subresetn <= 1'b0;
	end else begin
		unique case(resetstate)
			1'b0: begin
				subresetn <= 1'b0;
				resetstate <= 1'b1;
			end
			1'b1: begin
				subresetn <= 1'b1;
				resetstate <= 1'b1;
			end
		endcase
	end
end

assign delayedresetn = subresetn;

endmodule
