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

wire [16:0] tag = addr[30:14];	// Cache tag
wire [7:0] line = addr[13:6];	// Cache line
wire [3:0] offset = addr[5:2];	// Cache word offset

logic readdone = 1'b0;
assign rready = readdone;

logic [31:0] dataout = 32'd0;
assign dout = dataout;

logic [31:0] cacheaddress;
data_t cachedin[0:3];
logic memwritestrobe = 1'b0;
logic memreadstrobe = 1'b0;

logic [16:0] ctag;					// current cache tag (17 bits)
logic [3:0] coffset;				// current word offset 0..15
logic [7:0] clineaddr = 8'd0;		// current cache line 0..256
logic [7:0] lastline = 9'h1FF;		// last cache line we've read
logic [17:0] lasttag = 18'h3FFFF;	// last tag we've encountered

logic [17:0] cachelinetags[0:255];	// cache line tags (17 bits) + 1 bit for valid flag

logic [63:0] cachewe = 64'd0;		// byte select for 64 byte cache line
logic [511:0] cdin;					// input data to write to cache
wire [511:0] cdout;					// output data read from cache
logic [511:0] lastcdout;			// last cache line contents we've read

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
cachememhalf CacheMemory256Lines(
	.addra(cacheaccess),		// current cache line
	.clka(aclk),				// cache clock
	.dina(cdin),				// updated cache data to write
	.wea(cachewe),				// write strobe for current cache line
	.douta(cdout),				// output of currently selected cache line
	.rsta(~aresetn),			// Reset
	.rsta_busy(rsta_busy) );	// Reset busy

initial begin
	for (int i=0; i<256; i=i+1) begin	// 256 lines total
		cachelinetags[i] = 'd0;			// contents invalid, at start of memory
	end
end

logic clinewe = 1'b0;
logic [17:0] clinedin;
always @(posedge aclk) begin
	if (clinewe)
		cachelinetags[clineaddr] <= clinedin;
end
wire [17:0] clinedout = cachelinetags[clineaddr];

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

typedef enum logic [4:0] {
	IDLE,
	CREAD,
	CPOPULATE, CPOPULATEWAIT, CUPDATE, CUPDATEDELAY,
	INVALIDATEBEGIN, INVALIDATESTEP} cachestatetype;
cachestatetype cachestate = IDLE;

always_ff @(posedge aclk) begin
	memreadstrobe <= 1'b0;
	readdone <= 1'b0;
	cachewe <= 64'd0;
	clinewe <= 1'b0;

	unique case(cachestate)
		IDLE : begin
			coffset <= offset;	// Cache offset 0..15
			clineaddr <= line;	// Cache line
			ctag <= tag;		// Cache tag 00000..1ffff
			dccount <= 8'h00;

			// Return rapidly if we're still accessing the same
			// cache line and have the same tag
			if ((lastline == {1'b0, line}) && (lasttag == {1'b0, tag})) begin
				unique case(offset)
					4'b0000:  dataout <= lastcdout[31:0];
					4'b0001:  dataout <= lastcdout[63:32];
					4'b0010:  dataout <= lastcdout[95:64];
					4'b0011:  dataout <= lastcdout[127:96];
					4'b0100:  dataout <= lastcdout[159:128];
					4'b0101:  dataout <= lastcdout[191:160];
					4'b0110:  dataout <= lastcdout[223:192];
					4'b0111:  dataout <= lastcdout[255:224];
					4'b1000:  dataout <= lastcdout[287:256];
					4'b1001:  dataout <= lastcdout[319:288];
					4'b1010:  dataout <= lastcdout[351:320];
					4'b1011:  dataout <= lastcdout[383:352];
					4'b1100:  dataout <= lastcdout[415:384];
					4'b1101:  dataout <= lastcdout[447:416];
					4'b1110:  dataout <= lastcdout[479:448];
					4'b1111:  dataout <= lastcdout[511:480];
				endcase
				readdone <= ren;
			end

			unique casex ({icacheflush, ren})
				2'b1x: cachestate <= INVALIDATEBEGIN;
				2'b01: cachestate <= ((lastline == {1'b0, line}) && (lasttag == {1'b0, tag})) ? IDLE : CREAD;
				2'b00: cachestate <= IDLE;
			endcase
		end
		
		INVALIDATEBEGIN: begin
			// Invalidate
			lastline = 9'h1FF;
			lasttag = 18'h3FFFF;
			clineaddr <= dccount;
			clinedin <= 18'd0; // invalid + zero tag
			clinewe <= 1'b1;
			cachestate <= INVALIDATESTEP;
		end

		INVALIDATESTEP: begin
			dccount <= dccount + 8'd1;
			readdone <= dccount == 8'hFF;
			cachestate <= dccount == 8'hFF ? IDLE : INVALIDATEBEGIN;
		end

		CREAD: begin
			if ({1'b1, ctag} == clinedout) begin // Hit
				unique case(coffset)
					4'b0000:  dataout <= cdout[31:0];
					4'b0001:  dataout <= cdout[63:32];
					4'b0010:  dataout <= cdout[95:64];
					4'b0011:  dataout <= cdout[127:96];
					4'b0100:  dataout <= cdout[159:128];
					4'b0101:  dataout <= cdout[191:160];
					4'b0110:  dataout <= cdout[223:192];
					4'b0111:  dataout <= cdout[255:224];
					4'b1000:  dataout <= cdout[287:256];
					4'b1001:  dataout <= cdout[319:288];
					4'b1010:  dataout <= cdout[351:320];
					4'b1011:  dataout <= cdout[383:352];
					4'b1100:  dataout <= cdout[415:384];
					4'b1101:  dataout <= cdout[447:416];
					4'b1110:  dataout <= cdout[479:448];
					4'b1111:  dataout <= cdout[511:480];
				endcase
				// Remember for next time in case we hit the same line again
				lastcdout <= cdout;
				lastline <= {1'b0, clineaddr};
				lasttag <= {1'b0, ctag};
				readdone <= 1'b1;
				cachestate <= IDLE;
			end else begin // Miss
				cachestate <= CPOPULATE;
			end
		end

		CPOPULATE : begin
			// Same as current memory address with device selector, aligned to cache boundary, top bit ignored (cached address)
			cacheaddress <= {1'b0, ctag, clineaddr, 6'd0};
			memreadstrobe <= 1'b1;
			cachestate <= CPOPULATEWAIT;
		end

		CPOPULATEWAIT: begin
			cachestate <= rdone ? CUPDATE : CPOPULATEWAIT;
		end

		CUPDATE: begin
			cachewe <= 64'hFFFFFFFFFFFFFFFF; // All entries
			cdin <= {cachedin[3], cachedin[2], cachedin[1], cachedin[0]}; // Data from memory
			clinedin <= {1'b1, ctag}; // valid + tag
			clinewe <= 1'b1;
			cachestate <= CUPDATEDELAY;
		end

		CUPDATEDELAY: begin
			// Delay state for tag write completion
			cachestate <= CREAD;
		end
	endcase

	if (~aresetn) begin
		cachestate <= IDLE;
	end
end

endmodule
