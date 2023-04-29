`timescale 1ns / 1ps

module debounce(
	input wire clk,
	input wire reset,
	input wire bouncy,
	output wire stable);

logic [2:0] debounced = 3'b000;
logic stabilized;

always @(posedge clk) begin
	if (reset == 1) begin
		debounced <= 3'b000;
	end else begin
		debounced <= {debounced[1:0], bouncy};
		if (debounced == 3'b000)
			stabilized <= 1'b0;
		else if (debounced == 3'b111)
			stabilized <= 1'b1;
	end
end

assign stable = stabilized;

endmodule
