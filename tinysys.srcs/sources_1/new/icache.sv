`timescale 1ns / 1ps

`include "shared.vh"

module instructioncache(
	input wire aclk,
	input wire aresetn,
	input wire icacheflush,
	// custom bus to cpu
	input wire [31:0] addr,
	output wire [31:0] dout,
	input wire ren,
	output wire rready,
	axi4if.master m_axi);

wire [13:0] tag = addr[27:14];	// Cache tag
wire [7:0] line = addr[13:6];	// Cache line
wire [3:0] offset = addr[5:2];	// Cache word offset

logic readdone;
assign rready = readdone;

logic [31:0] dataout;
assign dout = dataout;

logic [31:0] cacheaddress;
data_t cachedin[0:3];
logic memreadstrobe;

logic [13:0] ctag;			// current cache tag (14 bits)
logic [3:0] coffset;		// current word offset 0..15
logic [7:0] clineaddr;		// current cache line 0..256

logic [14:0] cachelinetags[0:255];	// cache line tags (14 bits) + 1 bit for valid flag

logic cachewe;						// write control
logic [511:0] cdin;					// input data to write to cache
wire [511:0] cdout;					// output data read from cache

logic [7:0] dccount;				// line counter for cache flush/invalidate ops

logic memreset;
always_ff @(posedge aclk) begin
	memreset <= ~aresetn;
end

cachememhalf CacheMemory256Lines(
	.addra(line),				// current cache line
	.clka(aclk),				// cache clock
	.dina(cdin),				// updated cache data to write
	.wea(cachewe),				// write strobe for current cache line
	.douta(cdout),				// output of currently selected cache line
	.rsta(memreset));			// Reset

initial begin
	for (int i=0; i<256; i=i+1) begin	// 256 lines total
		cachelinetags[i] = 'd0;			// contents invalid, at start of memory
	end
end

logic ctagwe;
logic [14:0] clinedin;
always @(posedge aclk) begin
	if (ctagwe)
		cachelinetags[clineaddr] <= clinedin;
end
wire [14:0] ctagdout = cachelinetags[clineaddr];
wire cachehit = ({1'b1, ctag} == ctagdout) ? 1'b1 : 1'b0;

// ----------------------------------------------------------------------------
// cached/uncached memory controllers
// ----------------------------------------------------------------------------

wire rdone;
cachedmemorycontroller instructioncachectlinst(
	.aclk(aclk),
	.aresetn(aresetn),
	// From cache
	.addr(cacheaddress),
	.din(),
	.dout(cachedin),
	.start_read(memreadstrobe),
	.start_write(1'b0),
	.wdone(),
	.rdone(rdone),
	// To memory
	.m_axi(m_axi) );

typedef enum logic [2:0] {
	IDLE,
	CREAD,
	CPOPULATE, CUPDATE, CWRITEDELAY,
	INVALIDATEBEGIN, INVALIDATESTEP} cachestatetype;
cachestatetype cachestate = IDLE;

wire countdone = (dccount == 8'hFF);

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		memreadstrobe <= 1'b0;
		readdone <= 1'b0;
		cachewe <= 1'b0;
		ctagwe <= 1'b0;
		dccount <= 8'h00;
		dataout <= 32'd0;
		ctag <= 14'd0;
		coffset <= 4'd0;
		clineaddr <= 8'd0;
	end else begin
		memreadstrobe <= 1'b0;
		readdone <= 1'b0;
		cachewe <= 1'b0;
		ctagwe <= 1'b0;
		
		unique case(cachestate)
			IDLE : begin
				coffset <= offset;	// Cache offset 0..15
				clineaddr <= line;	// Cache line
				ctag <= tag;		// Cache tag 0000..3fff
				dccount <= 8'h00;
	
				casex ({icacheflush, ren})
					2'b1x: cachestate <= INVALIDATEBEGIN;
					2'bx1: cachestate <= CREAD;
					default: cachestate <= IDLE;
				endcase
			end
			
			INVALIDATEBEGIN: begin
				// Invalidate
				clineaddr <= dccount;
				clinedin <= 15'd0; // invalid + zero tag
				ctagwe <= 1'b1;
				cachestate <= INVALIDATESTEP;
			end
	
			INVALIDATESTEP: begin
				dccount <= dccount + 8'd1;
				readdone <= countdone;
				cachestate <= countdone ? IDLE : INVALIDATEBEGIN;
			end
	
			CREAD: begin
				if (cachehit) begin
					// Cache hit
					dataout <= cdout[coffset*32 +: 32];
					readdone <= 1'b1;
					cachestate <= IDLE;
				end else begin
					// Cache miss
					cachestate <= CPOPULATE;
				end
			end
	
			CPOPULATE : begin
				// Same as current memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
				cacheaddress <= {4'b0, ctag, clineaddr, 6'd0};
				memreadstrobe <= 1'b1;
				clinedin <= {1'b1, ctag}; // update valid + tag
				ctagwe <= 1'b1;
				cachestate <= CUPDATE;
			end
	
			CUPDATE: begin
				cdin <= {cachedin[3], cachedin[2], cachedin[1], cachedin[0]}; // Data from memory
				cachewe <= rdone;
				cachestate <= rdone ? CWRITEDELAY : CUPDATE;
			end
	
			CWRITEDELAY: begin
				// Delay state for tag write completion
				cachestate <= CREAD;
			end
		endcase

	end
end

endmodule
