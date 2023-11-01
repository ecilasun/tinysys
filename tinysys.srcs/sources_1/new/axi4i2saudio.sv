`timescale 1ns / 1ps

module axi4i2saudio(
	input wire aclk,
	input wire aresetn,
    input wire audioclock,				// 22.591MHz master clock
	axi4if.master m_axi,				// Direct memory access for burst reads
	input wire abempty,					// Command fifo empty
	input wire abvalid,					// Command available
	output wire audiore,				// Command read control
    input wire [31:0] audiodin,			// APU command input
	output wire [31:0] swapcount,		// Buffer swap counter for sync
	input wire [15:0] mixinput,			// Input from external audio source
    output wire tx_mclk,				// Audio bus output
    output wire tx_lrck,				// L/R select
    output wire tx_sclk,				// Stream clock
    output logic tx_sdout );			// Stream out

// ------------------------------------------------------------------------------------
// Clock divider
// ------------------------------------------------------------------------------------

// Counter for generating other divided clocks
logic [8:0] count;

COUNTER_LOAD_MACRO #(
	.COUNT_BY(48'd1),		// Count by 1
	.DEVICE("7SERIES"), 
	.WIDTH_DATA(9) ) counterinst (
	.Q(count),
	.CLK(audioclock),
	.CE(aresetn),
	.DIRECTION(1'b1),
	.LOAD(~aresetn),
	.LOAD_DATA(9'd0),
	.RST(1'b0) );

wire lrck = count[8];
wire sclk = count[2];
assign tx_lrck = lrck;			// Channel select 44.1KHz (/512)
assign tx_sclk = sclk;			// Sample clock 2.823MHz (/8)
assign tx_mclk = audioclock;	// Master clock 22.519MHz

// Internal L/R copies to stream out
logic [31:0] tx_data_lr = 0;
logic re = 1'b0;
assign audiore = re;

// ------------------------------------------------------------------------------------
// Command dispatch
// ------------------------------------------------------------------------------------

typedef enum logic [3:0] {
	INIT,
	WCMD, DISPATCH,
	APUSTART,
	APUBUFFERSIZE,
	APUSTOP, APUCLEARLOOP,
	APUSWAP,
	APUSETRATE,
	STARTDMA, WAITREADADDR, READLOOP,
	FINALIZE } apucmdmodetype;
apucmdmodetype cmdmode = INIT;

logic [31:0] apucmd;					// Command code
logic [31:0] apusourceaddr;				// Memory address to DMA the audio samples from
logic [9:0] apuwordcount = 10'd1023;	// Number of words to playback, minus one
logic [7:0] apuburstcount = 8'd255;		// Number of 16byte DMA reads, minus one

// Buffer address high bit to control DMA write page
logic writeBufferSelect = 1'b0;

// Internal sample buffers with up to 1K 32 bit(L/R) samples each (2x4096 bytes)
// This means each buffer has: 1024 stereo samples max, 256 bursts to read max
// e.g. for 512x16bit stereo samples  we have 512 pairs to read, 128 bursts to make
// Read and Write regions always alternate between offset 0 and offset 256
logic samplewe = 1'b0;
logic [7:0] writeCursor = 8'd0; 
logic [9:0] readCursor = 10'd0;
logic [127:0] sampleIn;
wire [31:0] sampleOut;
samplemem samplememinst (
  .clka(aclk),
  .wea(samplewe),
  .addra({writeBufferSelect, writeCursor}),
  .dina(sampleIn),
  .clkb(audioclock),
  .addrb({~writeBufferSelect, readCursor}),
  .doutb(sampleOut) );

// Number of buffer swaps so far (CDC from audio clock to bus clock)
logic bufferSwap;
(* async_reg = "true" *) logic bufferSwapCDC1 = 32'd0;
(* async_reg = "true" *) logic bufferSwapCDC2 = 32'd0;

// CPU can access this
assign swapcount = {31'd0, bufferSwap};

logic [1:0] sampleoutputrate = 2'b00;

always_ff @(posedge aclk) begin
	bufferSwapCDC1 <= bufferSwap;
	bufferSwapCDC2 <= bufferSwapCDC1;
end

assign m_axi.arsize = SIZE_16_BYTE;
assign m_axi.arburst = BURST_INCR;

assign m_axi.awsize = SIZE_4_BYTE;
assign m_axi.awburst = BURST_FIXED;

assign m_axi.awaddr = 0;
assign m_axi.awlen = 0;
assign m_axi.wdata = 0;

always_ff @(posedge aclk) begin

	re <= 1'b0;
	samplewe <= 1'b0;

	case (cmdmode)
		INIT: begin
			m_axi.awvalid <= 0;
			m_axi.wvalid <= 0;
			m_axi.wstrb <= 16'h0000;
			m_axi.wlast <= 0;
			m_axi.bready <= 0;
			m_axi.arvalid <= 0;
			m_axi.rready <= 0;
			writeCursor <= 8'd0;
			cmdmode <= WCMD;
		end

		WCMD: begin
			if (abvalid && ~abempty) begin
				apucmd <= audiodin;
				// Advance FIFO
				re <= 1'b1;
				// Dispatch cmd
				cmdmode <= DISPATCH;
			end
		end

		DISPATCH: begin
			case (apucmd)
				32'h00000000:	cmdmode <= APUBUFFERSIZE;	// Set up size of DMA copies and playback range, in words
				32'h00000001:	cmdmode <= APUSTART;		// Start DMA into write page
				32'h00000002:	cmdmode <= APUSTOP;			// TODO: Stop all sound output
				32'h00000003:	cmdmode <= APUSWAP;			// Swap r/w pages
				32'h00000004:	cmdmode <= APUSETRATE;		// TODO: Set sample duplication count to x1 (44.1KHz), x2(22.05KHz) or x4(11.025KHz)
				default:		cmdmode <= FINALIZE;		// Invalid command, wait one clock and try next
			endcase
		end

		APUSTART: begin
			if (abvalid && ~abempty) begin
				apusourceaddr <= audiodin;
				// Advance FIFO
				re <= 1'b1;
				cmdmode <= STARTDMA;
			end
		end

		APUBUFFERSIZE: begin
			if (abvalid && ~abempty) begin
				apuwordcount <= audiodin[9:0];		// wordcount-1 (0..1023), typically 1023
				apuburstcount <= audiodin[10:2];	// burstcount = wordcount>>2, typically 255
				// Advance FIFO
				re <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		APUSTOP: begin
			writeCursor <= 8'hFF;
			cmdmode <= APUCLEARLOOP;
		end
		
		APUCLEARLOOP: begin
			samplewe <= 1'b1;
			sampleIn <= 0;
			writeCursor <= writeCursor + 8'd1;
			cmdmode <= (writeCursor == 8'hFE) ? FINALIZE : APUCLEARLOOP;
		end

		APUSWAP: begin
			// Swap read/write buffers if we're at the end of a playback buffer
			writeBufferSelect <= ~writeBufferSelect;
			cmdmode <= FINALIZE;
		end

		APUSETRATE: begin
			if (abvalid && ~abempty) begin
				sampleoutputrate <= audiodin[1:0]; // 2'b00:x1, 2'b01:x2, 2'b10:x4, 2'b11:undefined
				// Advance FIFO
				re <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		STARTDMA: begin
			writeCursor <= 8'hFF;
			m_axi.arlen <= apuburstcount;
			m_axi.arvalid <= 1;
			m_axi.araddr <= apusourceaddr; 
			cmdmode <= WAITREADADDR;
		end

		WAITREADADDR: begin
			if (m_axi.arready) begin
				m_axi.arvalid <= 0;
				m_axi.rready <= 1;
				cmdmode <= READLOOP;
			end
		end

		READLOOP: begin
			if (m_axi.rvalid) begin
				m_axi.rready <= ~m_axi.rlast;
				sampleIn <= m_axi.rdata;
				samplewe <= 1'b1;
				writeCursor <= writeCursor + 8'd1;
				cmdmode <= ~m_axi.rlast ? READLOOP : FINALIZE;
			end
		end
		
		FINALIZE: begin
			cmdmode <= WCMD;
		end

	endcase

	if (~aresetn) begin
		cmdmode <= INIT;
	end
end

// ------------------------------------------------------------------------------------
// I2S output
// ------------------------------------------------------------------------------------

logic [1:0] sampleRateCounter = 2'b00;	// Sample rate counter
logic [1:0] audiooutstate = 2'b00;
always@(posedge audioclock) begin

	case (audiooutstate)
		2'b00: begin
			readCursor <= 10'd0;
			bufferSwap <= 32'd0;
			sampleRateCounter <= 2'd0;
			audiooutstate <= 2'b01;
		end
		2'b01: begin
			// New sample
			if (count==9'h0ff) begin
				// Update output
				tx_data_lr <= sampleOut;
				if (readCursor == apuwordcount) begin
					// Increment swap count at end of buffer
					bufferSwap <= ~bufferSwap;
				end
				// Increment read cursor based on sample rate, or rewind at end of buffer
				unique case (1'b1)
					sampleoutputrate[0]:		readCursor <= readCursor + (sampleRateCounter[0] ? 10'd1 : 10'd0);
					sampleoutputrate[1]:		readCursor <= readCursor + (sampleRateCounter[1] ? 10'd1 : 10'd0);
					readCursor == apuwordcount:	readCursor <= 0;
					default:					readCursor <= readCursor + 10'd1;
				endcase
				sampleRateCounter <= sampleRateCounter + 2'd1;
			end
		end
	endcase

	if (~aresetn) begin
		audiooutstate <= 2'b00;
	end
end

logic [23:0] tx_data_l_shift = 24'b0;
logic [23:0] tx_data_r_shift = 24'b0;
logic [23:0] leftmix = 24'b0;
logic [23:0] rightmix = 24'b0;

always@(posedge audioclock) begin
	if (count == 3'b00000111) begin
		// TODO: Move the mixer to an external device
		// TODO: (A+B) << volume2
		// NOTE: OPL2 input is mono
		leftmix <= {tx_data_lr[31:16] + mixinput, 8'd0};
		rightmix <= {tx_data_lr[15:0] + mixinput, 8'd0};
		tx_data_l_shift <= leftmix;
		tx_data_r_shift <= rightmix;
	end else if (count[2:0] == 3'b111 && count[7:3] >= 5'd1 && count[7:3] <= 5'd24) begin
		if (count[8] == 1'b1)
			tx_data_r_shift <= {tx_data_r_shift[22:0], 1'b0};
		else
			tx_data_l_shift <= {tx_data_l_shift[22:0], 1'b0};
	end

	if (~aresetn) begin
		tx_data_r_shift <= 24'd0;
		tx_data_l_shift <= 24'd0;
	end
end

always@(count, tx_data_l_shift, tx_data_r_shift) begin
	if (count[7:3] <= 5'd24 && count[7:3] >= 4'd1)
		if (count[8] == 1'b1)
			tx_sdout = tx_data_r_shift[23];
		else
			tx_sdout = tx_data_l_shift[23];
	else
		tx_sdout = 1'b0;
end

endmodule
