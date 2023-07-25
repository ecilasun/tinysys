`timescale 1ns / 1ps

import axi4pkg::*;

// Tile rasterizer

module rasterizer(
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi,
	input wire rasterfifoempty,
	input wire [31:0] rasterfifodout,
	output wire rasterfifore,
	input wire rasterfifovalid);

assign m_axi.arlen = 0;				// one burst
assign m_axi.arsize = SIZE_16_BYTE; // 128bit wide reads (not used yet)
assign m_axi.arburst = BURST_FIXED;

// NOTE: No memory read yet
assign m_axi.arvalid = 0;
assign m_axi.araddr = 0;
assign m_axi.rready = 0;

assign m_axi.awlen = 0;				// one burst
assign m_axi.awsize = SIZE_16_BYTE; // 128bit wide writes
assign m_axi.awburst = BURST_FIXED;

// Tile pixel coordinate of upper left corner
logic [15:0] tileX = 0;
logic [15:0] tileY = 0;

// Current triangle vertices
logic [95:0] vertexdata;

logic rena = 1'b0;
wire [15:0] emask01;
wire [15:0] emask12;
wire [15:0] emask20;
wire eready01, eready12, eready20;

edgemaskgen edgeTest01(
    .clk(aclk),
    .rstn(aresetn),
    .tx(tileX), .ty(tileY), .v1x(vertexdata[15:0]), .v1y(vertexdata[31:16]), .v0x(vertexdata[47:32]), .v0y(vertexdata[63:48]),
    .ena(rena), .ready(eready01),
    .rmask(emask01));

edgemaskgen edgeTest12(
    .clk(aclk),
    .rstn(aresetn),
    .tx(tileX), .ty(tileY), .v1x(vertexdata[47:32]), .v1y(vertexdata[63:48]), .v0x(vertexdata[79:64]), .v0y(vertexdata[95:80]),
    .ena(rena), .ready(eready12),
    .rmask(emask12));

edgemaskgen edgeTest20(
    .clk(aclk),
    .rstn(aresetn),
    .tx(tileX), .ty(tileY), .v1x(vertexdata[79:64]), .v1y(vertexdata[95:80]), .v0x(vertexdata[15:0]), .v0y(vertexdata[31:16]),
    .ena(rena), .ready(eready20),
    .rmask(emask20));

// Tile coverage mask (byte write enable mask)
logic [15:0] tilecoverage;

// --------------------------------------------------
// Command FIFO
// --------------------------------------------------

logic cmdre = 1'b0;
assign rasterfifore = cmdre;

typedef enum logic [4:0] {
	INIT,
	WCMD, DISPATCH,
	SETRASTEROUT,
	SETRASTERCOLOR,
	PUSHVERTEX,
	RASTERIZETILE, WRASTER, TILERASTERDONE, TILERASTERWBACK, TILERASTERWRITEWREADY, TILERASTEREND,
	FINALIZE } rastercmdmodetype;
rastercmdmodetype cmdmode = INIT;

logic [31:0] rastercmd = 'd0;
logic [31:0] outaddr;
logic [127:0] outdata;

always_ff @(posedge aclk) begin

	cmdre <= 1'b0;
	rena <= 1'b0;

	case (cmdmode)
		INIT: begin
			m_axi.awvalid <= 0;
			m_axi.awaddr <= 'd0;
			m_axi.wvalid <= 0;
			m_axi.wstrb <= 16'h0000;
			m_axi.wlast <= 1'b0;
			m_axi.bready <= 1'b0;
			// White color index in default VGA palette
			outdata <= 128'h0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F;
			outaddr <= 32'd0;
			cmdmode <= WCMD;
		end

		WCMD: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				rastercmd <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				// Dispatch cmd
				cmdmode <= DISPATCH;
			end
		end

		DISPATCH: begin
			case (rastercmd)
				32'h00000000:	cmdmode <= SETRASTEROUT;	// Set output address for the raster mask result
				32'h00000001:	cmdmode <= PUSHVERTEX;		// 16 bit signed x-y pair to follow
				32'h00000002:	cmdmode <= RASTERIZETILE;	// 2 sets of 16 bit tile min/max corners to follow. Tile mask written to memory on next clock.
				32'h00000003:   cmdmode <= SETRASTERCOLOR;  // Set color index / 16bpp color for rasterizer
				default:		cmdmode <= FINALIZE;		// Invalid command, wait one clock and try next
			endcase
		end

		SETRASTEROUT: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				outaddr <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		PUSHVERTEX: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				vertexdata <= {vertexdata[79:0], rasterfifodout};
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		RASTERIZETILE: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				{tileY, tileX} <= rasterfifodout;
				rena <= 1'b1;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= WRASTER;
			end
		end

		WRASTER: begin
			if (eready01 & eready12 & eready20) begin
    			tilecoverage <= emask01 & emask12 & emask20;
                // Binary tile overlap test result would be: (|emask01) & (|emask12) & (|emask20)
				cmdmode <= TILERASTERDONE;
			end
		end
		
		TILERASTERDONE: begin
			m_axi.awaddr <= outaddr;
			m_axi.awvalid <= 1'b1;
			cmdmode <= TILERASTERWBACK;
		end

		TILERASTERWBACK: begin
			if (m_axi.awready) begin
				m_axi.awvalid <= 1'b0;

                m_axi.wvalid <= 1'b1;
                m_axi.wstrb <= tilecoverage; // 4x4 tile coverage mask
                m_axi.wdata <= outdata;
                m_axi.wlast <= 1'b1;

				cmdmode <= TILERASTERWRITEWREADY;
			end
		end

		TILERASTERWRITEWREADY: begin
			if (m_axi.wready) begin
				m_axi.wvalid <= 1'b0;
				m_axi.wstrb <= 16'h0000;
				m_axi.wlast <= 1'b0;
				m_axi.bready <= 1;
				cmdmode <= TILERASTEREND;
			end
		end

		TILERASTEREND: begin
			if (m_axi.bvalid) begin // && m_axi.bready
				m_axi.bready <= 0;
				cmdmode <= FINALIZE;
			end
		end

		SETRASTERCOLOR: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				outdata <= {rasterfifodout, rasterfifodout, rasterfifodout, rasterfifodout};
				// Advance FIFO
				cmdre <= 1'b1;
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
