`timescale 1ns / 1ps

import axi4pkg::*;

module axi4audio(
	input wire aclk,
	input wire aresetn,
	input wire audiobaseclock,
    audiowires.def audioconn,
	axi4if.slave s_axi);
	
// ----------------------------------------------------------------------------
// Serial data output
// ----------------------------------------------------------------------------

wire outfifofull, outfifoempty, outfifovalid;
logic outfifowe = 1'b0, outfifore = 1'b0;
logic [15:0] outfifodin = 16'h00;
wire [15:0] outfifodout; // Top bit == CS

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

logic cs_n = 1'b1;
logic sclk = 1'b0;
logic sdin = 1'b0;

assign audioconn.cs_n = cs_n;
assign audioconn.sclk = sclk;
assign audioconn.sdin = sdin;

logic [15:0] senddata = 16'd0;
logic [1:0] sendstate = 2'b00;
logic [3:0] sendbitcount = 4'd0;
always @(posedge audiobaseclock) begin
	if (~aresetn) begin
		// Start with CS high
		cs_n <= 1'b1;
		sclk <= 1'b0;
	end else begin
		outfifore <= 1'b0;
		sclk <= ~sclk;
		case (sendstate)
			2'b00: begin
				if ((~outfifoempty) && outfifovalid) begin
					senddata <= outfifodout;
					sendstate <= 2'b01;
					// Advance FIFO
					outfifore <= 1'b1;
				end
			end
			2'b01: begin
				// Wait for low clock and pull CS down
				if (sclk == 1'b0) begin
					cs_n <= 1'b0;
					// Start sending
					sendbitcount <= 4'b1111;
					sendstate <= 2'b10;
				end
			end
			2'b10: begin
				// Send command bits when clock is low
				if (sclk == 1'b0) begin
					if (sendbitcount == 4'b0000) begin
						cs_n <= 1'b1;					
						sendstate <= 2'b00;
					end
					sdin <= senddata[15];
					senddata <= {senddata[14:0], 1'b0};
					sendbitcount <= sendbitcount - 4'd1;
				end
			end
		endcase
	end
end

// ----------------------------------------------------------------------------
// main state machine
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
		outfifodin <= 16'h00;

		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && (~outfifofull)) begin
					outfifodin <= s_axi.wdata[15:0];
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
