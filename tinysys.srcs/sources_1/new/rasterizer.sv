`timescale 1ns / 1ps

import axi4pkg::*;

module coarsetest(
	input wire aclk,
	input wire aresetn,
	input wire signed [15:0] pX,
	input wire signed [15:0] pY,
	input wire signed [15:0] x0,
	input wire signed [15:0] y0,
	input wire signed [15:0] x1,
	input wire signed [15:0] y1,
	output wire outmask );

logic signed [31:0] partialA;
logic signed [31:0] partialB;
logic signed [31:0] outval;

wire signed [15:0] A = (pY-y0);
wire signed [15:0] B = (pX-x0);
wire signed [15:0] dx = (x0-x1);
wire signed [15:0] dy = (y1-y0);

// Test line (x0,y0)-(x1,y1) against the point (px,py)
always @(posedge aclk) begin
	partialA <= A*dx;
	partialB <= B*dy;
	outval <= partialA + partialB;
end

// We only care about the sign bit for 'inside' test
assign outmask = outval[31];

endmodule

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
assign m_axi.arsize = SIZE_16_BYTE; // 128bit read bus
assign m_axi.arburst = BURST_INCR;

// NOTE: No memory read yet
assign m_axi.arvalid = 0;
assign m_axi.araddr = 0;
assign m_axi.rready = 0;

assign m_axi.awlen = 0;				// one burst
assign m_axi.awsize = SIZE_16_BYTE; // 128bit write bus
assign m_axi.awburst = BURST_INCR;

// Pixel coordinates of tile min/max
logic [15:0] tileMinX = 0;
logic [15:0] tileMinY = 0;
logic [15:0] tileMaxX = 0;
logic [15:0] tileMaxY = 0;

// Current triangle
logic [15:0] x0 = 0;
logic [15:0] y0 = 0;
logic [15:0] x1 = 0;
logic [15:0] y1 = 0;
logic [15:0] x2 = 0;
logic [15:0] y2 = 0;

// Edge vs tile test input and results
logic [15:0] ex0 = 0;
logic [15:0] ey0 = 0;
logic [15:0] ex1 = 0;
logic [15:0] ey1 = 0;
logic [3:0] E0;
logic [3:0] E1;
logic [3:0] E2;
logic [3:0] edgeMask;

// Test 4 corners of an edge
// Shared across all 3 edges (one per clock)
coarsetest tiletestA(
	.aclk(aclk), .aresetn(aresetn),
	.pX(tileMinX), .pY(tileMinY),
	.x0(ex0),.y0(ey0),.x1(ex1),.y1(ey1),.outmask(edgeMask[0]) );
coarsetest tiletestB(
	.aclk(aclk), .aresetn(aresetn),
	.pX(tileMaxX), .pY(tileMinY),
	.x0(ex0),.y0(ey0),.x1(ex1),.y1(ey1),.outmask(edgeMask[1]) );
coarsetest tiletestC(
	.aclk(aclk), .aresetn(aresetn),
	.pX(tileMinX), .pY(tileMaxY),
	.x0(ex0),.y0(ey0),.x1(ex1),.y1(ey1),.outmask(edgeMask[2]) );
coarsetest tiletestD(
	.aclk(aclk), .aresetn(aresetn),
	.pX(tileMaxX), .pY(tileMaxY),
	.x0(ex0),.y0(ey0),.x1(ex1),.y1(ey1),.outmask(edgeMask[3]) );

// Tile overlaps triangle
logic tilemask;

// Tile coverage mask (byte write enable mask)
logic [3:0] tilecoverage;

// --------------------------------------------------
// Command FIFO
// --------------------------------------------------

logic cmdre = 1'b0;
assign rasterfifore = cmdre;

typedef enum logic [4:0] {
	INIT,
	WCMD, DISPATCH,
	SETRASTEROUT,
	SETPRIMITIVE,
	SETVERTEX1,
	SETVERTEX2,
	RASTERTILE, RASTETILERADJ, TILERASTERWAIT0, TILERASTERWAIT1, TILERASTERWAIT2, TILERASTERDONE, TILERASTERWA, TILERASTERWB, TILERASTEREND,
	FINALIZE } rastercmdmodetype;
rastercmdmodetype cmdmode = INIT;

logic [31:0] rastercmd = 'd0;
logic [31:0] rasteraddr;

always_ff @(posedge aclk) begin
	cmdre <= 1'b0;

	case (cmdmode)
		INIT: begin
			m_axi.awvalid <= 0;
			m_axi.awaddr <= 'd0;
			m_axi.wvalid <= 0;
			m_axi.wstrb <= 16'h0000;
			m_axi.wlast <= 1'b0;
			m_axi.bready <= 1'b0;
			m_axi.wdata <= 128'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;
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
				32'h00000001:	cmdmode <= SETPRIMITIVE;	// 3 sets of 16 bit signed x-y pairs to follow
				32'h00000002:	cmdmode <= RASTERTILE;		// 2 sets of 16 bit tile min/max corners to follow. Tile mask written to memory on next clock.
				default:		cmdmode <= FINALIZE;		// Invalid command, wait one clock and try next
			endcase
		end
		
		SETRASTEROUT: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				rasteraddr <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		SETPRIMITIVE: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				{y0,x0} <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= SETVERTEX1;
			end
		end

		SETVERTEX1: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				{y1,x1} <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= SETVERTEX2;
			end
		end

		SETVERTEX2: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				{y2,x2} <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		RASTERTILE: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				{tileMinY, tileMinX} <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= RASTETILERADJ;
			end
		end
		
		RASTETILERADJ: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				{tileMaxY, tileMaxX} <= rasterfifodout;
				// Set up edge 0
				{ey0,ex0} <= {y0,x0};
				{ey1,ex1} <= {y1,x1};
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= TILERASTERWAIT0;
			end
		end
		
		TILERASTERWAIT0: begin
			// Read edge 0 and set up edge 1
			E0 <= edgeMask;
			{ey0,ex0} <= {y1,x1};
			{ey1,ex1} <= {y2,x2};
			cmdmode <= TILERASTERWAIT1;
		end

		TILERASTERWAIT1: begin
			// Read edge 1 and set up edge 2
			E1 <= edgeMask;
			{ey0,ex0} <= {y2,x2};
			{ey1,ex1} <= {y0,x0};
			cmdmode <= TILERASTERWAIT2;
		end

		TILERASTERWAIT2: begin
			// Read edge 2 and finalize
			E2 <= edgeMask;
			cmdmode <= TILERASTERDONE;
		end
		
		TILERASTERDONE: begin
			tilemask <= (|E0) & (|E1) & (|E2);	// This is the tile overlap test result
			tilecoverage <= E0 & E1 & E2;		// This is the byte write enable mask
			m_axi.awvalid <= 1'b1;
			m_axi.awaddr <= rasteraddr;
			cmdmode <= TILERASTERWA;
		end

		TILERASTERWA: begin
			if (m_axi.awready) begin
				m_axi.awvalid <= 1'b0;
				m_axi.wvalid <= 1'b1;
				m_axi.wstrb <= {12'h000, tilecoverage}; // TODO: Byte write mask for the 4x4 tile, for now it's only 4 vertices
				m_axi.wlast <= 1'b1;
				cmdmode <= TILERASTERWB;
			end
		end

		TILERASTERWB: begin
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

		FINALIZE: begin
			cmdmode <= WCMD;
		end

	endcase

	if (~aresetn) begin
		cmdmode <= INIT;
	end
end

endmodule
