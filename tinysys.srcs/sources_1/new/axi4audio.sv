`timescale 1ns / 1ps

import axi4pkg::*;

module axi4audio(
	input wire aclk,
	input wire aresetn,
	input wire audiobaseclock,	// TODO: This has to match the playback clock
	output wire dsleft,
	output wire dsright,
	axi4if.slave s_axi);

// ----------------------------------------------------------------------------
// PWM
// ----------------------------------------------------------------------------

module PWM(clk, PWM_in, PWM_out);
input clk;
input [15:0] PWM_in;
output PWM_out;

reg [16:0] PWM_accumulator;
always @(posedge clk) PWM_accumulator <= PWM_accumulator[15:0] + PWM_in;

assign PWM_out = PWM_accumulator[16];
endmodule

// ----------------------------------------------------------------------------
// Serial data output
// ----------------------------------------------------------------------------

wire outfifofull, outfifoempty, outfifovalid;
logic outfifowe = 1'b0, outfifore = 1'b0;
logic [31:0] outfifodin = 16'h00;
wire [31:0] outfifodout; // Top bit == CS

audiooutputfifo audiocmdoutfifo(
	.wr_clk(aclk),
	.full(outfifofull),
	.din(outfifodin),
	.wr_en(outfifowe),

	.rd_clk(audiobaseclock),
	.empty(outfifoempty),
	.dout(outfifodout),
	.rd_en(outfifore),
	.valid(outfifovalid),

	.rst(~aresetn) );

logic [15:0] dsleft_reg = 16'd0;
logic [15:0] dsright_reg = 16'd0;

PWM pwmleft(.clk(audiobaseclock), .PWM_in(dsleft_reg), .PWM_out(dsleft));
PWM pwmright(.clk(audiobaseclock), .PWM_in(dsright_reg), .PWM_out(dsright));

// Divide audio master clock by 16
logic divclk = 1'b0;
logic [3:0] divcounter;
always @(posedge audiobaseclock) begin
	divcounter <= divcounter + 4'd1;
	divclk <= divcounter[3];
end

// This has to clock x16 slower than PWM
always @(posedge divclk) begin
	if (~aresetn) begin
		//
	end else begin
		outfifore <= 1'b0;
		if ((~outfifoempty) && outfifovalid) begin
			dsleft_reg <= outfifodout[15:0];
			dsright_reg <= outfifodout[31:16];
			// Advance FIFO
			outfifore <= 1'b1;
		end
	end
end

// ----------------------------------------------------------------------------
// Main state machine
// ----------------------------------------------------------------------------

// No read
assign s_axi.rlast = 1'b0;
assign s_axi.arready = 1'b0;
assign s_axi.rvalid = 1'b0;
assign s_axi.rresp = 2'b00;
assign s_axi.rdata = 32'd0;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		s_axi.awready <= 1'b0;
		if (s_axi.awvalid) begin
			s_axi.awready <= 1'b1;
		end
	end
end

logic writestate = 1'b0;
always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.bresp = 2'b00;
	end else begin

		outfifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		outfifodin <= 32'h00;

		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && (~outfifofull)) begin
					// [31:16] -> rightsample, [15:0] -> leftsample
					outfifodin <= s_axi.wdata[31:0];
					outfifowe <= 1'b1; // (|s_axi.wstrb)
					s_axi.wready <= 1'b1;
					writestate <= 1'b1;
				end
			end
			1'b1: begin
				if(s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 1'b0;
				end
			end
		endcase
	end
end

endmodule
