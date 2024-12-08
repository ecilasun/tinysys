`timescale 1ns / 1ps
`default_nettype none

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
logic [3:0] count = 4'd0;
wire [64:0] dspproduct;
logic done = 1'b0;
assign ready = done;

typedef enum logic [1:0] {INIT, WCMD, MULLOOP} mulcmdmodetype;
mulcmdmodetype cmdmode = INIT;

logic mulce;
mult_gen_0 signextendedmultiplier(
	.CLK(aclk),
	.A(a),
	.B(b),
	.P(dspproduct),
	.CE(mulce) );

always_ff @(posedge aclk) begin

    done <= 1'b0;

    case (cmdmode)
        INIT: begin
            mulce <= 1'b0;
            product <= 32'd0;
            cmdmode <= WCMD;
        end

        WCMD: begin
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
				default: begin
					a <= 33'dz;
					b <= 33'dz;
				end
            endcase
            count <= 5; // Match this to the latency of the DSP multiplier IP
            mulce <= start;
            cmdmode <= start ? MULLOOP : WCMD;
        end

        MULLOOP: begin
			if (count == 0) begin
				unique case (func3)
					`F3_MUL: begin
						product <= dspproduct[31:0];
					end
					default : begin // `f3_mulh, `f3_mulhsu, `f3_mulhu
						product <= dspproduct[63:32];
					end
				endcase
				mulce <= 1'b0;
                done <= 1'b1;
    			cmdmode <= WCMD;
			end else begin
				count <= count - 4'd1;
    			cmdmode <= MULLOOP;
			end
        end
    endcase

	if (~aresetn) begin
		cmdmode <= INIT;
	end
end

endmodule
