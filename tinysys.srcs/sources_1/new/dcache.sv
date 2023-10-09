`timescale 1ns / 1ps

`include "shared.vh"

module datacache(
	input wire aclk,
	input wire aresetn,
	// custom bus to cpu
	input wire [1:0] dcacheop,
	input wire [31:0] addr,
	input wire [31:0] din,
	output logic [31:0] dout,
	input wire [3:0] wstrb,
	input wire ren,
	output logic rready,
	output logic wready,
	axi4if.master a4buscached,
	axi4if.master a4busuncached );

wire isuncached = addr[31];		// Uncached access
wire [15:0] tag = addr[30:15];	// Cache tag
wire [8:0] line = addr[14:6];	// Cache line
wire [3:0] offset = addr[5:2];	// Cache word offset

logic [31:0] inputdata;
logic [31:0] cacheaddress;
data_t cachedin[0:3];
data_t cachedout[0:3]; // x4 128 bits
logic memwritestrobe = 1'b0;
logic memreadstrobe = 1'b0;

logic [31:0] ucaddrs;
logic [31:0] ucdout;
wire [31:0] ucdin;
logic [3:0] ucwstrb = 4'h0;
logic ucre = 1'b0;
wire ucwritedone;
wire ucreaddone;

logic [3:0] bsel = 4'h0;			// copy of wstrobe
logic [1:0] rwmode = 2'b00;			// r/w mode bits

logic [15:0] ptag;					// previous cache tag (16 bits)
logic [15:0] ctag;					// current cache tag (16 bits)
logic [8:0] cline;					// current cache line 0..511
logic [3:0] coffset;				// current word offset 0..15

logic cachelinewb[0:511];			// cache line needs write back when high
logic [15:0] cachelinetags[0:511];	// cache line tags (16 bits)

logic [63:0] cachewe = 64'd0;		// byte select for 64 byte cache line
logic [511:0] cdin = 512'd0;		// input data to write to cache
wire [511:0] cdout;					// output data read from cache

logic flushing = 1'b0;				// high during cache flush operation
logic [15:0] flushline = 16'd0;		// contents of line being flushed
logic [8:0] dccount = 9'd0;			// line counter for cache flush/invalidate ops

logic [8:0] cacheaccess;
always_comb begin
	if (flushing)
		cacheaccess = dccount;
	else
		cacheaccess = line;
end

wire rsta_busy;
cachemem CacheMemory512Lines(
	.addra(cacheaccess),		// current cache line
	.clka(aclk),				// cache clock
	.dina(cdin),				// updated cache data to write
	.wea(cachewe),				// write strobe for current cache line
	.douta(cdout),				// output of currently selected cache line
	.rsta(~aresetn),			// Reset
	.rsta_busy(rsta_busy) );	// Reset busy

initial begin
	for (int i=0; i<512; i=i+1) begin	// 512 lines total
		cachelinewb[i] = 1'b0;			// cache lines do not require write-back for initial cache-miss
		cachelinetags[i] = 'd0;			// top of memory
	end
end

// ----------------------------------------------------------------------------
// cached/uncached memory controllers
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

// For now we have only one device, so we directly wire it here
uncachedmemorycontroller uncachedmemorycontrollerinst(
	.aclk(aclk),
	.aresetn(aresetn),
	// From cache
	.addr(ucaddrs),
	.din(ucdout),
	.dout(ucdin),
	.re(ucre),
	.wstrb(ucwstrb),
	.wdone(ucwritedone),
	.rdone(ucreaddone),
	// To memory mapped devices
	.m_axi(a4busuncached) );

typedef enum logic [4:0] {
	CRESET,
	IDLE,
	CWRITE, CREAD,
	UCWRITE, UCWRITEDELAY, UCREAD, UCREADDELAY,
	CWBACK, CWBACKWAIT,
	CPOPULATE, CPOPULATEWAIT, CUPDATE, CUPDATEDELAY,
	CDATANOFLUSHBEGIN, CDATANOFLUSHSTEP,
	CDATAFLUSHBEGIN, CDATAFLUSHWAITCREAD, CDATAFLUSH, CDATAFLUSHSKIP, CDATAFLUSHWAIT } cachestatetype;
