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

logic [127:0] din = 128'd0;
logic [31:0] addr = 32'd0;
logic [15:0] wstrb = 16'd0;
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
logic [119:0] edgefuncdin;
wire [119:0] edgefuncdout;
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

logic [17:0] x0;
logic [17:0] y0;
logic [17:0] x1;
logic [17:0] y1;
logic [17:0] x2;
logic [17:0] y2;

logic cmdre = 1'b0;
assign trufifore = cmdre;

typedef enum logic [3:0] {
	INIT,
	WCMD, DISPATCH,
	SETRASTEROUT,
	SETRASTERCOLOR,
	PUSHVERTEX,
	RASTERQUEUE,
	WCACHE,
	FINALIZE } rastercmdmodetype;
rastercmdmodetype cmdmode = INIT;

logic [31:0] rastercmd = 'd0;
logic [31:0] rasterbaseaddr;
logic [7:0] rastercolor;

always_ff @(posedge aclk) begin
	cflush <= 1'b0;
	cinvalidate <= 1'b0;
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
					2'b00: {y0,x0} <= {trufifodout[31], trufifodout[31], trufifodout[31:16], trufifodout[15], trufifodout[15], trufifodout[15:0]};
					2'b01: {y1,x1} <= {trufifodout[31], trufifodout[31], trufifodout[31:16], trufifodout[15], trufifodout[15], trufifodout[15:0]};
					default: {y2,x2} <= {trufifodout[31], trufifodout[31], trufifodout[31:16], trufifodout[15], trufifodout[15], trufifodout[15:0]}; // 2'b10
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
			case (rastercmd[15:12])
				4'd0: begin
					// Push to rasterizer queue
					edgefuncdin <= {rastercolor,y2,x2,y1,x1,y0,x0, rastercmd[15:12]};
					edgefuncwen <= ~edgefuncfull;
					cmdmode <= ~edgefuncfull ? FINALIZE : RASTERQUEUE;
				end
				4'd1: begin
					cflush <= 1'b1;
					cmdmode <= WCACHE;
				end
				4'd2: begin
					cinvalidate <= 1'b1;
					cmdmode <= WCACHE;
				end
			endcase
		end

		WCACHE: begin
			cmdmode <= wready ? FINALIZE : WCACHE;
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

typedef enum logic [3:0] {
	EDGEINIT,
	EDGEWCMD,
	BOUNDSSTEPA, BOUNDSSTEPB,
	EDGEWAITA, EDGEWAITB, EDGEWAITC, EDGEWAITD,
	CLIPTOVIEWPORT, REJECTBYVIEWPORT, EDGEFINALIZE } edgemodetype;
edgemodetype edgemode = EDGEINIT;

// Bounds
logic signed [17:0] rx0;
logic signed [17:0] rx1;
logic signed [17:0] rx2;
logic signed [17:0] ry0;
logic signed [17:0] ry1;
logic signed [17:0] ry2;
logic [7:0] rcolor;
logic [3:0] rcommandunused;

// Bounds
logic signed [17:0] minxA;
logic signed [17:0] minyA;
logic signed [17:0] maxxA;
logic signed [17:0] maxyA;
logic signed [17:0] minx;
logic signed [17:0] miny;
logic signed [17:0] maxx;
logic signed [17:0] maxy;

// Edge functions
logic signed [17:0] A12;
logic signed [17:0] B12;
logic signed [17:0] C12;
logic signed [17:0] A20;
logic signed [17:0] B20;
logic signed [17:0] C20;
logic signed [17:0] A01;
logic signed [17:0] B01;
logic signed [17:0] C01;
logic signed [17:0] W0_row;
logic signed [17:0] W1_row;
logic signed [17:0] W2_row;

// --------------------------------------------------
// Rasterizer input fifo
// --------------------------------------------------

wire rasterworkfull, rasterworkempty, rasterworkvalid;
logic [241:0] rasterworkdin;
wire [241:0] rasterworkdout;
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
				edgemode <= BOUNDSSTEPA;
			end
		end

		BOUNDSSTEPA: begin
			// Partial bounds
			minxA <= rx0<rx1 ? rx0:rx1;
			minyA <= ry0<ry1 ? ry0:ry1;
			maxxA <= rx0>rx1 ? rx0:rx1;
			maxyA <= ry0>ry1 ? ry0:ry1;

			edgemode <= BOUNDSSTEPB;
		end

		BOUNDSSTEPB: begin
			// Final bounds
			minx <= minxA<rx2 ? minxA:rx2;
			miny <= minyA<ry2 ? minyA:ry2;
			maxx <= maxxA>rx2 ? maxxA:rx2;
			maxy <= maxyA>ry2 ? maxyA:ry2;
			edgemode <= REJECTBYVIEWPORT;
		end

		REJECTBYVIEWPORT: begin
			if (minx>=319 || miny>=239 || maxx<0 || maxy<0)
				edgemode <= EDGEWCMD;
			else
				edgemode <= CLIPTOVIEWPORT;
		end

		CLIPTOVIEWPORT: begin
			minx <= minx < 0 ? 0 : minx;
			maxx <= maxx > 319 ? 319 : maxx;
			miny <= miny < 0 ? 0 : miny;
			maxy <= maxy > 239 ? 239 : maxy;

			edgemode <= EDGEWAITA;
		end

		EDGEWAITA: begin
			A12 <= ry1 - ry2;
			B12 <= rx2 - rx1;
			C12 <= rx0*ry1 - ry0*rx0;

			edgemode <= EDGEWAITB;
		end

		EDGEWAITB: begin
			W0_row <= A12*minx + B12*miny + C12;

			A20 <= ry2 - ry0;
			B20 <= rx0 - rx2;
			C20 <= rx0*ry1 - ry0*rx0;

			edgemode <= EDGEWAITC;
		end

		EDGEWAITC: begin
			W1_row <= A20*minx + B20*miny + C20;

			A01 <= ry0 - ry1;
			B01 <= rx1 - rx0;
			C01 <= rx0*ry1 - ry0*rx0;

			edgemode <= EDGEWAITD;
		end
		
		EDGEWAITD: begin
			W2_row <= A01*minx + B01*miny + C01;

			edgemode <= EDGEFINALIZE;
		end

		EDGEFINALIZE: begin
			// TODO: Reject backfacing before pushing the work
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

