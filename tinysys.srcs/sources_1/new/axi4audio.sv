`timescale 1ns / 1ps

module axi4audio (
	input wire aclk,
	input wire clk100,
	input wire aresetn,
    audiowires.def i2sconn,
    axi4if.slave s_axi);

// ------------------------------------------------------------------------------------
// Audio Init
// ------------------------------------------------------------------------------------

// What on earth is this? There's no I2C on board the 4344
/*wire initDone;		// I2C initialization is done
audio_init AudioI2CInit(
    .clk(clk100),
    .rst(~aresetn),
    .sda(sda),
    .scl(scl),
    .initDone(initDone) );*/

// ------------------------------------------------------------------------------------
// Audio buffer
// ------------------------------------------------------------------------------------

wire abfull, abempty, abvalid;
logic [31:0] abdin = 0; // left/right channel data (16 bits each)
logic abre = 1'b0;
logic abwe = 1'b0;
wire [31:0] abdout;

audiooutputfifo audiooutfifoinst(
	.full(abfull),
	.din(abdin),
	.wr_clk(aclk),
	.wr_en(abwe),
	.empty(abempty),
	.dout(abdout),
	.rd_clk(clk100),
	.rd_en(abre),
	.valid(abvalid),
	.rst(~aresetn) );

// ------------------------------------------------------------------------------------
// I2S Controller
// ------------------------------------------------------------------------------------

// 16 bit output data per channel
logic [15:0] leftout = 0;
logic [15:0] rightout = 0;

i2s_ctl I2SController(
	.CLK_I(clk100),
	.RST_I(~aresetn),
	.EN_TX_I(1'b1),
	.EN_RX_I(1'b0),		// No input
	.FS_I(4'b0101),
	// .MM_I(1'b0),
	.D_L_I(leftout),
	.D_R_I(rightout),
	.D_L_O(),
	.D_R_O(),
	.BCLK_O(i2sconn.sclk),
	.LRCLK_O(i2sconn.lrclk),
	.SDATA_O(i2sconn.sdin),
	.SDATA_I() );

logic lrclkD1 = 0;
logic lrclkD2 = 0;
logic [3:0] lrclkcnt = 4'h0;

always@(posedge clk100) begin
	// Edge detector
	lrclkD1 <= i2sconn.lrclk;
	lrclkD2 <= lrclkD1;

	// Stop pending reads from last clock
	abre <= 1'b0;

	// Load next sample
	if (lrclkcnt==8 && (~abempty) && abvalid)begin
		// TODO: If APU has its own burst read bus (as with GPU)
		// it can essentially use RAM instead of FIFO
		// That allows for playback by simply setting up a read
		// pointer, and sample length.
		leftout <= abdout[31:16];
		rightout <= abdout[15:0];
		// Advance FIFO
		abre <= 1'b1;
		lrclkcnt <= 0;
	end

	// ac_lrclk trigger high
	if (lrclkD1 & (~lrclkD2))
		lrclkcnt <= lrclkcnt + 4'd1;
end

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
		//abwe <= 1'b0;
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
