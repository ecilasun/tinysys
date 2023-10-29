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

/*logic [31:0] addr = 32'd0;
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
	.a4buscached(m_axi));*/

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
		
		SETRASTERCOLOR: begin
			if (rasterfifovalid && ~rasterfifoempty) begin
				//outdata <= {rasterfifodout, rasterfifodout, rasterfifodout, rasterfifodout};
				rastercolor <= rasterfifodout;
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end

		RASTERQUEUE: begin
			// Push to rasterizer queue
			rwdin <= {rastercolor,y2,x2,y1,x1,y0,x0, rastercmd[15:12]};
			rwwen <= ~rwfull;
			cmdmode <= ~rwfull ? FINALIZE : RASTERQUEUE;
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

typedef enum logic [1:0] {
	RINIT,
	RWCMD,
	RFINALIZE } rasterizermodetype;
rasterizermodetype rastermode = RINIT;

always_ff @(posedge aclk) begin

	rwren <= 1'b0;

	case (rastermode)
		RINIT: begin
			// White color index in default VGA palette
			rastermode <= RWCMD;
		end

		RWCMD: begin
			if (rwvalid && ~rwempty) begin
				//{rcolor,ry2,rx2,ry1,rx1,ry0,rx0,rcommandunused} <= rwdout;

				// Advance FIFO
				rwren <= 1'b1;

				// TODO: Dispatch
				rastermode <= RFINALIZE;
			end
		end

		RFINALIZE: begin
			rastermode <= RWCMD;
		end

	endcase

	if (~aresetn) begin
		rastermode <= RINIT;
	end

end

// Rasterizer completely idle
assign rasterstate = ~(rasterfifoempty && rwempty);

endmodule
