`timescale 1ns / 1ps
`default_nettype none

import axi4pkg::*;

module axi4dma(
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi,
	input wire dmafifoempty,
	input wire [31:0] dmafifodout,
	output wire dmafifore,
	input wire dmafifovalid,
	output wire dmabusy);

logic cmdre;
assign dmafifore = cmdre;

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// ------------------------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------------------------

//localparam burstcount = 8'd1;		// Single burst

//assign m_axi.arlen = burstcount - 8'd1;
assign m_axi.arsize = SIZE_16_BYTE; // 128bit read bus
assign m_axi.arburst = BURST_INCR;	// auto address increment for burst

//assign m_axi.awlen = burstcount - 8'd1;
assign m_axi.awsize = SIZE_16_BYTE; // 128bit write bus
assign m_axi.awburst = BURST_INCR;	// auto address increment for burst

// ------------------------------------------------------------------------------------
// Command FIFO
// ------------------------------------------------------------------------------------

typedef enum logic [3:0] {
	INIT,
	WCMD, DISPATCH,
	DMASOURCE, DMATARGET, DMABURST, DMASTART, DMATAG,
	FINALIZE } dmacmdmodetype;
dmacmdmodetype cmdmode;

logic [31:0] dmacmd;
logic [27:0] dmasourceaddr;
logic [31:0] dmatargetaddr;
logic [7:0] dmasingleburstcount;
logic misaligned;

// Work fifo
wire dfull, dempty, dvalid;
logic dwre, dre;
logic [69:0] ddin; // {misaligned,masked,len,src[31:4],dest[31:0]}
wire [69:0] ddout;

dmaworkfifo dmaworkfifoinst(
	.full(dfull),
	.din(ddin),
	.wr_en(dwre),
	.empty(dempty),
	.dout(ddout),
	.rd_en(dre),
	.valid(dvalid),
	.clk(aclk),
	.srst(~delayedresetn) );

always_ff @(posedge aclk) begin
	if (~delayedresetn) begin
		cmdre <= 1'b0;
		dmacmd <= 32'd0;
		dmasourceaddr <= 28'd0;
		dmatargetaddr <= 32'd0;
		misaligned <= 1'b0;
		dmasingleburstcount <= 8'd0;
		dwre <= 1'b0;
		ddin <= 70'd0;
		cmdmode <= INIT;
	end else begin
		cmdre <= 1'b0;
		dwre <= 1'b0;
	
		case (cmdmode)
			INIT: begin
				dmacmd <= 32'd0;
				cmdmode <= WCMD;
			end

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
				case (dmacmd[7:0])
					8'h00:		cmdmode <= DMASOURCE;			// Source address, byte aligned
					8'h01:		cmdmode <= DMATARGET;			// Target address, byte aligned
					8'h02:		cmdmode <= DMABURST;			// Number of 16 byte blocks to copy, minus one, up to a maximum of 256
					8'h03:		cmdmode <= DMASTART;			// Start transfer with current setup
					8'h04:		cmdmode <= DMATAG;				// Dummy command, used to ensure FIFO has something after the copy completes we can wait on
					default:	cmdmode <= FINALIZE;			// Invalid command, wait one clock and try next
				endcase
			end

			DMASOURCE: begin
				if (dmafifovalid && ~dmafifoempty) begin
					dmasourceaddr <= dmafifodout[31:4];
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			DMATARGET: begin
				if (dmafifovalid && ~dmafifoempty) begin
					dmatargetaddr <= dmafifodout;
					misaligned <= dmacmd[9];
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			DMABURST: begin
				if (dmafifovalid && ~dmafifoempty) begin
					// Single burst, size < 256
					dmasingleburstcount <= dmafifodout[7:0];
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			DMASTART: begin
				if (!dfull) begin
					// misaligned, maskmode, burstcount, sourceaddr[31:4], targetaddr[31:0]
					ddin <= {misaligned, dmacmd[8], dmasingleburstcount, dmasourceaddr, dmatargetaddr};
					dwre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end


			DMATAG: begin
				if (dmafifovalid && ~dmafifoempty) begin
					//dmatag <= dmafifodout; // Unused - TAG
					// Used as a marker to wait on from CPU side
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase

	end
