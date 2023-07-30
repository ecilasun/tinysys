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

// --------------------------------------------------
// Raster cache
// --------------------------------------------------

logic [31:0] addr = 32'd0;
logic [127:0] din = 128'd0;
logic [15:0] wstrb = 0;
logic ren = 1'b0;
logic cflush = 1'b0;
wire [127:0] dout;
wire rready, wready;

rastercache rcache(
	.aclk(aclk),
	.aresetn(aresetn),
	.addr(addr),
	.din(din),
	.dout(dout),
	.wstrb(wstrb),
	.ren(ren),
	.flush(cflush),
	.rready(rready),
	.wready(wready),
	.a4buscached(m_axi));

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

typedef enum logic [2:0] {
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
			// Command layout:
			// [31:16] optional parameter
			// [15:12] variant
			// [11:0]  command
			case (rastercmd[11:0])
				12'h000:	cmdmode <= SETRASTEROUT;	// Set base output address for the rasterizer
				12'h001:	cmdmode <= PUSHVERTEX;		// 16 bit signed x-y pair to follow (TODO: This comes from memory)
				12'h002:	cmdmode <= SETRASTERCOLOR;	// Set color index / 16bpp color for rasterizer
				12'h003:	cmdmode <= RASTERQUEUE;		// Push current primitive to rasterizer queue (optional variant to control cache)
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
			rwdin <= {rastercolor,y2,x2,y1,x1,y0,x0, rastercmd[15:12]};
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
logic [3:0] rcommand;

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
	WRITEBACKTILE,
	NEXTTILE,
	CACHESTROBE} rasterizermodetype;
rasterizermodetype rastermode = RINIT;

logic signed [15:0] minx;
logic signed [15:0] miny;
logic signed [15:0] maxx;
logic signed [15:0] maxy;

logic [13:0] cx;
logic [13:0] cy;
logic lasttile;

// Tile coverage mask (byte write enable mask)
logic [15:0] tilecoverage;

// Pending writes
logic [31:0] raddr;
logic pendingwrite;

always_ff @(posedge aclk) begin

	rwren <= 1'b0;
	rena <= 1'b0;
	wstrb <= 16'h0000;
	cflush <= 1'b0;

	if (wready) pendingwrite <= 1'b0;

	case (rastermode)
		RINIT: begin
			// White color index in default VGA palette
			outdata <= 128'h0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F;
			pendingwrite <= 1'b0;
			rastermode <= RWCMD;
		end

		RWCMD: begin
			if (rwvalid && ~rwempty) begin
				// TODO: If rcommand != 4'b0000 we need to flush the data cache
				{rcolor,ry2,rx2,ry1,rx1,ry0,rx0,rcommand} <= rwdout;
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
			rastermode <= (rcommand == 4'h1) ? CACHESTROBE : ENDSETUPBOUNDS;
		end
		
		CACHESTROBE: begin
			if (~pendingwrite) begin
				cflush <= 1'b1;
				pendingwrite <= 1'b1;
				rastermode <= RWCMD;
			end else begin
				rastermode <= CACHESTROBE;
			end
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
				raddr <= rasterbaseaddr + {(cx + cy*80), 4'd0};

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

				rastermode <= |(emask01 & emask12 & emask20) ? WRITEBACKTILE : NEXTTILE;
			end
		end

		WRITEBACKTILE: begin
			if (~pendingwrite) begin
				// Write the 4x4 tile using coverage mask as byte strobe for transparent writes
				addr <= raddr;
				wstrb <= tilecoverage;
				din <= outdata;
				pendingwrite <= 1'b1;
				rastermode <= NEXTTILE;
			end else begin
				rastermode <= WRITEBACKTILE;
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
