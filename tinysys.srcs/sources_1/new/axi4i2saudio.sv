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
logic [8:0] count = 9'd0;

always @(posedge audioclock)
	count <= count + 1;

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
	WCMD, DISPATCH,
	APUSTART,
	APUBUFFERSIZE,
	APUSTOP,
	APUSWAP,
	APUSETRATE,
	STARTDMA, WAITREADADDR, READLOOP,
	FINALIZE } apucmdmodetype;
apucmdmodetype cmdmode = WCMD;

logic [31:0] apucmd;					// Command code
logic [31:0] apusourceaddr;				// Memory address to DMA the audio samples from
logic [9:0] apuwordcount = 10'd1023;	// Number of words to playback, minus one
logic [7:0] apuburstcount = 8'd255;		// Number of 16byte DMA reads, minus one

// Internal sample buffers with up to 1K 32 bit(L/R) samples each (2x4096 bytes)
// This means each buffer has: 1024 stereo samples max, 256 bursts to read max
// e.g. for 512x16bit stereo samples  we have 512 pairs to read, 128 bursts to make
// Read and Write regions always alternate between offset 0 and offset 256
logic [127:0] samplebuffer[0:511];

// Buffer address high bit to control DMA write page
logic writeBufferSelect = 1'b0;

// Number of buffer swaps so far (CDC from audio clock to bus clock)
logic [31:0] bufferSwapCount = 32'd0;
(* async_reg = "true" *) logic [31:0] bufferSwapCountCDC1 = 32'd0;
(* async_reg = "true" *) logic [31:0] bufferSwapCountCDC2 = 32'd0;

// CPU can access this
assign swapcount = bufferSwapCountCDC2;

initial begin
	for(int i=0;i<512;i++) begin
		samplebuffer[i] = 128'd0;
	end
end

logic [7:0] burstcursor = 8'd0;
logic [1:0] sampleoutputrate = 2'b00;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		//
	end else begin
		bufferSwapCountCDC1 <= bufferSwapCount;
		bufferSwapCountCDC2 <= bufferSwapCountCDC1;
	end
end

logic [9:0] readcursor = 10'd0;			// Current sample read position
logic [9:0] readLine;					// Cache line select for reads
logic [9:0] writeLine;					// Cache line select for writes
always_comb begin
	readLine = {~writeBufferSelect, readcursor[9:2]};
	writeLine = {writeBufferSelect, burstcursor};
end

assign m_axi.arsize = SIZE_16_BYTE;
assign m_axi.arburst = BURST_INCR;

assign m_axi.awsize = SIZE_4_BYTE;
assign m_axi.awburst = BURST_FIXED;

assign m_axi.awaddr = 0;
assign m_axi.awlen = 0;
assign m_axi.wdata = 0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		cmdmode <= WCMD;
	end else begin

		re <= 1'b0;
		//opl2fifowe <= 1'b0;

		case (cmdmode)
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
				// TODO: zero out the current playback buffer or stop output altogether
				cmdmode <= FINALIZE;
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
				burstcursor <= 8'd0;
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
					samplebuffer[writeLine] <= m_axi.rdata;
					burstcursor <= burstcursor + 8'd1;
					cmdmode <= ~m_axi.rlast ? READLOOP : FINALIZE;
				end
			end

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase

	end
end

// ------------------------------------------------------------------------------------
// I2S output
// ------------------------------------------------------------------------------------

logic [1:0] sampleRateCounter = 2'b00;	// Sample rate counter

always@(posedge audioclock) begin

 	// Trigger new sample copy just before we select L channel again out of the LR pair
	if (count==9'h0ff) begin

		// readBufferSelect == ~writeBufferSelect
		unique case (readcursor[1:0])
			2'b00: tx_data_lr <= samplebuffer[readLine][31 : 0];
			2'b01: tx_data_lr <= samplebuffer[readLine][63 : 32];
			2'b10: tx_data_lr <= samplebuffer[readLine][95 : 64];
			2'b11: tx_data_lr <= samplebuffer[readLine][127: 96];
		endcase

		// Next sample rewinds if we're at the end
		// Also increment buffer swap count to notify CPU
		if (readcursor == apuwordcount) begin
			readcursor <= 0;
			bufferSwapCount <= bufferSwapCount + 32'd1;
		end else begin

		// Increment based on sample rate (or sometimes not)
		unique case (1'b1)
			sampleoutputrate[0]:	readcursor <= readcursor + (sampleRateCounter[0] ? 10'd1 : 10'd0);
			sampleoutputrate[1]:	readcursor <= readcursor + (sampleRateCounter[1] ? 10'd1 : 10'd0);
			default:				readcursor <= readcursor + 10'd1;
		endcase

		sampleRateCounter <= sampleRateCounter + 2'd1;

		end
	end
end

logic [23:0] tx_data_l_shift = 24'b0;
logic [23:0] tx_data_r_shift = 24'b0;

always@(posedge audioclock)
	if (count == 3'b00000111) begin
		// TODO: Implement a proper mixer here
		tx_data_l_shift <= {tx_data_lr[31:16] + mixinput, 8'd0};
		tx_data_r_shift <= {tx_data_lr[15:0] + mixinput, 8'd0};
		//tx_data_l_shift <= {tx_data_lr[31:16], 8'd0};
		//tx_data_r_shift <= {tx_data_lr[15:0], 8'd0};
	end else if (count[2:0] == 3'b111 && count[7:3] >= 5'd1 && count[7:3] <= 5'd24) begin
		if (count[8] == 1'b1)
			tx_data_r_shift <= {tx_data_r_shift[22:0], 1'b0};
		else
			tx_data_l_shift <= {tx_data_l_shift[22:0], 1'b0};
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