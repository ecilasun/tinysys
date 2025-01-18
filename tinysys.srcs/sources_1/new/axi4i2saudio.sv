`timescale 1ns / 1ps
`default_nettype none

module axi4i2saudio(
	input wire aclk,
	input wire aresetn,
	input wire rstaudion,
    input wire audioclock,				// 22.591MHz master clock
	axi4if.master m_axi,				// Direct memory access for burst reads
	input wire abempty,					// Command fifo empty
	input wire abvalid,					// Command available
	output wire audiore,				// Command read control
    input wire [31:0] audiodin,			// APU command input
	output wire [31:0] swapcount,		// Buffer swap counter for sync
	// Output
	output wire audiosampleclk,			// Audio clock
    output wire [31:0] tx_sdout );		// Stream out

// ------------------------------------------------------------------------------------
// Clock divider
// ------------------------------------------------------------------------------------

// Counter for generating other divided clocks
bit [8:0] count;
bit counterenabled;

COUNTER_LOAD_MACRO #(
	.COUNT_BY(48'd1),		// Count by 1
	.DEVICE("7SERIES"), 
	.WIDTH_DATA(9) ) counterinst (
	.Q(count),
	.CLK(audioclock),
	.CE(counterenabled),
	.DIRECTION(1'b1),
	.LOAD(~rstaudion),
	.LOAD_DATA(9'd0),
	.RST(1'b0) );

// 44.1KHz output clock for audio
assign audiosampleclk = count[8];

//wire lrck = count[8];
//wire sclk = count[2];
//assign tx_lrck = lrck;		// Channel select 44.1KHz (/512)
//assign tx_sclk = sclk;		// Sample clock 2.823MHz (/8)
//assign tx_mclk = audioclock;	// Master clock 22.519MHz

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// ------------------------------------------------------------------------------------
// Command dispatch
// ------------------------------------------------------------------------------------

typedef enum bit [3:0] {
	INIT,
	WCMD, DISPATCH,
	APUSTART,
	APUBUFFERSIZE,
	APUSETRATE,
	STARTDMA, WAITREADADDR, READLOOP,
	FINALIZE } apucmdmodetype;
apucmdmodetype cmdmode = INIT;

bit [31:0] apucmd;			// Command code
bit [31:0] apusourceaddr;		// Memory address to DMA the audio samples from
bit [9:0] apuwordcount;		// Number of words to playback, minus one
bit [7:0] apuburstcount;		// Number of 16byte DMA reads, minus one

// Buffer address high bit to control DMA write page
bit writeBufferSelect;

// Internal sample buffers with up to 1K 32 bit(L/R) samples each (2x4096 bytes)
// This means each buffer has: 1024 stereo samples max, 256 bursts to read max
// e.g. for 512x16bit stereo samples  we have 512 pairs to read, 128 bursts to make
// Read and Write regions always alternate between offset 0 and offset 256
bit samplewe;
bit [7:0] writeCursor;
bit [9:0] readCursor;
bit [1:0] readLowbits;
bit [127:0] sampleIn;
bit samplere;
wire [31:0] sampleOut;

wire [8:0] inaddr = {writeBufferSelect, writeCursor};
wire [10:0] outaddr = {~writeBufferSelect, readCursor};

// Internal L/R copies to stream out
bit [31:0] tx_data_lr;
bit re;
assign audiore = re;
assign tx_sdout = tx_data_lr;

// Internal sample memory
samplemem samplememinst (
  .clka(aclk),
  .wea(samplewe),
  .addra(inaddr),
  .dina(sampleIn),
  .clkb(audioclock),
  .addrb(outaddr),
  .enb(samplere),
  .doutb(sampleOut) );

// Number of buffer swaps so far (CDC from audio clock to bus clock)
bit bufferSwap;
(* async_reg = "true" *) bit bufferSwapCDC1;
(* async_reg = "true" *) bit bufferSwapCDC2;

bit [3:0] sampleoutputrateselector;

always_ff @(posedge aclk) begin
	if (~delayedresetn) begin
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
	if (~delayedresetn) begin
		re <= 1'b0;
		samplewe <= 1'b0;
		counterenabled <= 1'b0;
		writeCursor <= 8'd0;
		sampleoutputrateselector <= 4'b0000;
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
					apuwordcount <= audiodin[9:0];		// wordcount-1 (0..1023), typically 1023, times 4 for rate matching
					apuburstcount <= audiodin[10:2];	// burstcount = wordcount>>2, typically 255
					// Advance FIFO
					re <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end
	
			APUSETRATE: begin
				if (abvalid && ~abempty) begin
					// 2'b00:x1, 2'b01:x2, 2'b10:x4, 2'b11:quiet
					unique case(audiodin[1:0])
						2'b00: sampleoutputrateselector <= 4'b0100;
						2'b01: sampleoutputrateselector <= 4'b0010;
						2'b10: sampleoutputrateselector <= 4'b0001;
						2'b11: sampleoutputrateselector <= 4'b0000;
					endcase
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

always@(posedge audioclock) begin
	if (~rstaudion) begin
		tx_data_lr <= 0;
		readCursor <= 10'd0;
		readLowbits <= 2'd0;
		bufferSwap <= 1'd0;
		writeBufferSelect <= 1'b0;
		samplere <= 1'b0;
	end else begin

		samplere <= 1'b0;

		if (count==9'h0ff) begin
			// Step cursor based on playback rate
			{readCursor, readLowbits} <= {readCursor, readLowbits} + {8'd0, sampleoutputrateselector};

			// New sample and read enable control
			tx_data_lr <= sampleoutputrateselector == 4'd0 ? 32'd0 : sampleOut;
			samplere <= sampleoutputrateselector == 4'd0 ? 1'b0 : 1'b1;
		end

		// Read next pair of stereo samples
		tx_data_lr <= sampleoutputrateselector == 4'd0 ? 32'd0 : sampleOut;
		samplere <= sampleoutputrateselector == 4'd0 ? 1'b0 : 1'b1;

		// Increment swap count at end of buffer
		if (readCursor == apuwordcount) begin
			// Switch playback buffer and also swap CPU side r/w page ID
			bufferSwap <= ~bufferSwap;
			writeBufferSelect <= ~writeBufferSelect;
			readCursor <= 10'd0;
			readLowbits <= 2'd0;
		end
	end
end

endmodule
