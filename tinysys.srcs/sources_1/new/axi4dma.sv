`timescale 1ns / 1ps

import axi4pkg::*;

module axi4dma(
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi,
	input wire dmafifoempty,
	input wire [31:0] dmafifodout,
	output wire dmafifore,
	input wire dmafifovalid,
	output wire romReady,
	output wire dmabusy);

logic cmdre = 1'b0;
logic dmainprogress = 1'b0;
logic ROMavailable = 1'b0;
assign dmafifore = cmdre;
assign dmabusy = dmainprogress;

assign romReady = ROMavailable;

// --------------------------------------------------
// Boot ROM
// Grouped as 128bit (16 byte) cache line entries
// --------------------------------------------------

logic [12:0] bootROMaddr;
logic [127:0] bootROM[0:8191];

initial begin
	$readmemh("romimage.mem", bootROM);
end

logic [127:0] bootROMdout;
logic romre = 1'b0;
always @(posedge aclk) begin
	if (romre) bootROMdout <= bootROM[bootROMaddr];
end

// ------------------------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------------------------

localparam burstcount = 8'd1;		// Single burst

assign m_axi.arlen = burstcount - 8'd1;
assign m_axi.arsize = SIZE_16_BYTE; // 128bit read bus
assign m_axi.arburst = BURST_INCR;	// auto address increment for burst

assign m_axi.awlen = burstcount - 8'd1;
assign m_axi.awsize = SIZE_16_BYTE; // 128bit write bus
assign m_axi.awburst = BURST_INCR;	// auto address increment for burst

// ------------------------------------------------------------------------------------
// Command FIFO
// ------------------------------------------------------------------------------------

typedef enum logic [2:0] {
	WCMD, DISPATCH,
	DMASOURCE,
	DMATARGET,
	DMALENGTH,
	DMAENQUEUE,
	FINALIZE } dmacmdmodetype;
dmacmdmodetype cmdmode = WCMD;

logic [31:0] dmacmd = 'd0;

wire dmaqueuefull;
wire dmaqueuevalid;
wire dmaqueueempty;
logic dmaqueuewe = 1'b0;
logic [95:0] dmaqueuedin = 0;
logic dmaqueuere = 1'b0;
wire [95:0] dmaqueuedout;

dmaqueue dmaqueueinst(
	.full(dmaqueuefull),
	.din(dmaqueuedin),
	.wr_en(dmaqueuewe),
	.empty(dmaqueueempty),
	.dout(dmaqueuedout),
	.rd_en(dmaqueuere),
	.valid(dmaqueuevalid),
	.clk(aclk),
	.rst(~aresetn) );

logic [31:0] dmasourceaddr;
logic [31:0] dmatargetaddr;
logic [31:0] dmablockcount;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		cmdmode <= WCMD;
	end else begin

		cmdre <= 1'b0;
		dmaqueuewe <= 1'b0;

		case (cmdmode)
			WCMD: begin
				if (dmafifovalid && ~dmafifoempty) begin
					dmacmd <= dmafifodout;
					// Advance FIFO
					cmdre <= 1'b1;
					// Dispatch cmd
					cmdmode <= DISPATCH;
				end
			end

			DISPATCH: begin
				case (dmacmd)
					32'h00000000:	cmdmode <= DMASOURCE;	// Source address, byte aligned
					32'h00000001:	cmdmode <= DMATARGET;	// Target address, byte aligned
					32'h00000002:	cmdmode <= DMALENGTH;	// Set length in bytes (divided into 128 bit blocks, r/w masks handle leading and trailing ledges)
					32'h00000003:	cmdmode <= DMAENQUEUE;	// Push current setup into DMA transfer queue
					default:		cmdmode <= FINALIZE;	// Invalid command, wait one clock and try next
				endcase
			end

			DMASOURCE: begin
				if (dmafifovalid && ~dmafifoempty) begin
					dmasourceaddr <= dmafifodout;
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			DMATARGET: begin
				if (dmafifovalid && ~dmafifoempty) begin
					dmatargetaddr <= dmafifodout;
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			DMALENGTH: begin
				if (dmafifovalid && ~dmafifoempty) begin
					// NOTE: Need to generate leadmask and trailmask for misaligned
					// start/end bits and manage the middle section count
					dmablockcount <= dmafifodout;
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			DMAENQUEUE: begin
				dmaqueuedin <= {dmasourceaddr, dmatargetaddr, dmablockcount};
				dmaqueuewe <= 1'b1;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase
	end
end

// ------------------------------------------------------------------------------------
// Output data fifo (populated by read unit)
// ------------------------------------------------------------------------------------

wire dmacopyfull, dmacopyempty, dmacopyvalid;
logic [127:0] dmacopydin;
wire [127:0] dmacopydout;
logic dmacopyre = 0;
logic dmacopywe = 0;

dmadatafifo dmadatafifoinst(
	.full(dmacopyfull),
	.din(dmacopydin),
	.wr_en(dmacopywe),
	.empty(dmacopyempty),
	.dout(dmacopydout),
	.rd_en(dmacopyre),
	.valid(dmacopyvalid),
	.clk(aclk),
	.rst(~aresetn) );

// ------------------------------------------------------------------------------------
// DMA logic - Read
// ------------------------------------------------------------------------------------

typedef enum logic [2:0] {DETECTCMD, STARTDMA, DMAREADSOURCE, COPYBLOCK, COPYLOOP, WAITFORPENDINGWRITES} dmareadstatetype;
dmareadstatetype dmareadstate = DETECTCMD;

logic [31:0] dmaop_source;
logic [31:0] dmaop_target;
logic [31:0] dmaop_count;
logic writestrobe = 0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		dmareadstate <= DETECTCMD;
	end else begin

		dmaqueuere <= 1'b0;
		dmacopywe <= 1'b0;
		writestrobe <= 1'b0;

		case (dmareadstate)
			DETECTCMD: begin
				// TODO: Instead of strobe, write command to a DMA fifo
				if (~dmaqueueempty && dmaqueuevalid) begin

					// TBD: This unit doesn't handle misaligned copies yet
					// TBD: This unit doesn't do burst copy yet

					dmaop_source <= dmaqueuedout[95:64];
					dmaop_target <= dmaqueuedout[63:32];
					dmaop_count <= dmaqueuedout[31:0];

					dmainprogress <= 1'b1;
					writestrobe <= 1'b1;

					// Advance FIFO
					dmaqueuere <= 1'b1;
					dmareadstate <= STARTDMA;
				end
			end

			STARTDMA: begin
				// Set up read
				m_axi.arvalid <= 1;
				m_axi.araddr <= dmaop_source;
				dmaop_source <= dmaop_source + 32'd16; // Next batch

				// Dummy state, go back to where we were
				dmareadstate <= DMAREADSOURCE;
			end

			DMAREADSOURCE: begin
				if (/*m_axi.arvalid && */m_axi.arready) begin
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					dmaop_count <= dmaop_count - 'd1;
					dmareadstate <= COPYBLOCK;
				end
			end

			COPYBLOCK: begin
				if (m_axi.rvalid  /*&& m_axi.rready*/) begin
					m_axi.rready <= 1'b0;

					// Let write module take care of it
					dmacopywe <= 1'b1;
					dmacopydin <= m_axi.rdata;

					dmareadstate <= COPYLOOP;
				end
			end

			COPYLOOP: begin
				// Set up next read, if there's one
				m_axi.arvalid <= (dmaop_count == 'd0) ? 1'b0 : 1'b1;
				m_axi.araddr <= dmaop_source;
				dmaop_source <= dmaop_source + 32'd16; // Next batch

				// If we're done reading, wait for the writes trailing behind
				dmareadstate <= (dmaop_count == 'd0) ? WAITFORPENDINGWRITES : DMAREADSOURCE;
			end

			WAITFORPENDINGWRITES: begin
				dmainprogress <= ~dmacopyempty;
				dmareadstate <= dmacopyempty ? DETECTCMD : WAITFORPENDINGWRITES;
			end

		endcase
	end
end

// ------------------------------------------------------------------------------------
// DMA logic - Write
// ------------------------------------------------------------------------------------

typedef enum logic [3:0] {
	INIT, STARTCOPYROM, ROMWRITEWORD, ROMWAITWREADY, ROMWAITBREADY,
	WRITEIDLE,
	DETECTFIFO, STARTWRITE, DMAWRITEDEST, DMAWRITELOOP, DMAWRITETRAIL
} dmawritestatetype;
dmawritestatetype dmawritestate = INIT;

logic [127:0] copydata;
logic [31:0] dmaop_target_copy;
logic [31:0] dmaop_count_copy;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		ROMavailable <= 1'b0;
		dmawritestate <= INIT;
	end else begin
		dmacopyre <= 1'b0;
		romre <= 1'b0;

		case (dmawritestate)
			INIT: begin
				// Set up for 128Kbytes of ROM copy starting at 0x0FFE0000
				dmaop_target_copy <= 32'h0FFE0000;
				dmaop_count_copy <= 32'd8192;
				romre <= 1'b0;
				bootROMaddr <= 13'd0;
				dmawritestate <= STARTCOPYROM;
			end

			STARTCOPYROM: begin
				m_axi.awvalid <= 1'b1;
				m_axi.awaddr <= dmaop_target_copy;
				romre <= 1'b1;
				dmaop_target_copy <= dmaop_target_copy + 32'd16; // Next batch
				dmawritestate <= ROMWRITEWORD;
			end

			ROMWRITEWORD: begin
				if (/*m_axi.awvalid &&*/ m_axi.awready) begin
					m_axi.awvalid <= 1'b0;

					m_axi.wvalid <= 1'b1;
					m_axi.wstrb <= 16'hFFFF;
					m_axi.wdata <= bootROMdout;
					m_axi.wlast <= 1'b1;

					bootROMaddr <= bootROMaddr + 13'd1;
					dmaop_count_copy <= dmaop_count_copy - 'd1;

					dmawritestate <= ROMWAITWREADY;
				end
			end

			ROMWAITWREADY: begin
				if (/*m_axi.wvalid &&*/ m_axi.wready) begin
					m_axi.wvalid <= 0;
					m_axi.wstrb <= 16'h0000;
					m_axi.wlast <= 0;
					m_axi.bready <= 1;
					dmawritestate <= ROMWAITBREADY;
				end
			end

			ROMWAITBREADY: begin
				if (m_axi.bvalid /*&& m_axi.bready*/) begin
					m_axi.bready <= 0;
					ROMavailable <= (dmaop_count_copy == 0) ? 1'b1 : 1'b0;
					// Drop into idle state if copy is done, otherwise loop
					dmawritestate <= (dmaop_count_copy == 0) ? WRITEIDLE : STARTCOPYROM;
				end
			end

			WRITEIDLE: begin
				if (writestrobe) begin
					dmaop_target_copy <= dmaop_target;
					dmaop_count_copy <= dmaop_count;
					dmawritestate <= DETECTFIFO;
				end
			end
			
			DETECTFIFO: begin
				if (~dmacopyempty && dmacopyvalid) begin
					copydata <= dmacopydout;
					// Advance FIFO
					dmacopyre <= 1'b1;
					dmawritestate <= STARTWRITE;
				end
			end

			STARTWRITE: begin
				m_axi.awvalid <= 1'b1;
				m_axi.awaddr <= dmaop_target_copy;
				dmaop_target_copy <= dmaop_target_copy + 32'd16; // Next batch
				dmawritestate <= DMAWRITEDEST;
			end

			DMAWRITEDEST: begin
				if (/*m_axi.awvalid &&*/ m_axi.awready) begin
					m_axi.awvalid <= 1'b0;

					m_axi.wvalid <= 1'b1;
					m_axi.wstrb <= 16'hFFFF;
					m_axi.wdata <= copydata;
					m_axi.wlast <= 1'b1;

					dmaop_count_copy <= dmaop_count_copy - 'd1;

					dmawritestate <= DMAWRITELOOP;
				end
			end

			DMAWRITELOOP: begin
				if (/*m_axi.wvalid &&*/ m_axi.wready) begin
					m_axi.wvalid <= 0;

					m_axi.wstrb <= 16'h0000;
					m_axi.wlast <= 0;
					m_axi.bready <= 1;

					dmawritestate <= DMAWRITETRAIL;
				end
			end

			DMAWRITETRAIL: begin
				if (m_axi.bvalid /*&& m_axi.bready*/) begin
					m_axi.bready <= 0;
					// Done with one write, go fetch the next, if any
					dmawritestate <= (dmaop_count_copy == 0) ? WRITEIDLE : DETECTFIFO;
				end
			end
		endcase
	end
end

endmodule
