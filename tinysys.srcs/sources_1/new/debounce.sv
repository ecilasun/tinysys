`timescale 1ns / 1ps

module debounce(
	input wire clk,
	input wire reset,
	input wire bouncy,
	output wire stable);

logic [7:0] debounced = 8'h00;
logic stabilized;

always @(posedge clk) begin
	if (reset == 1) begin
		debounced <= 8'h00;
	end else begin
		debounced <= {debounced[6:0], bouncy};
		if (debounced == 8'h00)
			stabilized <= 1'b0;
		else if (debounced == 8'hFF)
			stabilized <= 1'b1;
	end
end

assign stable = stabilized;

endmodule
