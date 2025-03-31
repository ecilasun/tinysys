`timescale 1ns / 1ps
`default_nettype none

//
// f2ui4sat
//
// Converts a fp32 value in range [0.0, 1.0] to an unsigned integer in range [0, 15].
// Values outside the range are clamped to the limits.
//
// C equivalent: max(0, min(15, (int)(16.0f * value)))
//
module f2ui4sat(
    input wire clk,
    input wire aresetn,
    input wire [31:0] value,
    input wire start,
    output wire [31:0] result,
    output wire ready
);

logic [31:0] saturated;
logic [7:0] shift;
logic [1:0] done;

assign result = saturated;
assign ready = done == 2'b10 ? 1'b1 : 1'b0;

wire s = value[31];
wire [7:0] e = value[30:23];
wire [22:0] m = value[22:0];

always @(posedge clk) begin
	if (~aresetn) begin
		done <= 2'b00;
		saturated <= 32'd0;
		shift <= 8'd0;
	end else begin
		if (start) begin
			done <= 2'b01;
		end else begin
			done <= {done[0], 1'b0};
			if (s == 1'b1 || e < 8'd115) begin // Negative or exponents that will overflow shift go directly to zero
				saturated <= 0;
			end else if (e > 8'd126) begin // Values greater than 1.0 go directly to saturate
				saturated <= 15;
			end else begin // All other values follow adjusted f2i shift.
				// NOTE: This is pipelined to complete in 2 clocks
				shift <= {(8'd146) - e}; // (8'd127 + 8'd23 - 8'd4) - e;
				saturated <= ((m | 32'h800000) >> shift[5:0]) & 4'hF;
			end
		end
	end
end

endmodule
