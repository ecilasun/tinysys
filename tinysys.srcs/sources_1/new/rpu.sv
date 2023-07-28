`timescale 1ns / 1ps

import axi4pkg::*;

// Tile rasterizer

module rastercore(
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi,
	input wire rasterfifoempty,
	input wire [31:0] rasterfifodout,
	output wire rasterfifore,
	input wire rasterfifovalid,
	output wire rasterstate);

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

// Tile coverage mask (byte write enable mask)
logic [15:0] tilecoverage;

// --------------------------------------------------
// Rasterizer input fifo
// --------------------------------------------------

wire rwfull, rwempty, rwvalid;
logic [107:0] rwdin;
wire [107:0] rwdout;
logic rwwen, rwren;
rasterizerinputfifo rworkfifo(
	.full(rwfull),
	.din(rwdin),
	.wr_en(rwwen),
	.empty(rwempty),
	.dout(rwdout),
	.rd_en(rwren),
	.valid(rwvalid),
	.clk(aclk),
	.rst(~aresetn) );

// --------------------------------------------------
// Command processor
// --------------------------------------------------

logic [15:0] x0;
logic [15:0] y0;
logic [15:0] x1;
logic [15:0] y1;
logic [15:0] x2;
logic [15:0] y2;

logic cmdre = 1'b0;
assign rasterfifore = cmdre;

typedef enum logic [4:0] {
	INIT,
	WCMD, DISPATCH,
	SETRASTEROUT,
	SETRASTERCOLOR,
	PUSHVERTEX,
	RASTERQUEUE,
	FINALIZE } rastercmdmodetype;
rastercmdmodetype cmdmode = INIT;

logic [31:0] rastercmd = 'd0;
logic [31:0] rasterbaseaddr;
logic [7:0] rastercolor;

always_ff @(posedge aclk) begin

	cmdre <= 1'b0;
	rwwen <= 1'b0;

	case (cmdmode)
		INIT: begin
			rastercolor <= 8'd0;
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
			case (rastercmd[15:0])
				16'h0000:	cmdmode <= SETRASTEROUT;	// Set base output address for the rasterizer
				16'h0001:	cmdmode <= PUSHVERTEX;		// 16 bit signed x-y pair to follow (TODO: This comes from memory)
				16'h0002:	cmdmode <= SETRASTERCOLOR;	// Set color index / 16bpp color for rasterizer
				16'h0003:	cmdmode <= RASTERQUEUE;		// Push current primitive to rasterizer queue
				default:	cmdmode <= FINALIZE;		// Invalid command, wait one clock and try next
			endcase
		end

		SETRASTEROUT: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				rasterbaseaddr <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		PUSHVERTEX: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				unique case (rastercmd[17:16])
					2'b00: {y0,x0} <= rasterfifodout;
					2'b01: {y1,x1} <= rasterfifodout;
					default: {y2,x2} <= rasterfifodout; // 2'b10
				endcase
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end
		
		RASTERQUEUE: begin
			// Push to rasterizer coarse tiling fifo
			rwdin <= {4'd0,rastercolor,y2,x2,y1,x1,y0,x0};
			rwwen <= ~rwfull;
			cmdmode <= ~rwfull ? FINALIZE : RASTERQUEUE;
		end

		SETRASTERCOLOR: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				//outdata <= {rasterfifodout, rasterfifodout, rasterfifodout, rasterfifodout};
				rastercolor <= rasterfifodout;
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

// --------------------------------------------------
// Rasterizer (mask generator / sweeped / output)
// --------------------------------------------------

logic [127:0] outdata;

logic signed [15:0] rtilex;
logic signed [15:0] rtiley;
logic signed [15:0] rx0;
logic signed [15:0] ry0;
logic signed [15:0] rx1;
logic signed [15:0] ry1;
logic signed [15:0] rx2;
logic signed [15:0] ry2;
logic [7:0] rcolor;
logic [3:0] runused;

logic rena = 1'b0;
wire [15:0] emask01;
wire [15:0] emask12;
wire [15:0] emask20;
wire eready01, eready12, eready20;

edgemaskgen edgeTest01(
    .clk(aclk),
    .rstn(aresetn),
    .tx(rtilex), .ty(rtiley), .v1x(rx0), .v1y(ry0), .v0x(rx1), .v0y(ry1),
    .ena(rena), .ready(eready01),
    .rmask(emask01));

edgemaskgen edgeTest12(
    .clk(aclk),
    .rstn(aresetn),
    .tx(rtilex), .ty(rtiley), .v1x(rx1), .v1y(ry1), .v0x(rx2), .v0y(ry2),
    .ena(rena), .ready(eready12),
    .rmask(emask12));

edgemaskgen edgeTest20(
    .clk(aclk),
    .rstn(aresetn),
    .tx(rtilex), .ty(rtiley), .v1x(rx2), .v1y(ry2), .v0x(rx0), .v0y(ry0),
    .ena(rena), .ready(eready20),
    .rmask(emask20));

typedef enum logic [3:0] {
	RINIT,
	RWCMD,
	SETUPBOUNDS, ENDSETUPBOUNDS, CLIPBOUNDS,
	BEGINSWEEP, RASTERIZETILE, EMITTILE,
	WAITTILEWADDR, WAITTILEWREADY, WAITTILEBREADY,
	NEXTTILE } rasterizermodetype;
rasterizermodetype rastermode = RINIT;

logic signed [15:0] minx;
logic signed [15:0] miny;
logic signed [15:0] maxx;
logic signed [15:0] maxy;

logic [13:0] cx;
logic [13:0] cy;
logic lasttile;

always_ff @(posedge aclk) begin

	rwren <= 1'b0;
	rena <= 1'b0;

	case (rastermode)
		RINIT: begin
			m_axi.awvalid <= 0;
			m_axi.awaddr <= 'd0;
			m_axi.wvalid <= 0;
			m_axi.wstrb <= 16'h0000;
			m_axi.wlast <= 1'b0;
			m_axi.bready <= 1'b0;
			// White color index in default VGA palette
			outdata <= 128'h0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F;
			rastermode <= RWCMD;
		end

		RWCMD: begin
			if (rwvalid && ~rwempty) begin
				{runused,rcolor,ry2,rx2,ry1,rx1,ry0,rx0} <= rwdout;
				// Advance FIFO
				rwren <= 1'b1;
				rastermode <= SETUPBOUNDS;
			end
		end
	   
		SETUPBOUNDS: begin
			// Find min/max bounds
			minx <= rx0 < rx1 ? rx0 : rx1;
			miny <= ry0 < ry1 ? ry0 : ry1;
			maxx <= rx0 >= rx1 ? rx0 : rx1;
			maxy <= ry0 >= ry1 ? ry0 : ry1;
			rastermode <= ENDSETUPBOUNDS;
		end
		
		ENDSETUPBOUNDS: begin
			minx <= minx < rx2 ? minx : rx2;
			miny <= miny < ry2 ? miny : ry2;
			maxx <= maxx >= rx2 ? maxx : rx2;
			maxy <= maxy >= ry2 ? maxy : ry2;
			rastermode <= CLIPBOUNDS;    // TODO: Make this optional via view_clip_enable flag
		end

		CLIPBOUNDS: begin
			// Clamp to viewport
			minx <= minx < 0 ? 0 : minx;
			miny <= miny < 0 ? 0 : miny;
			maxx <= maxx > 319 ? 319 : maxx;
			maxy <= maxy > 239 ? 239 : maxy;
			// Do not attempt to rasterize if bounds are offscreen
			rastermode <= (minx>319 || miny>239 || maxx<0 || maxy<0) ? RWCMD : BEGINSWEEP;
		end

		BEGINSWEEP: begin
			// Set up tile cursor
			cx <= minx[15:2]; // pixel position/4 == tile index
			cy <= miny[15:2];
			// Set up output color (expanded to 128 bits from 8 bits)
			outdata <= {rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor,rcolor};
			rastermode <= RASTERIZETILE;
		end

		RASTERIZETILE: begin
			// Rasterize current tile
			rtilex <= {cx,2'b00}; // Rasterizer requires pixel positions, hence the *4
			rtiley <= {cy,2'b00};
			rena <= 1'b1;
			rastermode <= EMITTILE;
		end

		EMITTILE: begin
			if (eready01 & eready12 & eready20) begin
				// This is the wstrb for a 4x4 tile
				tilecoverage <= emask01 & emask12 & emask20;
				// Push to output fifo if the tile mask isn't zero
				//twe <= |(emask01 & emask12 & emask20);

				// We have 80 tiles horizontal and 60 tiles vertical for a total of 4800 (0x12C0) tiles
				// For each tile, the corresponding memory address is base address (16byte aligned)
				// plus tile index times 16(bytes) in indexed color mode. For 16bit color mode the tile
				// width is reduced by half (i.e. to 2 pixels of 16bits each from 4 pixels of 8bits each)
				m_axi.awaddr <= rasterbaseaddr + {(cx + cy*80), 4'd0};
				// Start memory write for an occupied tile
				m_axi.awvalid <= |(emask01 & emask12 & emask20);

				// Step one line down if we're at the last column
				if (cx >= maxx[15:2]) begin
					cx <= minx[15:2];
					cy <= cy + 1;
				end else begin
					// Next tile on this row
					cx <= cx + 1;
				end

				// High when this is the last tile
				lasttile <= (cy >= maxy[15:2]) && (cx >= maxx[15:2]);

				rastermode <= |(emask01 & emask12 & emask20) ? WAITTILEWADDR : NEXTTILE;
			end
		end

		WAITTILEWADDR: begin
			if (m_axi.awready) begin
				m_axi.awvalid <= 1'b0;

                m_axi.wvalid <= 1'b1;
                m_axi.wstrb <= tilecoverage; // 4x4 tile coverage mask
                m_axi.wdata <= outdata;
                m_axi.wlast <= 1'b1;

				rastermode <= WAITTILEWREADY;
			end
		end

		WAITTILEWREADY: begin
			if (m_axi.wready) begin
				m_axi.wvalid <= 1'b0;
				m_axi.wstrb <= 16'h0000;
				m_axi.wlast <= 1'b0;
				m_axi.bready <= 1;
				rastermode <= WAITTILEBREADY;
			end
		end

		WAITTILEBREADY: begin
			if (m_axi.bvalid) begin // && m_axi.bready
				m_axi.bready <= 0;
				rastermode <= NEXTTILE;
			end
		end

		NEXTTILE: begin
			// Stop if we're at the last tile
			rastermode <= lasttile ? RWCMD : RASTERIZETILE;
		end

	endcase

	if (~aresetn) begin
		rastermode <= RINIT;
	end
    
end

// Rasterizer completely idle
assign rasterstate = ~(rasterfifoempty && rwempty);

endmodule