end

// ------------------------------------------------------------------------------------
// Address and mask registers
// ------------------------------------------------------------------------------------

logic wmisaligned;
logic wmasked;
logic [27:0] wsourceaddr;
logic [31:0] wtargetaddr;
logic [7:0] wsingleburstcount;

// ------------------------------------------------------------------------------------
// Aligned / Aligned masked packet worker
// ------------------------------------------------------------------------------------

logic [7:0] rburstcursor;
logic [7:0] rburstcursorPrev;
logic [7:0] wburstcursor;
logic [127:0] burstcache[0:255];

initial begin
	for(int i=0;i<256;i++)
		burstcache[i] = 128'd0;
end

logic burstwe;
logic [127:0] burstdin;
always @(posedge aclk) begin
	if (burstwe)
		burstcache[wburstcursor] <= burstdin;
end

logic burstre;
logic [127:0] burstdout;
logic [127:0] burstdoutPrev;
always @(posedge aclk) begin
	if (burstre) begin
		burstdout <= burstcache[rburstcursor];
		burstdoutPrev <= burstdout;//burstcache[rburstcursorPrev];
	end
end

wire lastburst = rburstcursor == wsingleburstcount;
wire firstburst = rburstcursor == 0;
wire midburst = !(lastburst || firstburst);

