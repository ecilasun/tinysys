`timescale 1ns / 1ps

module axi4audio (
	input wire aclk,
	input wire clk12,
	input wire aresetn,
    audiowires.def i2sconn,
    axi4if.slave s_axi);

// ------------------------------------------------------------------------------------
// Audio buffer
// ------------------------------------------------------------------------------------

wire abfull, abempty, abvalid;
logic [31:0] abdin = 0; // left/right channel data (16 bits each)
wire abre;
logic abwe = 1'b0;
wire [31:0] abdout;

audiooutputfifo audiooutfifoinst(
	.full(abfull),
	.din(abdin),
	.wr_clk(aclk),
	.wr_en(abwe),
	.empty(abempty),
	.dout(abdout),
	.rd_clk(clk12),
	.rd_en(abre),
	.valid(abvalid),
	.rst(~aresetn) );

// ------------------------------------------------------------------------------------
// I2S Controller
// ------------------------------------------------------------------------------------

i2saudio i2saudioinst(
    .audioclock(clk12),	// 22.591MHz master clock

	.abempty(abempty),
	.abvalid(abvalid),
	.audiore(abre),
    .leftrightchannels(abdout),

    .tx_mclk(i2sconn.mclk),
    .tx_lrck(i2sconn.lrclk),
    .tx_sclk(i2sconn.sclk),
    .tx_sdout(i2sconn.sdin) );

// ------------------------------------------------------------------------------------
// Main state machine
// ------------------------------------------------------------------------------------

logic waddrstate = 1'b0;
logic writestate = 1'b0;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		// write address
		unique case (waddrstate)
			1'b0: begin
				if (s_axi.awvalid) begin
					s_axi.awready <= 1'b1;
					// TODO: Might want volume control or other effects at different s_axi.awaddr here
					waddrstate <= 1'b1;
				end
			end
			1'b1: begin
				s_axi.awready <= 1'b0;
				waddrstate <= 1'b0;
			end
		endcase
	end
end

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.bresp <= 2'b00; // okay
		s_axi.bvalid <= 1'b0;
		s_axi.wready <= 1'b0;
	end else begin
		// write data
		abwe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && (~abfull)) begin
					abdin <= s_axi.wdata[31:0];
					abwe <= 1'b1;
					writestate <= 1'b1;
					s_axi.wready <= 1'b1;
				end
			end
			1'b1: begin
				if (s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 1'b0;
				end
			end
		endcase
	end
end

// Can't read from the APU just yet
assign s_axi.rlast = 1'b1;
assign s_axi.arready = 1'b1;
assign s_axi.rvalid = 1'b1;
assign s_axi.rresp = 2'b00;
assign s_axi.rdata = 0;

endmodule
