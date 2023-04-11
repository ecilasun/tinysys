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

typedef enum logic [2:0] {
	WCMD, DISPATCH,
	DMASOURCE,
	DMATARGET,
	DMALENGTH,
	DMAKICK,
	FINALIZE } dmacmdmodetype;
dmacmdmodetype cmdmode = WCMD;

logic [31:0] dmacmd = 'd0;

logic [31:0] dmasourceaddr;
logic [31:0] dmatargetaddr;
logic [31:0] dmatargetend;
logic [31:0] dmablockcount;

// This module won't read just yet (TBD)
assign m_axi.arvalid = 0;
assign m_axi.rready = 0;


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
					32'h00000003:	cmdmode <= DMAKICK;		// Start transfer with current setup
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

			DMAKICK: begin
				//dmaqueuedin <= {dmasourceaddr, dmatargetaddr, dmablockcount};
				//dmaqueuewe <= 1'b1;

				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE; // TODO: start dma instead, this will completely ignore the kick
			end
			
			// ----------------------- DMA OP

			/*STARTDMA: begin
				// Set up read
				m_axi.arvalid <= 1;
				m_axi.araddr <= dmaop_source;
				dmaop_source <= dmaop_source + 32'd16; // Next batch

				// Dummy state, go back to where we were
				dmareadstate <= DMAREADSOURCE;
			end

			DMAREADSOURCE: begin
				if (m_axi.arready) begin // && m_axi.arvalid
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					dmaop_count <= dmaop_count - 'd1;
					dmareadstate <= COPYBLOCK;
				end
			end

			COPYBLOCK: begin
				if (m_axi.rvalid) begin // && m_axi.rready
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
			end*/
			
			// ----------------------- RESTART

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase
	end
end

endmodule
