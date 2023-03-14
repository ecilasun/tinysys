`timescale 1ns / 1ps

`include "shared.vh"

module integermultiplier(
    input wire aclk,				// clock input
    input wire aresetn,				// reset line
    input wire start,				// kick multiply operation (hold for one clock)
    input wire [2:0] func3,			// to determine which mul op this is
    input wire [31:0] multiplicand,	// input a
    input wire [31:0] multiplier,	// input b
    output logic [31:0] product,	// result
    output wire ready );	

logic busy = 1'b0;
logic busy2 = 1'b0;
logic [32:0] a = 33'd0;
logic [32:0] b = 33'd0;
logic [3:0] n = 4'd0;
wire [65:0] dspproduct;
assign ready = ~busy&busy2;

mult_gen_0 signextendedmultiplier(
	.CLK(aclk),
	.A(a),
	.B(b),
	.P(dspproduct),
	.CE(aresetn & (start | busy)) );

always_ff @(posedge aclk) begin
	if (~aresetn) begin
//		busy <= 1'b0;
//		busy2 <= 1'b0;
	end else begin
		busy2 <= busy;
		if (start) begin
			unique case (func3)
				`F3_MUL, `F3_MULH: begin
					a <= {multiplicand[31], multiplicand};
					b <= {multiplier[31], multiplier};
				end
				`F3_MULHSU: begin
					a <= {multiplicand[31], multiplicand};
					b <= {1'b0, multiplier};
				end
				`F3_MULHU: begin
					a <= {1'b0, multiplicand};
					b <= {1'b0, multiplier};
				end
			endcase
			n <= 5; // Match this to the latency of the DSP multiplier
			busy <= 1'b1;
		end else begin
			if (busy) begin
				if (n == 0) begin
					unique case (func3)
						`F3_MUL: begin
							product <= dspproduct[31:0];
						end
						default : begin // `f3_mulh, `f3_mulhsu, `f3_mulhu
							product <= dspproduct[63:32]; // or is this 64:33 ?
						end
					endcase
					busy <= 1'b0;
				end else begin
					n <= n - 4'd1;
				end 
			end else begin
				product <= 32'd0;
			end
		end
	end
end

endmodule
