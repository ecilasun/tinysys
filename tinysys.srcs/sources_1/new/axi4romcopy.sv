`timescale 1ns / 1ps

import axi4pkg::*;

module axi4romcopy #(
	parameter int RESETVECTOR = 32'h20000000
) (
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi,
	output wire romReady);

logic ROMavailable = 1'b0;
assign romReady = ROMavailable;

// ------------------------------------------------------------------------------------
// Boot ROM (grouped as 128bit (16 byte) cache line entries)
// ------------------------------------------------------------------------------------

localparam ROMSTART = RESETVECTOR;						// This is also the start of the ROM target
localparam ROMSIZE = 8192;								// Full size of the actual ROM including blank space
localparam ROMIMAGEEND = ROMSTART + (ROMSIZE-1)*16;		// We assume a full ROM image of 8192 entries
logic [12:0] bootROMaddr;
logic [127:0] bootROM[0:ROMSIZE-1];

initial begin
	// Zero-init the entire ROM
	for (int i=0; i<ROMSIZE; ++i)
		bootROM[i] = 128'd0;
	// Replace start section with actual ROM contents
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
// Copy logic
// ------------------------------------------------------------------------------------

typedef enum logic [3:0] {
	INIT,
	STARTCOPYROM, ROMWRITELINE, ROMWAITWREADY, ROMWAITBREADY,
	IDLE } romcopymodetype;
romcopymodetype cmdmode = INIT;

logic [31:0] dmatargetaddr;
logic [31:0] dmatargetend;

// This module does not read from memory
assign m_axi.arvalid = 0;
assign m_axi.rready = 0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		ROMavailable <= 1'b0;
		bootROMaddr <= 13'd0;
		cmdmode <= INIT;
	end else begin

		romre <= 1'b0;

		case (cmdmode)

			INIT: begin
				// Set up for 128Kbytes of ROM copy starting at 0x0FFE0000
				dmatargetaddr <= ROMSTART;
				dmatargetend <= ROMIMAGEEND;

				romre <= 1'b0;
				cmdmode <= STARTCOPYROM;
			end

			STARTCOPYROM: begin
				m_axi.awvalid <= 1'b1;
				m_axi.awaddr <= dmatargetaddr;
				romre <= 1'b1;
				dmatargetaddr <= dmatargetaddr + 32'd16; // Next batch
				cmdmode <= ROMWRITELINE;
			end

			ROMWRITELINE: begin
				if (m_axi.awready) begin // m_axi.awvalid
					m_axi.awvalid <= 1'b0;

					m_axi.wvalid <= 1'b1;
					m_axi.wstrb <= 16'hFFFF;
					m_axi.wdata <= bootROMdout;
					m_axi.wlast <= 1'b1;

					bootROMaddr <= bootROMaddr + 13'd1;

					cmdmode <= ROMWAITWREADY;
				end
			end

			ROMWAITWREADY: begin
				if (m_axi.wready) begin // && m_axi.wvalid
					m_axi.wvalid <= 0;
					m_axi.wstrb <= 16'h0000;
					m_axi.wlast <= 0;
					m_axi.bready <= 1;
					cmdmode <= ROMWAITBREADY;
				end
			end

			ROMWAITBREADY: begin
				if (m_axi.bvalid) begin // && m_axi.bready
					m_axi.bready <= 0;
					ROMavailable <= (dmatargetaddr == dmatargetend) ? 1'b1 : 1'b0;
					// Drop into idle state if ROM copy is done, otherwise loop
					cmdmode <= (dmatargetaddr == dmatargetend) ? IDLE : STARTCOPYROM;
				end
			end
			
			IDLE: begin
                // TODO: Is there a way to power this module off entirely?
				cmdmode <= IDLE;
			end

		endcase
	end
end

endmodule
