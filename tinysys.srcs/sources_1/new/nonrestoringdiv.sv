`timescale 1ns / 1ps

module nonrestoringdiv(
	input wire aclk,
	input wire aresetn,
	input wire start,
	input wire [31:0] dividend,
	input wire [31:0] divisor,
	output wire done,
	output wire [31:0] quotient,
	output wire [31:0] remainder );

// intermediate
logic [32:0] M;  // divisor
logic [32:0] Mn; // neg_divisor
logic [32:0] A;  // accumulator
logic [5:0] n;   // bit count

// output
logic [31:0] Q; // quotient
logic [31:0] R; // remainder

assign quotient = Q;
assign remainder = R;

// state machine
logic [1:0] state = 'd0;
logic complete = 1'b0;

assign done = complete;

always @(posedge aclk) begin
	if (~aresetn) begin
		
	end else begin
		complete <= 1'b0;
		case (state)
			'd0: begin
				if (start) begin
					n <= 'd32;
					M <= {1'b0, divisor};
					Mn <= {1'b1, ~divisor + 1};
					A <= 'd0;
					Q <= dividend;
				end
				state <= start ? 'd1 : 'd0;
			end
			'd1: begin
				{A, Q} <= {A[31:0], Q[31:0], 1'bz}; // shift A:Q left
				state <= 'd2;
			end
			'd2: begin
				case (A[32])
					1'b0: A <= A + Mn;
					1'b1: A <= A + M;
				endcase
				state <= 'd3;
			end
			'd3: begin
				Q[0] <= ~A[32];
				n <= n - 1;
				if (n == 'd1) begin
					//Q <= ... // quotient (already set)
					R <= A[32] ? A + M : A; // remainder 
					complete <= 1'b1; // we're done
					state <= 'd0;
				end else begin
					state <= 'd1;
				end
			end
		endcase
	end
end

endmodule
