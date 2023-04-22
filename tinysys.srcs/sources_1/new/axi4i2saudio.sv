`timescale 1ns / 1ps

module axi4i2saudio(
	input wire aclk,		// Bus clock
	input wire aresetn,
    input wire audioclock,	// 22.591MHz master clock

	axi4if.master m_axi,	// Direct memory access

	input wire abempty,
	input wire abvalid,
	output wire audiore,
    input wire [31:0] audiodin,			// Audio input is 16bits in our device (TODO: this is an audio command instead)

	output wire [31:0] swapcount,		// Return the swap count

    output wire tx_mclk,		// Audio bus output
    output wire tx_lrck,		// L/R select
    output wire tx_sclk,		// Stream clock
    output logic tx_sdout );	// Stream out

// Counter for generating other divided clocks
logic [8:0] count = 9'd0;

always @(posedge audioclock)
	count <= count + 1;

wire lrck = count[8];
wire sclk = count[2];
assign tx_lrck = lrck;			// Channel select 44.1KHz (/512)
assign tx_sclk = sclk;			// Sample clock 2.823MHz (/8)
assign tx_mclk = audioclock;	// Master clock 22.519MHz

// Inernal L/R copies to stream out
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
	WAITSYNC, WAITACK,
	FINALIZE } apucmdmodetype;
apucmdmodetype cmdmode = WCMD;

logic [31:0] apucmd;				// Command code
logic [31:0] apusourceaddr;			// Memory address to DMA the audio samples from
logic [10:0] apuwordcount;			// Number of words to DMA and range to loop over
logic [9:0] apusyncpoint = 10'd0;	// Sync point to wait for

logic [9:0] readcursor = 10'd0;		// Current sample read positionl

// Internal sample buffers with up to 1K 32 bit(L/R) samples each (2x4096 bytes)
logic [127:0] samplebuffer[0:511];
// Buffer address high bit
logic currentBuffer = 1'b0;
// Number of buffer swaps so far
logic [31:0] bufferSwapCount = 32'd0;
assign swapcount = bufferSwapCount;

initial begin
	for(int i=0;i<512;i++) begin
		samplebuffer[i] = 128'd0;
	end
end

// Burst synchronization
logic burstStrobe = 1'b0;
logic dmaack = 1'b0;

logic [9:0] readcursorcdc1;
logic [9:0] readcursorcdc2;
always @(posedge aclk) begin
	readcursorcdc1 <= readcursor;
	readcursorcdc2 <= readcursorcdc1;
end

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		cmdmode <= WCMD;
	end else begin

		re <= 1'b0;
		burstStrobe <= 1'b0;

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
					32'h00000000:	cmdmode <= APUSTART;		// Start DMA into write page
					32'h00000001:	cmdmode <= APUBUFFERSIZE;	// Set up size of DMA copies and playback range, in words
					32'h00000002:	cmdmode <= APUSTOP;			// TODO: Stop all sound output
					32'h00000003:	cmdmode <= APUSWAP;			// Wait for read cursor to reach given value, then swap r/w pages, also increment swap count
					32'h00000004:	cmdmode <= APUSETRATE;		// TODO: Set sample duplication count, x1 (44.1KHz) default, can use x2(22KHz) or x4(11KHz)
					default:		cmdmode <= FINALIZE;		// Invalid command, wait one clock and try next
				endcase
			end

			APUSTART: begin
				if (abvalid && ~abempty) begin
					apusourceaddr <= audiodin;
					// This will kick DMA async to the command queue
					burstStrobe <= 1'b1;
					// Advance FIFO
					re <= 1'b1;
					cmdmode <= WAITACK;
				end
			end

			APUBUFFERSIZE: begin
				if (abvalid && ~abempty) begin
					apuwordcount <= audiodin[10:0];	// wordcount (0..1024)
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
				if (abvalid && ~abempty) begin
					apusyncpoint <= audiodin[9:0];
					// Advance FIFO
					re <= 1'b1;
					cmdmode <= WAITSYNC;
				end
			end

			APUSETRATE: begin
				// TODO:
				if (abvalid && ~abempty) begin
					//sampleoutputrate <= audiodin;
					// Advance FIFO
					re <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			WAITSYNC: begin
				// Wait for playback buffer's read cursor to reach desired sync point
				if (readcursorcdc2 == apusyncpoint) begin
					// Increment swap count
					bufferSwapCount <= bufferSwapCount + 32'd1;
					// Swap read/write buffers
					currentBuffer <= ~currentBuffer;
					cmdmode <= FINALIZE;
				end
			end

			WAITACK: begin
				cmdmode <= dmaack ? FINALIZE : WAITACK;
			end

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase

	end
end

// ------------------------------------------------------------------------------------
// Async burst read
// ------------------------------------------------------------------------------------

typedef enum logic [1:0] {
	DMAIDLE,
	STARTDMA, WAITREADADDR, READLOOP
} apudmamodetype;
apudmamodetype dmamode = DMAIDLE;

logic [7:0] burstcursor = 8'd0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		dmamode <= DMAIDLE;
	end else begin
		dmaack <= 1'b0;

		case (dmamode)
			DMAIDLE: begin
				dmaack <= 1'b1;
				dmamode <= burstStrobe == 1'b1 ? STARTDMA : DMAIDLE;
			end

			STARTDMA: begin
				burstcursor <= 8'd0;
				m_axi.arlen <= apuwordcount[10:3] - 8'd1;	// burstcount = (wordcount/4)-1
				m_axi.arvalid <= 1;
				m_axi.araddr <= apusourceaddr; 
				dmamode <= WAITREADADDR;
			end

			WAITREADADDR: begin
				if (m_axi.arready) begin
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					dmamode <= READLOOP;
				end
			end

			READLOOP: begin
				if (m_axi.rvalid) begin
					m_axi.rready <= ~m_axi.rlast;
					samplebuffer[{currentBuffer,burstcursor}] <= m_axi.rdata;
					burstcursor <= burstcursor + 8'd1;
					dmamode <= ~m_axi.rlast ? READLOOP : DMAIDLE;
				end
			end
		endcase
	end
end

// ------------------------------------------------------------------------------------
// I2S output
// ------------------------------------------------------------------------------------

always@(posedge audioclock) begin
	// Trigger new sample copy just before we select L channel again out of the LR pair
	if (count==9'h0ff) begin
		unique case (readcursor[1:0])
			2'b00: tx_data_lr <= samplebuffer[{~currentBuffer,readcursor}[9:2]][31 : 0];
			2'b01: tx_data_lr <= samplebuffer[{~currentBuffer,readcursor}[9:2]][63 : 31];
			2'b10: tx_data_lr <= samplebuffer[{~currentBuffer,readcursor}[9:2]][95 : 63];
			2'b11: tx_data_lr <= samplebuffer[{~currentBuffer,readcursor}[9:2]][127: 96];
		endcase
		readcursor <= (readcursor + 10'd1) % apuwordcount;
	end
end

logic [23:0] tx_data_l_shift = 24'b0;
logic [23:0] tx_data_r_shift = 24'b0;

always@(posedge audioclock)
	if (count == 3'b000000111) begin
		tx_data_l_shift <= {tx_data_lr[31:16],8'd0};
		tx_data_r_shift <= {tx_data_lr[15:0],8'd0};
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