typedef enum logic [3:0] {
	RASTERINIT,
	RASTERWCMD,
	RASTERSETUP, SWEEPROW, EDGETEST, WRITEROW, WRITEWAIT, STEPTILE, SWEEPCOLUMN, STARTNEXTROW,
	RASTERFINALIZE } rastermodetype;
rastermodetype rastermode = RASTERINIT;
rastermodetype postwait = RASTERINIT;

logic [7:0] scolor;
logic signed [17:0] sminx;
logic signed [17:0] sminy;
logic signed [17:0] smaxx;
logic signed [17:0] smaxy;
logic signed [17:0] sA12;
logic signed [17:0] sB12;
logic signed [17:0] sA20;
logic signed [17:0] sB20;
logic signed [17:0] sA01;
logic signed [17:0] sB01;
logic signed [17:0] sW0_row;
logic signed [17:0] sW1_row;
logic signed [17:0] sW2_row;

// Sweep temps
logic signed [13:0] scx;
logic signed [13:0] scendx;
logic signed [17:0] scy;
logic signed [17:0] scendy;
logic signed [17:0] sw0;
logic signed [17:0] sw1;
logic signed [17:0] sw2;

logic [15:0] sv;
logic [17:0] E0;
logic [17:0] E1;
logic [17:0] E2;

logic [3:0] edgecnt;
always_ff @(posedge aclk) begin
	rasterworkren <= 1'b0;
	wstrb <= 16'd0;

	case (rastermode)
		RASTERINIT: begin
			rastermode <= RASTERWCMD;
		end

		RASTERWCMD: begin
			if (rasterworkvalid && ~rasterworkempty) begin
				{scolor, sminx, sminy, smaxx, smaxy, sA12, sB12, sW0_row, sA20, sB20, sW1_row, sA01, sB01, sW2_row} <= rasterworkdout;
				rasterworkren <= 1'b1;
				rastermode <= RASTERSETUP;
			end
		end

		RASTERSETUP: begin
			// Using tiles of 16x1 in size for easy mask generation
			scx <= sminx[17:4];
			scy <= sminy;
			scendx <= smaxx[17:4];
			scendy <= smaxy;
			sw0 <= sW0_row;
			sw1 <= sW1_row;
			sw2 <= sW1_row;
			rastermode <= SWEEPROW;
		end

		SWEEPROW: begin
			addr <= rasterbaseaddr + {scx + scy*20, 4'h0}; // base + blockaddress*16
			din <= {scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor, scolor};
			E0 <= sw0;
			E1 <= sw1;
			E2 <= sw2;
			sv <= 16'h0000;
			edgecnt <= 4'd0;
			rastermode <= WRITEROW;//EDGETEST;
		end

		/*EDGETEST: begin
			E0 <= E0 + sA12;
			E1 <= E1 + sA20;
			E2 <= E2 + sA01;
			sv <= {sv[14:0], E0[17] & E1[17] & E2[17]};
			edgecnt <= edgecnt + 4'd1;
			rastermode <= edgecnt==15 ? WRITEROW : EDGETEST;
		end*/

		WRITEROW: begin
			// We can now use the sign bits of above calculation as our write mask and emit the 16x1 tile's color
			wstrb <= 16'hFFFF;//sv;

			// Next 16 pixel block
			scx <= scx + 14'd1;

			// Are we at the end?
			if (scx == scendx) begin
				// Rewind to start x
				scx <= sminx[17:4];
			end
			rastermode <= WRITEWAIT;
			postwait <= (scx == scendx) ? SWEEPCOLUMN : SWEEPROW;
		end

		WRITEWAIT: begin
			rastermode <= wready ? STEPTILE : WRITEWAIT;
		end

		STEPTILE: begin
			// Skip to the next 16x1 pixel tile
			sw0 <= sw0 + 16*sA12;
			sw1 <= sw1 + 16*sA20;
			sw2 <= sw2 + 16*sA01;
			rastermode <= postwait; // Either continue on current line or go to the next line
		end

		SWEEPCOLUMN: begin
			sW0_row <= + sB12;
			sW1_row <= + sB20;
			sW2_row <= + sB01;
			rastermode <= STARTNEXTROW;
		end

		STARTNEXTROW: begin
			// Step one line down
			scy <= scy + 18'd1;
			// Go to start of next row
			sw0 <= sW0_row;
			sw1 <= sW1_row;
			sw2 <= sW2_row;
			rastermode <= (scy == scendy) ? RASTERFINALIZE : SWEEPROW;
		end

		RASTERFINALIZE: begin
			rastermode <= RASTERWCMD;
		end

	endcase

	if (~aresetn) begin
		addr <= 32'd0;
		wstrb <= 16'd0;
		rastermode <= RASTERINIT;
	end
end

// --------------------------------------------------
// Rasterizer unit busy state
// --------------------------------------------------

assign trustate = {31'd0, ~(trufifoempty && edgefuncempty && rasterworkempty)};

endmodule
