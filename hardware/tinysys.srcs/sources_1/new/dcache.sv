`timescale 1ns / 1ps
`default_nettype none

`include "shared.vh"

module datacache(
	input wire aclk,
	input wire aresetn,
	// custom bus to cpu
	input wire [1:0] dcacheop,
	input wire [31:0] addr,
	input wire [31:0] din,
	output wire [31:0] dout,
	input wire [3:0] wstrb,
	input wire ren,
	output wire rready,
	output wire wready,
	axi4if.master a4buscached,
	axi4if.master a4busuncached );

wire iscached = ~addr[31];		// Cached access when high
wire [12:0] tag = addr[27:15];	// Cache tag
wire [8:0] line = addr[14:6];	// Cache line
wire [3:0] offset = addr[5:2];	// Cache word offset

logic [31:0] inputdata;
logic [31:0] cacheaddress;
data_t cachedin[0:3];
data_t cachedout[0:3]; // x4 128 bits
logic memwritestrobe;
logic memreadstrobe;

logic [31:0] ucaddrs;
logic [31:0] ucdout;
wire [31:0] ucdin;
logic [3:0] ucwstrb;
logic ucre;
wire ucwritedone;
wire ucreaddone;

logic [3:0] bsel;	// copy of wstrobe
logic [1:0] rwmode;	// r/w mode bits

logic [13:0] ptag;		// previous cache tag (13 bits) + 1 valid bit 
logic [13:0] ctag;		// current cache tag (13 bits) + 1 valid bit
logic [8:0] cline;		// current cache line 0..511
logic [3:0] coffset;	// current word offset 0..15

logic cachelinewb[0:511];										// cache line needs write back when high
(* extract_reset = "yes" *) logic [13:0] cachelinetags[0:511];	// cache line tags (13 bits) + 1 valid bit

logic [63:0] cachewe = 64'd0;	// byte select for 64 byte cache line
logic [511:0] cdin;				// input data to write to cache
wire [511:0] cdout;				// output data read from cache

logic flushing;			// high during cache flush operation
logic [13:0] flushtag;	// contents of line being flushed
logic [8:0] dccount;	// line counter for cache flush/invalidate ops

logic [8:0] cacheaccess;
always_comb begin
	if (flushing)
		cacheaccess = dccount;
	else
		cacheaccess = line;
end

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

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
		cachelinetags[i] = 14'd0;		// top of memory and invalid by default
	end
end

// ----------------------------------------------------------------------------
// cached/uncached memory controllers
// ----------------------------------------------------------------------------

wire rdone, wdone;
cachedmemorycontroller datacachectlinst(
	.aclk(aclk),
	.aresetn(delayedresetn),
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
	.aresetn(delayedresetn),
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
	IDLE,
	CWRITE, CREAD,
	UCWRITE, UCWRITEDELAY, UCREAD, UCREADDELAY,
	CWBACK, CWBACKWAIT,
	CPOPULATE, CPOPULATEWAIT, CUPDATE, CUPDATEDELAY,
	CDISCARDBEGIN, CDISCARDSTEP,
	CFLUSHBEGIN, CFLUSHWAITCREAD, CFLUSH, CFLUSHSKIP, CFLUSHWAIT } cachestatetype;
cachestatetype cachestate;

wire countdone = dccount == 9'h1FF;

// ----------------------------------------------------------------------------
// Read/Write completion
// ----------------------------------------------------------------------------

logic [31:0] dout_r;
logic rready_r, wready_r;

assign dout = dout_r;
assign rready = rready_r;
assign wready = wready_r;

// ----------------------------------------------------------------------------
// Main state machine
// ----------------------------------------------------------------------------

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		cdin <= 512'd0;
	end else begin
		unique case(cachestate)
			CWRITE:		cdin <= {inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata, inputdata};
			CUPDATE:	cdin <= {cachedin[3], cachedin[2], cachedin[1], cachedin[0]}; // Data from memory
			default:	cdin <= 512'd0;
		endcase
	end
end

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		memwritestrobe <= 1'b0;
		memreadstrobe <= 1'b0;
		flushing <= 1'b0;
		bsel <= 4'h0;
		rwmode <= 2'b00;
		ucwstrb <= 4'h0;
		ucre <= 1'b0;
		flushtag <= 14'd0;
		dccount <= 9'd0;
		cachestate <= IDLE;
		ptag <= 14'd0;
		ctag <= 14'd0;
		cline <= 9'd0;
		coffset <= 4'd0;
		inputdata <= 32'd0;
		cacheaddress <= 32'd0;
		cachewe <= 64'd0;
		wready_r <= 1'b0;
		rready_r <= 1'b0;
		dout_r <= 32'd0;
	end else begin
		memwritestrobe <= 1'b0;
		memreadstrobe <= 1'b0;
		wready_r <= 1'b0;
		rready_r <= 1'b0;
		ucwstrb <= 4'h0;
		ucre <= 1'b0;
		cachewe <= 64'd0;

		unique case(cachestate)
			IDLE : begin
				rwmode <= {ren, |wstrb};		// Record r/w mode
				bsel <= wstrb;					// Write byte select
				coffset <= offset;				// Cache offset 0..15
				cline <= line;					// Cache line
				ctag <= {1'b1, tag};			// Cache tag 0000..1fff (we want it to be 'valid')
				ptag <= cachelinetags[line];	// Previous cache tag + validity bit
				inputdata <= din;

				casex ({dcacheop, iscached, ren, |wstrb})
					5'b00101: cachestate <= CWRITE;
					5'b00110: cachestate <= CREAD;
					5'b00001: cachestate <= UCWRITE;
					5'b00010: cachestate <= UCREAD;
					5'b01xxx: cachestate <= CDISCARDBEGIN;
					5'b11xxx: cachestate <= CFLUSHBEGIN;
					default: cachestate <= IDLE;
				endcase
			end

			CDISCARDBEGIN: begin
				// Clear and invalidate cache line
				cachelinewb[dccount] <= 1'b0;
				cachelinetags[dccount] <= 14'd0;
				cachestate <= CDISCARDSTEP;
			end

			CDISCARDSTEP: begin
				// Go to next line (wraps around to 0 at 511)
				dccount <= dccount + 9'd1;
				// Finish our mock 'write' operation
				wready_r <= countdone;
				// Repeat until we process line 0xFF and go back to idle state
				cachestate <= countdone ? IDLE : CDISCARDBEGIN;
			end

			CFLUSHBEGIN: begin
				// Switch cache address to use flush counter
				flushing <= 1'b1;
				cachestate <= CFLUSHWAITCREAD;
			end

			CFLUSHWAITCREAD: begin
				// We keep the tag same, since we only want to make sure data is written back, not evicted
				flushtag <= cachelinetags[dccount];
				// One clock delay to read cache value at {dccount}
				cachestate <= CFLUSH;
			end

			CFLUSH: begin
				// Write back if it's a valid entry and we've modified it
				if (cachelinewb[dccount] && flushtag[13]) begin
					// Write current line back to RAM
					cacheaddress <= {4'd0, flushtag[12:0], dccount, 6'd0};
					cachedout <= {cdout[127:0], cdout[255:128], cdout[383:256], cdout[511:384]};
					memwritestrobe <= 1'b1;
					// We're done if this was the last write
					cachestate <= CFLUSHWAIT;
				end else begin // Otherwise, skip write back
					// Skip this line if it doesn't need a write back operation
					cachestate <= CFLUSHSKIP;
				end
			end

			CFLUSHSKIP: begin
				// Nothing to write back for next time around
				cachelinewb[dccount] <= 1'b0;
				// Go to next line (wraps around to 0 at 511)
				dccount <= dccount + 9'd1;
				// Stop 'flushing' mode if we're done
				flushing <= ~countdone;
				// Finish our mock 'write' operation if we're done
				wready_r <= countdone;
				// Repeat until we process line 0x1FF and go back to idle state
				cachestate <= countdone ? IDLE : CFLUSHWAITCREAD;
			end

			CFLUSHWAIT: begin
				if (wdone) begin
					// Go to next line (wraps around to 0 at 511)
					dccount <= dccount + 9'd1;
					// Stop 'flushing' mode if we're done
					flushing <= ~countdone;
					// Finish our mock 'write' operation if we're done
					wready_r <= countdone;
					// Repeat until we process line 0x1FF and go back to idle state
					cachestate <= countdone ? IDLE : CFLUSHWAITCREAD;
				end else begin
					// Memory write didn't complete yet
					cachestate <= CFLUSHWAIT;
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
					wready_r <= 1'b1;
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
					dout_r <= ucdin;
					rready_r <= 1'b1;
					cachestate <= IDLE;
				end else begin
					cachestate <= UCREADDELAY;
				end
			end

			CWRITE: begin
				if (ctag == ptag) begin
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
					wready_r <= 1'b1;
					cachestate <= IDLE;
				end else begin
					cachestate <= cachelinewb[cline] ? CWBACK : CPOPULATE;
				end
			end

			CREAD: begin
				if (ctag == ptag) begin
					dout_r <= cdout[coffset*32 +: 32];
					rready_r <= 1'b1;
					cachestate <= IDLE;
				end else begin // Cache miss when ctag != ptag
					cachestate <= cachelinewb[cline] ? CWBACK : CPOPULATE;
				end
			end

			CWBACK : begin
				// Use old memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
				cacheaddress <= {4'd0, ptag[12:0], cline, 6'd0};
				cachedout <= {cdout[127:0], cdout[255:128], cdout[383:256], cdout[511:384]};
				memwritestrobe <= 1'b1;
				cachestate <= CWBACKWAIT;
			end

			CWBACKWAIT: begin
				cachestate <= wdone ? CPOPULATE : CWBACKWAIT;
			end

			CPOPULATE : begin
				// Same as current memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
				cacheaddress <= {4'd0, ctag[12:0], cline, 6'd0};
				memreadstrobe <= 1'b1;
				cachestate <= CPOPULATEWAIT;
			end

			CPOPULATEWAIT: begin
				cachestate <= rdone ? CUPDATE : CPOPULATEWAIT;
			end

			CUPDATE: begin
				cachewe <= 64'hFFFFFFFFFFFFFFFF; // Write all entries
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
	end
end

endmodule
