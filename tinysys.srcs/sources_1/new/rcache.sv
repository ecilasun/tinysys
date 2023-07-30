`timescale 1ns / 1ps

`include "shared.vh"

module rastercache(
	input wire aclk,
	input wire aresetn,
	// custom bus to RPU 
	input wire [31:0] addr,
	input wire [127:0] din,
	output logic [127:0] dout,
	input wire [15:0] wstrb,
	input wire ren,
	input wire flush,
	output logic rready,
	output logic wready,
	axi4if.master a4buscached);

wire [7:0] line = addr[13:6];	// Cache line
wire [16:0] tag = addr[30:14];	// Cache tag
wire [1:0] offset = addr[5:4];	// Cache qword offset
// Address bits 3:0 are unused since everything is 16 byte aligned from RPU side

logic [127:0] inputdata;
logic [31:0] cacheaddress;
data_t cachedin[0:3];
data_t cachedout[0:3]; // x4 128 bits
logic memwritestrobe = 1'b0;
logic memreadstrobe = 1'b0;

logic [15:0] bsel = 16'h0;			// copy of wstrobe
logic [1:0] rwmode = 2'b00;			// r/w mode bits
logic [16:0] ptag;					// previous cache tag (17 bits)
logic [16:0] ctag;					// current cache tag (17 bits)
logic [1:0] coffset;				// current qword offset 0..3
logic [7:0] cline;					// current cache line 0..256
logic valid;						// valid flag for current cache line

logic cachelinewb[0:255];			// cache line needs write back when high
logic cachelinevalid[0:255];		// cache line state (invalid / valid)
logic [16:0] cachelinetags[0:255];	// cache line tags (17 bits)

logic [63:0] cachewe = 64'd0;		// byte select for 64 byte cache line
logic [511:0] cdin = 512'd0;		// input data to write to cache
wire [511:0] cdout;					// output data read from cache

logic flushing = 1'b0;				// high during cache flush operation
logic [7:0] dccount = 8'h00;		// line counter for cache flush/invalidate ops
logic [16:0] flushline = 18'd0;		// contents of line being flushed

logic [7:0] cacheaccess;
always_comb begin
	if (flushing)
		cacheaccess = dccount;
	else
		cacheaccess = line;
end

wire rsta_busy;
cachemem CacheMemory512(
	.addra(cacheaccess),		// current cache line
	.clka(aclk),				// cache clock
	.dina(cdin),				// updated cache data to write
	.wea(cachewe),				// write strobe for current cache line
	.douta(cdout),				// output of currently selected cache line
	.rsta(~aresetn),			// Reset
	.rsta_busy(rsta_busy) );	// Reset busy

initial begin
	for (int i=0; i<256; i=i+1) begin	// 256 lines total
		cachelinewb[i] = 1'b0;			// cache lines do not require write-back for initial cache-miss
		cachelinevalid[i] = 1'b0;		// this cache line is invalid (contents not associated with a memory location)
		cachelinetags[i] = 'd0;			// top of memory
	end
end

// ----------------------------------------------------------------------------
// cached memory controllers
// ----------------------------------------------------------------------------

wire rdone, wdone;
cachedmemorycontroller datacachectlinst(
	.aclk(aclk),
	.aresetn(aresetn),
	// From cache
	.addr(cacheaddress),
	.din(cachedout),
	.dout(cachedin),
	.start_read(memreadstrobe),
	.start_write(memwritestrobe),
	.wdone(wdone),
	.rdone(rdone),
	// To memory
	.m_axi(a4buscached) );

typedef enum logic [3:0] {
	IDLE,
	CWRITE, CREAD,
	CWBACK, CWBACKWAIT,
	CPOPULATE, CPOPULATEWAIT, CUPDATE, CUPDATEDELAY,
	CDATAFLUSHBEGIN, CDATAFLUSHWAITCREAD, CDATAFLUSH, CDATAFLUSHSKIP, CDATAFLUSHWAIT } cachestatetype;
cachestatetype cachestate = IDLE;

