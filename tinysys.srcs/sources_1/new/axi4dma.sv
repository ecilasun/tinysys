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
logic dmainprogress = 1'b0;
assign dmafifore = cmdre;
assign dmabusy = dmainprogress;

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

typedef enum logic [3:0] {
	WCMD, DISPATCH,
	DMASOURCE,
	DMATARGET,
	DMALENGTH,
	STARTDMA, DMAREADSOURCE, COPYLOOP, DMAWRITETARGET, DMAWAITWRITE, DMAWAITBREADY,
	FINALIZE } dmacmdmodetype;
dmacmdmodetype cmdmode = WCMD;

logic [31:0] dmacmd = 'd0;

logic [31:0] dmasourceaddr;
logic [31:0] dmatargetaddr;
logic [31:0] dmatargetend;
logic [31:0] dmablockcount;

logic [127:0] datalatch;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		cmdmode <= WCMD;
	end else begin

		cmdre <= 1'b0;

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
					32'h00000003:	cmdmode <= STARTDMA;	// Start transfer with current setup
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
			
			// ----------------------- DMA OP

			STARTDMA: begin
				// Set up read
				m_axi.arvalid <= 1;
				m_axi.araddr <= dmasourceaddr;

				// Make sure to offset the batch once in this header section
				dmasourceaddr <= dmasourceaddr + 32'd16;

				// Dummy state, go back to where we were
				cmdmode <= DMAREADSOURCE;
			end

			DMAREADSOURCE: begin
				if (m_axi.arready) begin // && m_axi.arvalid
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					cmdmode <= COPYLOOP;
				end
			end

			COPYLOOP: begin
				if (m_axi.rvalid ) begin // && m_axi.rready
					m_axi.rready <= 1'b0;
					// Set up write
					m_axi.awvalid <= 1'b1;
					m_axi.awaddr <= dmatargetaddr;
					datalatch <= m_axi.rdata;
					cmdmode <= DMAWRITETARGET;
				end
			end

			DMAWRITETARGET: begin
				// Ready to write?
				if (m_axi.awready) begin
					m_axi.awvalid <= 1'b0;
					m_axi.wvalid <= 1'b1;
					m_axi.wstrb <= 16'hFFFF;
					m_axi.wdata <= datalatch;
					m_axi.wlast <= 1'b1;
					cmdmode <= DMAWAITWRITE;
				end
			end

			DMAWAITWRITE: begin
				if (m_axi.wready) begin
					m_axi.wvalid <= 0;
					m_axi.wstrb <= 16'h0000;
					m_axi.wlast <= 0;
					m_axi.bready <= 1;
					dmablockcount <= dmablockcount - 'd1;
					cmdmode <= DMAWAITBREADY;
				end
			end

			DMAWAITBREADY: begin
				if (m_axi.bvalid) begin // && m_axi.bready
					m_axi.bready <= 0;

					// Set up next read, if there's one
					m_axi.arvalid <= (dmablockcount == 'd0) ? 1'b0 : 1'b1;
					m_axi.araddr <= dmasourceaddr;

					// Next batch
					dmasourceaddr <= dmasourceaddr + 32'd16;
					dmatargetaddr <= dmatargetaddr + 32'd16;

					// Stop when done
					cmdmode <= (dmablockcount == 'd0) ? FINALIZE : DMAREADSOURCE;
				end
			end

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase
	end
end

endmodule