wire [3:0] targetAlignMask = wtargetaddr[3:0];
wire [6:0] targetAlignByte = {targetAlignMask, 3'd0};
wire [127:0] targetAlignedBytes = targetAlignMask == 4'h0 ? burstdout : {{burstdout, burstdoutPrev}<<targetAlignByte}[255:128];
logic [15:0] leadingMask;
logic [15:0] trailingMask;

// Generate an optional write mask to avoid writing zero bytes to the target, controlled by the wmasked flag
wire [15:0] automask = {
	|targetAlignedBytes[127:120], |targetAlignedBytes[119:112], |targetAlignedBytes[111:104], |targetAlignedBytes[103:96],
	|targetAlignedBytes[95:88], |targetAlignedBytes[87:80], |targetAlignedBytes[79:72], |targetAlignedBytes[71:64],
	|targetAlignedBytes[63:56], |targetAlignedBytes[55:48], |targetAlignedBytes[47:40], |targetAlignedBytes[39:32],
	|targetAlignedBytes[31:24], |targetAlignedBytes[23:16], |targetAlignedBytes[15:8], |targetAlignedBytes[7:0] };

typedef enum logic [3:0] {
	WINIT,
	DMACMD, DMABEGIN,
	WAITREADADDR, READLOOP, SETUPWRITE, WAITWRITEADDR, WRITEBEGIN, WRITELOOP, WRITETRAIL,
	WFINALIZE } workercmdtype;
workercmdtype workmode;

always @(posedge aclk) begin
	if (~delayedresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		burstwe <= 1'b0;
		burstre <= 1'b0;
		rburstcursor <= 8'd0;
		rburstcursorPrev <= 8'd0;
		wburstcursor <= 8'd0;
		wmasked <= 1'b0;
		wmisaligned <= 1'b0;
		wsourceaddr <= 28'd0;
		wtargetaddr <= 32'd0;
		wsingleburstcount <= 8'd0;
		leadingMask <= 16'h0000;
		trailingMask <= 16'h0000;
		dre <= 1'b0;
		workmode <= WINIT;
	end else begin
		burstwe <= 1'b0;
		burstre <= 1'b0;
		dre <= 1'b0;

		case (workmode)
			WINIT: begin
				// TODO:
				workmode <= DMACMD;
			end

			DMACMD: begin
				if (dvalid && ~dempty) begin
					{wmisaligned, wmasked, wsingleburstcount, wsourceaddr, wtargetaddr} <= ddout;
					workmode <= DMABEGIN;
				end
			end
			
			DMABEGIN: begin
				case (1'b1)
					wmisaligned: begin
						leadingMask <= {{32'hFFFF0000}<<targetAlignMask}[31:16];
						trailingMask <= {{32'h0000FFFF}<<targetAlignMask}[31:16];
					end
					default: begin
						leadingMask <= 16'hFFFF;
						trailingMask <=  16'hFFFF;
					end
				endcase

				// Auto byte mask enable/disable flag
				// NOTE: Not to complicate hardware, we make sure to set this to burstcount-8'd1 in software
				m_axi.arlen <= wsingleburstcount;
				m_axi.arvalid <= 1;
				m_axi.araddr <= {wsourceaddr,4'h0}; 
				workmode <= WAITREADADDR;
			end

			WAITREADADDR: begin
				if (m_axi.arready) begin
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					wburstcursor <= 8'hFF;
					workmode <= READLOOP;
				end
			end

			READLOOP: begin
				if (m_axi.rvalid) begin
					m_axi.rready <= ~m_axi.rlast;
					burstwe <= 1'b1;
					burstdin <= m_axi.rdata;
					wburstcursor <= wburstcursor + 8'd1;
					workmode <= ~m_axi.rlast ? READLOOP : SETUPWRITE;
				end
			end

			SETUPWRITE: begin
				// TODO: We need to write an extra 16 bytes at the end,
				// masked to clip extra bits due to misalignment. 
				//m_axi.awlen <= misaligned ? wsingleburstcount+8'd1 : wsingleburstcount;
				m_axi.awlen <= wsingleburstcount;
				m_axi.awvalid <= 1;
				m_axi.awaddr <= wtargetaddr;

				// First read from the cache
				rburstcursor <= 8'h00;
				rburstcursorPrev <= 8'hFF;
				burstre <= 1'b1;

				workmode <= WAITWRITEADDR;
			end

			WAITWRITEADDR: begin
				if (m_axi.awready) begin
					m_axi.awvalid <= 1'b0;
					m_axi.bready <= 1'b1;
					workmode <= WRITEBEGIN;
				end
			end

			WRITEBEGIN: begin
				case (2'b11)
					{wmasked, firstburst}:	m_axi.wstrb <= automask & leadingMask;
					{wmasked, lastburst}:	m_axi.wstrb <= automask & trailingMask;
					{wmasked, midburst}:	m_axi.wstrb <= automask;
					{!wmasked, firstburst}:	m_axi.wstrb <= leadingMask;
					{!wmasked, lastburst}:	m_axi.wstrb <= trailingMask;
					{!wmasked, midburst}:	m_axi.wstrb <= 16'hFFFF;
					default:				m_axi.wstrb <= 16'hFFFF;
				endcase

				m_axi.wdata <= targetAlignedBytes;
				m_axi.wvalid <= 1'b1;
				m_axi.wlast <= lastburst;

				// Set up for next read
				rburstcursor <= rburstcursor + 8'h01;
				rburstcursorPrev <= rburstcursorPrev + 8'h01;
				burstre <= 1'b1;

				workmode <= WRITELOOP;
			end

			WRITELOOP: begin
				if (m_axi.wready) begin
					m_axi.wvalid <= 1'b0;
					m_axi.wstrb <= 16'h0000;
					workmode <= m_axi.wlast ? WRITETRAIL : WRITEBEGIN;
				end
			end

			WRITETRAIL: begin
				if (m_axi.bvalid) begin
					// NOTE: We retire the pending FIFO output once we're done with the packet copy.
					// This ensures that the CPU can track DMA packet completion status
					// instead of just the command read.
					dre <= 1'b1;
					m_axi.bready <= 1'b0;
					workmode <= WFINALIZE;
				end
			end

			WFINALIZE: begin
				workmode <= DMACMD;
			end

		endcase		
	end
end

// ------------------------------------------------------------------------------------
// Syncronization mechanism
// ------------------------------------------------------------------------------------

// Add one extra dummy item after the last DMA command (i.e. the TAG command)
// Wait for command FIFO to be drained including the TAG, as well the work fifo
assign dmabusy = (~dmafifoempty) && (~dempty);

endmodule