always_ff @(posedge aclk) begin
	memwritestrobe <= 1'b0;
	memreadstrobe <= 1'b0;
	wready <= 1'b0;
	rready <= 1'b0;
	cachewe <= 64'd0;

	unique case(cachestate)
		IDLE : begin
			rwmode <= {ren, |wstrb};		// Record r/w mode
			bsel <= wstrb;					// Write byte select
			coffset <= offset;				// Cache offset 0..3
			cline <= line;					// Cache line
			ctag <= tag;					// Cache tag 00000..1ffff
			ptag <= cachelinetags[line];	// Previous cache tag
			inputdata <= din;
			valid <= cachelinevalid[line];

			casex ({flush, ren, |wstrb})
				3'b001: cachestate <= CWRITE;
				3'b01x: cachestate <= CREAD;
				3'b1xx: cachestate <= CDATAFLUSHBEGIN;
				default: cachestate <= IDLE;
			endcase
		end

		CDATAFLUSHBEGIN: begin
			// Switch cache address to use flush counter
			flushing <= 1'b1;
			cachestate <= CDATAFLUSHWAITCREAD;
		end

		CDATAFLUSHWAITCREAD: begin
			flushline <= cachelinetags[dccount];
			// One clock delay to read cache value at {dccount}
			cachestate <= CDATAFLUSH;
		end

		CDATAFLUSH: begin
			// Nothing to write back for next time around
			cachelinewb[dccount] <= 1'b0;
			// Either write back to memory or skip
			// If cache line is valid, we keep it valid
			if (cachelinewb[dccount] && cachelinevalid[dccount]) begin
				// Write current line back to RAM
				cacheaddress <= {1'b0, flushline, dccount, 6'd0};
				cachedout <= {cdout[127:0], cdout[255:128], cdout[383:256], cdout[511:384]};
				memwritestrobe <= 1'b1;
				// We're done if this was the last write
				cachestate <= CDATAFLUSHWAIT;
			end else begin // Otherwise, skip write back
				// Skip this line if it doesn't need a write back operation
				cachestate <= CDATAFLUSHSKIP;
			end
		end

		CDATAFLUSHSKIP: begin
			// Go to next line (wraps around to 0 at 255)
			dccount <= dccount + 8'd1;
			// Stop 'flushing' mode if we're done
			flushing <= dccount != 8'hFF;
			// Finish our mock 'write' operation if we're done
			wready <= dccount == 8'hFF;
			// Repeat until we process line 0xFF and go back to idle state
			cachestate <= dccount == 8'hFF ? IDLE : CDATAFLUSHWAITCREAD;
		end

		CDATAFLUSHWAIT: begin
			if (wdone) begin
				// Go to next line (wraps around to 0 at 255)
				dccount <= dccount + 8'd1;
				// Stop 'flushing' mode if we're done
				flushing <= dccount != 8'hFF;
				// Finish our mock 'write' operation if we're done
				wready <= dccount == 8'hFF;
				// Repeat until we process line 0xFF and go back to idle state
				cachestate <= dccount == 8'hFF ? IDLE : CDATAFLUSHWAITCREAD;
			end else begin
				// Memory write didn't complete yet
				cachestate <= CDATAFLUSHWAIT;
			end
		end

		CWRITE: begin
			if ((ctag == ptag) && valid) begin
				cdin <= { inputdata, inputdata, inputdata, inputdata };	// Incoming data replicated to 512bits, to be masked by cachewe
				unique case(coffset)
					2'b00: cachewe <= { 48'd0, bsel        };
					2'b01: cachewe <= { 32'd0, bsel, 16'd0 };
					2'b10: cachewe <= { 16'd0, bsel, 32'd0 };
					2'b11: cachewe <= {        bsel, 48'd0 };
				endcase
				// This cache line needs to be written back to memory on next miss
				cachelinewb[cline] <= 1'b1;
				wready <= 1'b1;
				cachestate <= IDLE;
			end else begin
				cachestate <= cachelinewb[cline] ? CWBACK : CPOPULATE;
			end
		end

		CREAD: begin
			if ((ctag == ptag) && valid) begin
				// Return word directly from cache
				unique case(coffset)
					2'b00:  dout <= cdout[127:0];
					2'b01:  dout <= cdout[255:128];
					2'b10:  dout <= cdout[383:256];
					2'b11:  dout <= cdout[511:384];
				endcase
				rready <= 1'b1;
				cachestate <= IDLE;
			end else begin // Cache miss when ctag != ptag
				cachestate <= cachelinewb[cline] ? CWBACK : CPOPULATE;
			end
		end

		CWBACK : begin
			if (valid) begin
				// Use old memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
				cacheaddress <= {1'b0, ptag, cline, 6'd0};
				cachedout <= {cdout[127:0], cdout[255:128], cdout[383:256], cdout[511:384]};
				memwritestrobe <= 1'b1;
				cachestate <= CWBACKWAIT;
			end else begin
				// Nothing to write back for invalid line, just populate
				cachestate <= CPOPULATE;
			end
		end

		CWBACKWAIT: begin
			cachestate <= wdone ? CPOPULATE : CWBACKWAIT;
		end

		CPOPULATE : begin
			// Same as current memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
			cacheaddress <= {1'b0, ctag, cline, 6'd0};
			memreadstrobe <= 1'b1;
			cachestate <= CPOPULATEWAIT;
		end

		CPOPULATEWAIT: begin
			cachestate <= rdone ? CUPDATE : CPOPULATEWAIT;
		end

		CUPDATE: begin
			cachewe <= 64'hFFFFFFFFFFFFFFFF; // All entries
			cdin <= {cachedin[3], cachedin[2], cachedin[1], cachedin[0]}; // Data from memory
			cachestate <= CUPDATEDELAY;
		end

		CUPDATEDELAY: begin
			ptag <= ctag;
			cachelinetags[cline] <= ctag;
			// No need to write back since contents are valid and unmodifed
			cachelinewb[cline] <= 1'b0;
			// Contents are now associated with a memory location
			cachelinevalid[cline] <= 1'b1;
			valid <= 1'b1;
			cachestate <= (rwmode == 2'b01) ? CWRITE : CREAD;
		end
	endcase

	if (~aresetn) begin
		cachestate <= IDLE;
	end
end

endmodule
