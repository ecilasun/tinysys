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
    output wire tx_mclk,				// Audio bus output
    output wire tx_lrck,				// L/R select
    output wire tx_sclk,				// Stream clock
    output logic tx_sdout );			// Stream out

// ------------------------------------------------------------------------------------
// Clock divider
// ------------------------------------------------------------------------------------

// Counter for generating other divided clocks
logic [8:0] count;
logic counterenabled;

COUNTER_LOAD_MACRO #(
	.COUNT_BY(48'd1),		// Count by 1
	.DEVICE("7SERIES"), 
	.WIDTH_DATA(9) ) counterinst (
	.Q(count),
	.CLK(audioclock),
	.CE(counterenabled),
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
logic [31:0] tx_data_lr;
logic re;
assign audiore = re;

// ------------------------------------------------------------------------------------
// Command dispatch
// ------------------------------------------------------------------------------------

typedef enum logic [3:0] {
	INIT,
	WCMD, DISPATCH,
	APUSTART,
	APUBUFFERSIZE,
	APUSETRATE,
	STARTDMA, WAITREADADDR, READLOOP,
	FINALIZE } apucmdmodetype;
apucmdmodetype cmdmode = INIT;

logic [31:0] apucmd;			// Command code
logic [31:0] apusourceaddr;		// Memory address to DMA the audio samples from
logic [9:0] apuwordcount;		// Number of words to playback, minus one
logic [7:0] apuburstcount;		// Number of 16byte DMA reads, minus one

// Buffer address high bit to control DMA write page
logic writeBufferSelect;

// Internal sample buffers with up to 1K 32 bit(L/R) samples each (2x4096 bytes)
// This means each buffer has: 1024 stereo samples max, 256 bursts to read max
// e.g. for 512x16bit stereo samples  we have 512 pairs to read, 128 bursts to make
// Read and Write regions always alternate between offset 0 and offset 256
logic samplewe;
logic [7:0] writeCursor;
logic [9:0] readCursor;
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
(* async_reg = "true" *) logic bufferSwapCDC1;
(* async_reg = "true" *) logic bufferSwapCDC2;

logic [1:0] sampleoutputrate;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		bufferSwapCDC1 <= 1'b0;
		bufferSwapCDC2 <= 1'b0;
	end else begin
		bufferSwapCDC1 <= bufferSwap;
		bufferSwapCDC2 <= bufferSwapCDC1;
	end
end

// CPU can access this
assign swapcount = {31'd0, bufferSwapCDC2};

assign m_axi.arsize = SIZE_16_BYTE;
assign m_axi.arburst = BURST_INCR;

assign m_axi.awsize = SIZE_4_BYTE;
assign m_axi.awburst = BURST_FIXED;

assign m_axi.awaddr = 0;
assign m_axi.awlen = 0;
assign m_axi.wdata = 0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		re <= 1'b0;
		samplewe <= 1'b0;
		counterenabled <= 1'b0;
		writeCursor <= 8'd0;
		sampleoutputrate <= 2'b00;
		apuwordcount <= 10'd1023;
		apuburstcount <= 8'd255;
		cmdmode <= INIT;
	end else begin
		re <= 1'b0;
		samplewe <= 1'b0;
	
		case (cmdmode)
			INIT: begin
				counterenabled <= 1'b1;
				m_axi.awvalid <= 0;
				m_axi.wvalid <= 0;
				m_axi.wstrb <= 16'h0000;
				m_axi.wlast <= 0;
				m_axi.bready <= 0;
				m_axi.arvalid <= 0;
				m_axi.rready <= 0;
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
					//32'h00000002:	cmdmode <= UNUSED;			// Spare command, unused
					//32'h00000003:	cmdmode <= UNUSED;			// Spare command, unused
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
	
			APUSETRATE: begin
				if (abvalid && ~abempty) begin
					sampleoutputrate <= audiodin[1:0]; // 2'b00:x1, 2'b01:x2, 2'b10:x4, 2'b11:quiet
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
	
	end
end

// ------------------------------------------------------------------------------------
// I2S output
// ------------------------------------------------------------------------------------

logic [1:0] sampleRateCounter;	// Sample rate counter

always@(posedge audioclock) begin
	if (~aresetn) begin
		tx_data_lr <= 0;
		readCursor <= 10'd0;
		bufferSwap <= 1'd0;
		writeBufferSelect <= 1'b0;
		sampleRateCounter <= 2'd0;
	end else begin
		if (count==9'h0ff) begin
			// New sample
			tx_data_lr <= (sampleoutputrate==2'b11) ? 0 : sampleOut;

			// Increment read cursor based on sample rate, or rewind at end of buffer
			// We always run at 44KHz but duplicate samples to emulate 22KHz or 11KHz
			unique case (sampleoutputrate)
				2'b00:		readCursor <= readCursor + 10'd1;
				2'b01:		readCursor <= readCursor + {9'd0, sampleRateCounter[0]};
				2'b10:		readCursor <= readCursor + {9'd0, sampleRateCounter[1]};
				2'b11:		readCursor <= 0;	// Halt output
			endcase

			// Increment swap count at end of buffer
			if (readCursor == apuwordcount) begin
				// Switch plaback buffer and also swap CPU side r/w page ID
				bufferSwap <= ~bufferSwap;
				writeBufferSelect <= ~writeBufferSelect;
				readCursor <= 0;
			end

			sampleRateCounter <= sampleRateCounter + 2'd1;
		end
	end
end

logic [23:0] tx_data_l_shift;
logic [23:0] tx_data_r_shift;

always@(posedge audioclock) begin
	if (~aresetn) begin
		tx_data_r_shift <= 24'd0;
		tx_data_l_shift <= 24'd0;
	end else begin
		if (count == 3'b00000111) begin
			tx_data_l_shift <= {tx_data_lr[31:16], 8'd0};
			tx_data_r_shift <= {tx_data_lr[15:0], 8'd0};
		end else if (count[2:0] == 3'b111 && count[7:3] >= 5'd1 && count[7:3] <= 5'd24) begin
			if (count[8] == 1'b1)
				tx_data_r_shift <= {tx_data_r_shift[22:0], 1'b0};
			else
				tx_data_l_shift <= {tx_data_l_shift[22:0], 1'b0};
		end
	end
end

always@(count, tx_data_l_shift, tx_data_r_shift, aresetn) begin
	if (~aresetn) begin
		tx_sdout = 1'b0;
	end else begin
		if (count[7:3] <= 5'd24 && count[7:3] >= 4'd1)
			if (count[8] == 1'b1)
				tx_sdout = tx_data_r_shift[23];
			else
				tx_sdout = tx_data_l_shift[23];
		else
			tx_sdout = 1'b0;
	end
end

endmodule
