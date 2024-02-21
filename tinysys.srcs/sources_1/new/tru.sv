`timescale 1ns / 1ps

import axi4pkg::*;

// Tile rasterizer

module trianglerasterizer(
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi,
	input wire trufifoempty,
	input wire [31:0] trufifodout,
	output wire trufifore,
	input wire trufifovalid,
	output wire [31:0] trustate);

// --------------------------------------------------
// Raster cache
// --------------------------------------------------

logic [31:0] addr = 32'd0;
logic [127:0] din = 128'd0;
logic [15:0] wstrb = 0;
logic cflush = 1'b0;
logic cinvalidate = 1'b0;
wire wready;

rastercache rcache(
	.aclk(aclk),
	.aresetn(aresetn),
	.addr(addr),
	.din(din),
	.wstrb(wstrb),
	.flush(cflush),
	.invalidate(cinvalidate),
	.wready(wready),
	.a4buscached(m_axi));

// --------------------------------------------------
// Edge function unit fifo
// --------------------------------------------------

wire edgefuncfull, edgefuncempty, edgefuncvalid;
logic [107:0] edgefuncdin;
wire [107:0] edgefuncdout;
logic edgefuncwen, edgefuncren;
rasterizerinputfifo edgefuncfifo(
	.full(edgefuncfull),
	.din(edgefuncdin),
	.wr_en(edgefuncwen),
	.empty(edgefuncempty),
	.dout(edgefuncdout),
	.rd_en(edgefuncren),
	.valid(edgefuncvalid),
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
assign trufifore = cmdre;

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
	edgefuncwen <= 1'b0;

	case (cmdmode)
		INIT: begin
			rastercolor <= 8'd0;
			cmdmode <= WCMD;
		end

		WCMD: begin
			if (trufifovalid && ~trufifoempty) begin
				rastercmd <= trufifodout;
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
			if (trufifovalid && ~trufifoempty) begin
				rasterbaseaddr <= trufifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		PUSHVERTEX: begin
			if (trufifovalid && ~trufifoempty) begin
				unique case (rastercmd[17:16])
					2'b00: {y0,x0} <= trufifodout;
					2'b01: {y1,x1} <= trufifodout;
					default: {y2,x2} <= trufifodout; // 2'b10
				endcase
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end
		
		SETRASTERCOLOR: begin
			if (trufifovalid && ~trufifoempty) begin
				rastercolor <= trufifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		RASTERQUEUE: begin
			// Push to rasterizer queue
			edgefuncdin <= {rastercolor,y2,x2,y1,x1,y0,x0, rastercmd[15:12]};
			edgefuncwen <= ~edgefuncfull;
			cmdmode <= ~edgefuncfull ? FINALIZE : RASTERQUEUE;
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
// Edge function and bounds generator
// --------------------------------------------------

typedef enum logic [2:0] {
	EDGEINIT,
	EDGEWCMD,
	BOUNDSPARTIAL, BOUNDSFINAL,
	EDGEFINALIZE } edgemodetype;
edgemodetype edgemode = EDGEINIT;

// Bounds
logic signed [15:0] rx0;
logic signed [15:0] rx1;
logic signed [15:0] rx2;
logic signed [15:0] ry0;
logic signed [15:0] ry1;
logic signed [15:0] ry2;
logic [7:0] rcolor;
logic [3:0] rcommandunused;

// Bounds
logic signed [15:0] minxA;
logic signed [15:0] minyA;
logic signed [15:0] maxxA;
logic signed [15:0] maxyA;
logic signed [15:0] minx;
logic signed [15:0] miny;
logic signed [15:0] maxx;
logic signed [15:0] maxy;

// Edge functions
wire signed [15:0] A12;
wire signed [15:0] B12;
wire signed [15:0] A20;
wire signed [15:0] B20;
wire signed [15:0] A01;
wire signed [15:0] B01;
wire signed [31:0] W0_row;
wire signed [31:0] W1_row;
wire signed [31:0] W2_row;

edgefunction edge01(.px(minx), .py(miny), .x0(rx1), .y0(ry1), .x1(rx2), .y1(ry2), .A_out(A12), .B_out(B12), .W_out(W0_row));
edgefunction edge12(.px(minx), .py(miny), .x0(rx2), .y0(ry2), .x1(rx0), .y1(ry0), .A_out(A20), .B_out(B20), .W_out(W1_row));
edgefunction edge20(.px(minx), .py(miny), .x0(rx0), .y0(ry0), .x1(rx1), .y1(ry1), .A_out(A01), .B_out(B01), .W_out(W2_row));

// --------------------------------------------------
// Rasterizer input fifo
// --------------------------------------------------

wire rasterworkfull, rasterworkempty, rasterworkvalid;
logic [263:0] rasterworkdin;
wire [263:0] rasterworkdout;
logic rasterworkwen, rasterworkren;
sweepinputfifo rasterworkfifo(
	.full(rasterworkfull),
	.din(rasterworkdin),
	.wr_en(rasterworkwen),
	.empty(rasterworkempty),
	.dout(rasterworkdout),
	.rd_en(rasterworkren),
	.valid(rasterworkvalid),
	.clk(aclk),
	.rst(~aresetn) );

always_ff @(posedge aclk) begin

	edgefuncren <= 1'b0;
	rasterworkwen <= 1'b0;

	case (edgemode)
		EDGEINIT: begin
			edgemode <= EDGEWCMD;
		end

		EDGEWCMD: begin
			if (edgefuncvalid && ~edgefuncempty) begin
				{rcolor,ry2,rx2,ry1,rx1,ry0,rx0,rcommandunused} <= edgefuncdout;
				edgefuncren <= 1'b1;
				edgemode <= BOUNDSPARTIAL;
			end
		end

		BOUNDSPARTIAL: begin
			// Partial bounds
			minxA <= rx0<rx1 ? rx0:rx1;
			minyA <= ry0<ry1 ? ry0:ry1;
			maxxA <= rx0>=rx1 ? rx0:rx1;
			maxyA <= ry0>=ry1 ? ry0:ry1;
			edgemode <= BOUNDSFINAL;
		end

		BOUNDSFINAL: begin
			// Final bounds
			minx <= minxA<rx2 ? minx:rx2;
			miny <= minyA<ry2 ? miny:ry2;
			maxx <= maxxA>=rx2 ? minx:rx2;
			maxy <= maxyA>=ry2 ? miny:ry2;
			edgemode <= EDGEFINALIZE;
		end

		EDGEFINALIZE: begin
			// TODO: Push to sweep rasterizer queue
			rasterworkdin <= {rcolor, minx, miny, maxx, maxy, A12, B12, W0_row, A20, B20, W1_row, A01, B01, W2_row};
			rasterworkwen <= 1'b1;
			edgemode <= EDGEWCMD;
		end

	endcase

	if (~aresetn) begin
		edgemode <= EDGEINIT;
	end

end

// --------------------------------------------------
// Sweep rasterizer
// --------------------------------------------------

typedef enum logic [2:0] {
	RASTERINIT,
	RASTERWCMD,
	RASTERFINALIZE } rastermodetype;
rastermodetype rastermode = RASTERINIT;

logic signed [15:0] sminx;
logic signed [15:0] sminy;
logic signed [15:0] smaxx;
logic signed [15:0] smaxy;
logic signed [15:0] sA12;
logic signed [15:0] sB12;
logic signed [15:0] sA20;
logic signed [15:0] sB20;
logic signed [15:0] sA01;
logic signed [15:0] sB01;
logic signed [31:0] sW0_row;
logic signed [31:0] sW1_row;
logic signed [31:0] sW2_row;
logic [7:0] scolor;

always_ff @(posedge aclk) begin
	rasterworkren <= 1'b0;

	case (rastermode)
		RASTERINIT: begin
			rastermode <= RASTERWCMD;
		end

		RASTERWCMD: begin
			if (rasterworkvalid && ~rasterworkempty) begin
				{scolor, sminx, sminy, smaxx, smaxy, sA12, sB12, sW0_row, sA20, sB20, sW1_row, sA01, sB01, sW2_row} <= rasterworkdout;
				rasterworkren <= 1'b1;
				rastermode <= RASTERFINALIZE;
			end
		end

		RASTERFINALIZE: begin
			// TODO:
			rastermode <= RASTERWCMD;
		end

	endcase

	if (~aresetn) begin
		rastermode <= RASTERINIT;
	end
end

// --------------------------------------------------
// Rasterizer unit idle state
// --------------------------------------------------

assign trustate = {31'd0, ~(trufifoempty && edgefuncempty)};

endmodule