cachestatetype cachestate = CRESET;

always_ff @(posedge aclk) begin
	memwritestrobe <= 1'b0;
	memreadstrobe <= 1'b0;
	wready <= 1'b0;
	rready <= 1'b0;
	ucwstrb <= 4'h0;
	ucre <= 1'b0;
	cachewe <= 64'd0;

	unique case(cachestate)
		CRESET: begin
			rwmode <= 2'b00;
			dccount <= 9'd0;
			cachestate <= IDLE;
		end

		IDLE : begin
			rwmode <= {ren, |wstrb};		// Record r/w mode
			bsel <= wstrb;					// Write byte select
			coffset <= offset;				// Cache offset 0..15
			cline <= line;					// Cache line
			ctag <= tag;					// Cache tag 00000..1ffff
			ptag <= cachelinetags[line];	// Previous cache tag
			inputdata <= din;

			casex ({dcacheop[0], isuncached, ren, |wstrb})
				4'b0001: cachestate <= CWRITE;
				4'b0010: cachestate <= CREAD;
				4'b0101: cachestate <= UCWRITE;
				4'b0110: cachestate <= UCREAD;
				4'b1xxx: cachestate <= dcacheop[1] ? CDATAFLUSHBEGIN : CDATANOFLUSHBEGIN;
				default: cachestate <= IDLE;
			endcase
		end

		CDATANOFLUSHBEGIN: begin
			// Clear and invalidate cache line
			cachelinewb[dccount] <= 1'b0;
			cachelinetags[dccount] <= 'd0;
			cachestate <= CDATANOFLUSHSTEP;
		end

		CDATANOFLUSHSTEP: begin
			// Go to next line (wraps around to 0 at 511)
			dccount <= dccount + 9'd1;
			// Finish our mock 'write' operation
			wready <= dccount == 9'h1FF;
			// Repeat until we process line 0xFF and go back to idle state
			cachestate <= dccount == 9'h1FF ? IDLE : CDATANOFLUSHBEGIN;
		end
		
		CDATAFLUSHBEGIN: begin
			// Switch cache address to use flush counter
			flushing <= 1'b1;
			cachestate <= CDATAFLUSHWAITCREAD;
		end

		CDATAFLUSHWAITCREAD: begin
			// We keep the tag same, since we only want to make sure data is written back, not evicted
			flushline <= cachelinetags[dccount];
			// One clock delay to read cache value at {dccount}
			cachestate <= CDATAFLUSH;
		end

		CDATAFLUSH: begin
			// Nothing to write back for next time around
			cachelinewb[dccount] <= 1'b0;
			// Either write back to memory or skip
			if (cachelinewb[dccount]) begin
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
			// Go to next line (wraps around to 0 at 511)
			dccount <= dccount + 9'd1;
			// Stop 'flushing' mode if we're done
			flushing <= dccount != 9'h1FF;
			// Finish our mock 'write' operation if we're done
			wready <= dccount == 9'h1FF;
			// Repeat until we process line 0x1FF and go back to idle state
			cachestate <= dccount == 9'h1FF ? IDLE : CDATAFLUSHWAITCREAD;
		end

		CDATAFLUSHWAIT: begin
			if (wdone) begin
				// Go to next line (wraps around to 0 at 511)
				dccount <= dccount + 9'd1;
				// Stop 'flushing' mode if we're done
				flushing <= dccount != 9'h1FF;
				// Finish our mock 'write' operation if we're done
				wready <= dccount == 9'h1FF;
				// Repeat until we process line 0x1FF and go back to idle state
				cachestate <= dccount == 9'h1FF ? IDLE : CDATAFLUSHWAITCREAD;
			end else begin
				// Memory write didn't complete yet
				cachestate <= CDATAFLUSHWAIT;
			end
		end

		UCWRITE: begin
			ucaddrs <= addr;
			ucdout <= inputdata;
			ucwstrb <= bsel;
			cachestate <= UCWRITEDELAY;
		end

		UCWRITEDELAY: begin
			if (ucwritedone) begin
				wready <= 1'b1;
				cachestate <= IDLE;
			end else begin
				cachestate <= UCWRITEDELAY;
			end
		end

		UCREAD: begin
			ucaddrs <= addr;
			ucre <= 1'b1;
			cachestate <= UCREADDELAY;
		end

		UCREADDELAY: begin
			if (ucreaddone) begin
				dout <= ucdin;
				rready <= 1'b1;
				cachestate <= IDLE;
			end else begin
				cachestate <= UCREADDELAY;
			end
		end

		CWRITE: begin
			if (ctag == ptag) begin
				cdin <= {	inputdata, inputdata, inputdata, inputdata,
							inputdata, inputdata, inputdata, inputdata,
							inputdata, inputdata, inputdata, inputdata,
							inputdata, inputdata, inputdata, inputdata};	// Incoming data replicated to 512bits, to be masked by cachewe
				unique case(coffset)
					4'b0000: cachewe <= { 60'd0, bsel        };
					4'b0001: cachewe <= { 56'd0, bsel, 4'd0  };
					4'b0010: cachewe <= { 52'd0, bsel, 8'd0  };
					4'b0011: cachewe <= { 48'd0, bsel, 12'd0 };
					4'b0100: cachewe <= { 44'd0, bsel, 16'd0 };
					4'b0101: cachewe <= { 40'd0, bsel, 20'd0 };
					4'b0110: cachewe <= { 36'd0, bsel, 24'd0 };
					4'b0111: cachewe <= { 32'd0, bsel, 28'd0 };
					4'b1000: cachewe <= { 28'd0, bsel, 32'd0 };
					4'b1001: cachewe <= { 24'd0, bsel, 36'd0 };
					4'b1010: cachewe <= { 20'd0, bsel, 40'd0 };
					4'b1011: cachewe <= { 16'd0, bsel, 44'd0 };
					4'b1100: cachewe <= { 12'd0, bsel, 48'd0 };
					4'b1101: cachewe <= { 8'd0,  bsel, 52'd0 };
					4'b1110: cachewe <= { 4'd0,  bsel, 56'd0 };
					4'b1111: cachewe <= {        bsel, 60'd0 };
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
			if (ctag == ptag) begin
				// Return word directly from cache
				unique case(coffset)
					4'b0000:  dout <= cdout[31:0];
					4'b0001:  dout <= cdout[63:32];
					4'b0010:  dout <= cdout[95:64];
					4'b0011:  dout <= cdout[127:96];
					4'b0100:  dout <= cdout[159:128];
					4'b0101:  dout <= cdout[191:160];
					4'b0110:  dout <= cdout[223:192];
					4'b0111:  dout <= cdout[255:224];
					4'b1000:  dout <= cdout[287:256];
					4'b1001:  dout <= cdout[319:288];
					4'b1010:  dout <= cdout[351:320];
					4'b1011:  dout <= cdout[383:352];
					4'b1100:  dout <= cdout[415:384];
					4'b1101:  dout <= cdout[447:416];
					4'b1110:  dout <= cdout[479:448];
					4'b1111:  dout <= cdout[511:480];
				endcase
				rready <= 1'b1;
				cachestate <= IDLE;
			end else begin // Cache miss when ctag != ptag
				cachestate <= cachelinewb[cline] ? CWBACK : CPOPULATE;
			end
		end

		CWBACK : begin
			// Use old memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
			cacheaddress <= {1'b0, ptag, cline, 6'd0};
			cachedout <= {cdout[127:0], cdout[255:128], cdout[383:256], cdout[511:384]};
			memwritestrobe <= 1'b1;
			cachestate <= CWBACKWAIT;
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
			// No need to write back since contents are now valid
			cachelinewb[cline] <= 1'b0;
			cachestate <= (rwmode == 2'b01) ? CWRITE : CREAD;
		end
	endcase

	if (~aresetn) begin
		cachestate <= CRESET;
	end
end

endmodule
