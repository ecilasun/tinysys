`timescale 1ns / 1ps
`default_nettype none

module debounce(
	input wire clk,
	input wire reset,
	input wire bouncy,
	output wire stable);

logic [7:0] debounced = 8'h00;
logic stabilized;

always @(posedge clk) begin

	debounced <= {debounced[6:0], bouncy};

	if (debounced == 8'h00)
		stabilized <= 1'b0;
	else if (debounced == 8'hFF)
		stabilized <= 1'b1;

	if (reset == 1) begin
		debounced <= 8'h00;
	end
end

assign stable = stabilized;

endmodule
