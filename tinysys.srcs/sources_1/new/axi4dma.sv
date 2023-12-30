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
	output wire dmabusy);

logic cmdre = 1'b0;
assign dmafifore = cmdre;

// Syncronization mechanism:
// Add one extra dummy item after the last DMA command
// Wait for FIFO to become empty
// That essantially means we've DMAd everyting, turned
// around and also read the last dummy command.
assign dmabusy = ~dmafifoempty;

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
	DMASOURCE,
	DMATARGET,
	DMABURST,
	DMASTART,
	DMATAG,
	WAITREADADDR, READLOOP, SETUPWRITE, WAITWRITEADDR, WRITEBEGIN, WRITELOOP, WRITETRAIL,
	FINALIZE } dmacmdmodetype;
dmacmdmodetype cmdmode = INIT;

logic [31:0] dmacmd;
logic [31:0] dmasourceaddr;
logic [31:0] dmatargetaddr;
logic [7:0] dmasingleburstcount = 8'd0;
logic [7:0] burstcursor = 8'd0;
logic [127:0] burstcache[0:255];

initial begin
	for(int i=0;i<256;i++)
		burstcache[i] = 128'd0;
end

logic burstwe;
logic [127:0] burstdin;
always @(posedge aclk) begin
	if (burstwe)
		burstcache[burstcursor] <= burstdin;
end
wire [127:0] burstdout = burstcache[burstcursor]; 

always_ff @(posedge aclk) begin

	cmdre <= 1'b0;
	burstwe <= 1'b0;

	case (cmdmode)

		INIT: begin
			m_axi.awvalid <= 0;
			m_axi.wvalid <= 0;
			m_axi.wstrb <= 16'h0000;
			m_axi.wlast <= 0;
			m_axi.bready <= 0;
			m_axi.arvalid <= 0;
			m_axi.rready <= 0;
			burstwe <= 1'b0;
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
			case (dmacmd)
				32'h00000000:	cmdmode <= DMASOURCE;		// Source address, byte aligned
				32'h00000001:	cmdmode <= DMATARGET;		// Target address, byte aligned
				32'h00000002:	cmdmode <= DMABURST;		// Number of 16 byte blocks to copy, minus one, up to a maximum of 256
				32'h00000003:	cmdmode <= DMASTART;		// Start transfer with current setup
				32'h00000004:	cmdmode <= DMATAG;			// Dummy command, used to ensure FIFO has something after the copy completes we can wait on
				default:		cmdmode <= FINALIZE;		// Invalid command, wait one clock and try next
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

		DMABURST: begin
			if (dmafifovalid && ~dmafifoempty) begin
				// Single burst, size < 256
				dmasingleburstcount <= dmafifodout[7:0];
				burstcursor <= 8'hFF;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		DMASTART: begin
			// NOTE: Not to complicate hardware, we make sure to set this to burstcount-8'd1 in software
			m_axi.arlen <= dmasingleburstcount;
			m_axi.arvalid <= 1;
			m_axi.araddr <= dmasourceaddr; 
			cmdmode <= WAITREADADDR;
		end

		DMATAG: begin
			if (dmafifovalid && ~dmafifoempty) begin
				//dmatag <= dmafifodout; // Unused - TAG
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		// ----------------------- DMA OP

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
				burstwe <= 1'b1;
				burstdin <= m_axi.rdata;
				burstcursor <= burstcursor + 8'd1;
				cmdmode <= ~m_axi.rlast ? READLOOP : SETUPWRITE;
			end
		end

		SETUPWRITE: begin
			burstcursor <= 8'd0;
			m_axi.awlen <= dmasingleburstcount;
			m_axi.awvalid <= 1;
			m_axi.awaddr <= dmatargetaddr;
			cmdmode <= WAITWRITEADDR;
		end
		
		WAITWRITEADDR: begin
			if (m_axi.awready) begin
				m_axi.awvalid <= 1'b0;
				m_axi.bready <= 1'b1;
				cmdmode <= WRITEBEGIN;
			end
		end
		
		WRITEBEGIN: begin
			m_axi.wdata <= burstdout;
			m_axi.wstrb <= 16'hFFFF;
			m_axi.wvalid <= 1'b1;
			m_axi.wlast <= (burstcursor==dmasingleburstcount) ? 1'b1 : 1'b0;
			cmdmode <= WRITELOOP;
		end

		WRITELOOP: begin
			if (m_axi.wready) begin
				burstcursor <= burstcursor + 8'd1;
				m_axi.wvalid <= 1'b0;
				m_axi.wstrb <= 16'h0000;
				cmdmode <= m_axi.wlast ? WRITETRAIL : WRITEBEGIN;
			end
		end

		WRITETRAIL: begin
			if (m_axi.bvalid) begin
				m_axi.bready <= 1'b0;
				cmdmode <= FINALIZE;
			end
		end

		FINALIZE: begin
			cmdmode <= WCMD;
		end

	endcase

	if (~aresetn) begin
		cmdmode <= INIT;
	end
end

endmodule
