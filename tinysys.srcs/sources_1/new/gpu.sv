`timescale 1ns / 1ps

import axi4pkg::*;

module gpucore(
	input wire aclk,
	input wire clk25,
	input wire clk125,
	input wire aresetn,
	axi4if.master m_axi,
	gpuwires.def gpuvideoout,
	input wire gpufifoempty,
	input wire [31:0] gpufifodout,
	output wire gpufifore,
	input wire gpufifovalid,
	output wire [31:0] vblankcount);

// A simple graphics unit with the following features:
// - Two video output modes (320x240 and 640x480)
// - Indexed color output (8bpp)
// - Framebuffer scan-out from any cache aligned memory location
// - Frame counter support for vsync implementations
// - Memory mapped command buffer interface

// TODO:
// - Allow command data to be read from any cache aligned memory location in 16 byte bursts
// - Add sprite caching support at end of each scanline, and sprite compositing
// - Add rasterization support

// --------------------------------------------------
// Common
// --------------------------------------------------

wire hsync, vsync, blank;
wire [10:0] video_x;
wire [10:0] video_y;

logic hsync_d, vsync_d, blank_d;

wire [1:0] out_tmds_red;
wire [1:0] out_tmds_green;
wire [1:0] out_tmds_blue;
wire [1:0] out_tmds_clk;

logic cmdre = 1'b0;
assign gpufifore = cmdre;

// --------------------------------------------------
// Setup
// --------------------------------------------------

logic [7:0] burstlen = 'd20;	// 20 reads for 320*240, 40 reads for 640*480 in paletted mode (each read is 128bits)
logic scanmode = 1'b0;			// 320 pixel mode (640 when high)

// --------------------------------------------------
// Scanline cache and output address selection
// --------------------------------------------------

logic [127:0] scanlinecache [0:63]; // 64 blocks of 16 pixels worth of scanline cache (20 used in index color 320*240 mode)

// localindex is the 16 pixel pixel index counter, which can move either at 1:2 (pixel doubling) or 1:1 the scan rate (no doubling)
logic [3:0] localindex;
// cacheindex is the 16 pixel wide block index across a scanline
logic [5:0] cacheindex;

always_comb begin
	localindex = scanmode ? video_x[3:0] : video_x[4:1];
	cacheindex = scanmode ? video_x[9:4] : video_x[10:5];
end

logic [7:0] palettewa;
logic palettewe = 1'b0;
logic [7:0] palettera;
logic [23:0] palettedin = 24'h000000;

// Generate palette read address from current pixel's color index
always_ff @(posedge clk25) begin
	if (~aresetn) begin
		// 
	end else begin
		unique case (localindex)
			4'b0000: palettera <= scanlinecache[cacheindex][7 : 0];
			4'b0001: palettera <= scanlinecache[cacheindex][15 : 8];
			4'b0010: palettera <= scanlinecache[cacheindex][23 : 16];
			4'b0011: palettera <= scanlinecache[cacheindex][31 : 24];
			4'b0100: palettera <= scanlinecache[cacheindex][39 : 32];
			4'b0101: palettera <= scanlinecache[cacheindex][47 : 40];
			4'b0110: palettera <= scanlinecache[cacheindex][55 : 48];
			4'b0111: palettera <= scanlinecache[cacheindex][63 : 56];
			4'b1000: palettera <= scanlinecache[cacheindex][71 : 64];
			4'b1001: palettera <= scanlinecache[cacheindex][79 : 72];
			4'b1010: palettera <= scanlinecache[cacheindex][87 : 80];
			4'b1011: palettera <= scanlinecache[cacheindex][95 : 88];
			4'b1100: palettera <= scanlinecache[cacheindex][103 : 96];
			4'b1101: palettera <= scanlinecache[cacheindex][111 : 104];
			4'b1110: palettera <= scanlinecache[cacheindex][119 : 112];
			4'b1111: palettera <= scanlinecache[cacheindex][127 : 120];
		endcase
	end
end

// --------------------------------------------------
// Palette RAM
// --------------------------------------------------

logic [23:0] paletteentries[0:255];

initial begin
	$readmemh("colorpalette.mem", paletteentries);
end

// Write port
always @(posedge aclk) begin // Tied to GPU clock
	if (~aresetn) begin
		//
	end else begin
		if (palettewe)
			paletteentries[palettewa] <= palettedin;
	end
end

// Read port
logic [23:0] paletteout;
always @(posedge clk25) begin // Tied to GPU clock
	if (~aresetn) begin
		paletteout <= 24'd0;
	end else begin
		paletteout <= paletteentries[palettera];
	end
end

// --------------------------------------------------
// Video signals
// --------------------------------------------------

my_vga_clk_generator VGAClkGen(
   .pclk(clk25),
   .out_hsync(hsync),
   .out_vsync(vsync),
   .out_blank(blank),
   .out_hcnt(video_x),
   .out_vcnt(video_y),
   .reset_n(aresetn) );

always @(posedge clk25) begin
	hsync_d <= hsync;
	vsync_d <= vsync;
	blank_d <= blank;
end

hdmi_device HDMI(
   .pclk(clk25),
   .tmds_clk(clk125), // pixel clock x5

   .in_vga_red(paletteout[15:8]),
   .in_vga_green(paletteout[23:16]),
   .in_vga_blue(paletteout[7:0]),

   .in_vga_blank(blank_d),
   .in_vga_vsync(vsync_d),
   .in_vga_hsync(hsync_d),

   .out_tmds_red(out_tmds_red),
   .out_tmds_green(out_tmds_green),
   .out_tmds_blue(out_tmds_blue),
   .out_tmds_clk(out_tmds_clk) );

// DDR DVI output
wire out_ddr_tmds_clk, out_ddr_tmds_red, out_ddr_tmds_green, out_ddr_tmds_blue;

ODDR #(.DDR_CLK_EDGE("SAME_EDGE"), .INIT(1'b0), .SRTYPE ("ASYNC")) oddr_clk (.D1(out_tmds_clk[0]), .D2(out_tmds_clk[1]), .C(clk125), .CE(1'b1), .Q(out_ddr_tmds_clk), .R(1'b0), .S(1'b0) );
OBUFDS OBUFDS_clk(.I(out_ddr_tmds_clk), .O(gpuvideoout.tmdsclkp), .OB(gpuvideoout.tmdsclkn));

ODDR #(.DDR_CLK_EDGE("SAME_EDGE"), .INIT(1'b0), .SRTYPE ("ASYNC")) oddr_red (.D1(out_tmds_red[0]), .D2(out_tmds_red[1]), .C(clk125), .CE(1'b1), .Q(out_ddr_tmds_red), .R(1'b0), .S(1'b0) );
OBUFDS OBUFDS_red(.I(out_ddr_tmds_red), .O(gpuvideoout.tmdsp[2]), .OB(gpuvideoout.tmdsn[2]));

ODDR #(.DDR_CLK_EDGE("SAME_EDGE"), .INIT(1'b0), .SRTYPE ("ASYNC")) oddr_green (.D1(out_tmds_green[0]), .D2(out_tmds_green[1]), .C(clk125), .CE(1'b1), .Q(out_ddr_tmds_green), .R(1'b0), .S(1'b0) );
OBUFDS OBUFDS_green(.I(out_ddr_tmds_green), .O(gpuvideoout.tmdsp[1]), .OB(gpuvideoout.tmdsn[1]));

ODDR #(.DDR_CLK_EDGE("SAME_EDGE"), .INIT(1'b0), .SRTYPE ("ASYNC")) oddr_blue (.D1(out_tmds_blue[0]), .D2(out_tmds_blue[1]), .C(clk125), .CE(1'b1), .Q(out_ddr_tmds_blue), .R(1'b0), .S(1'b0) );
OBUFDS OBUFDS_blue(.I(out_ddr_tmds_blue), .O(gpuvideoout.tmdsp[0]), .OB(gpuvideoout.tmdsn[0]));

assign m_axi.arsize = SIZE_16_BYTE; // 128bit read bus
assign m_axi.arburst = BURST_INCR;

assign m_axi.awlen = 0;				// one burst
assign m_axi.awsize = SIZE_16_BYTE; // 128bit write bus
assign m_axi.awburst = BURST_INCR;

// NOTE: This unit does not write to memory yet
// TODO: Will do so when raster or DMA unit is online
assign m_axi.awvalid = 0;
assign m_axi.awaddr = 'd0;
assign m_axi.wvalid = 0;
assign m_axi.wstrb = 16'h0000; // For raster unit or DMA, this will be the byte write mask for a 16 pixel horizontal tile
assign m_axi.wlast = 0;
assign m_axi.wdata = 'd0;
assign m_axi.bready = 0;

typedef enum logic [2:0] {DETECTSCANLINEEND, STARTLOAD, TRIGGERBURST, DATABURST} scanstatetype;
scanstatetype scanstate = DETECTSCANLINEEND;

// NOTE: First, set up the scanout address, then enable video scanout
logic [31:0] scanaddr = 32'h00000000;
logic [31:0] scanoffset = 0;
logic scanenable = 1'b0;

logic [5:0] rdata_cnt = 'd0;

// --------------------------------------------------
// Command FIFO
// --------------------------------------------------

typedef enum logic [2:0] {
	WCMD, DISPATCH,
	SETVPAGE,
	SETPAL,
	VMODE,
	//DMASOURCE,
	//DMATARGET,
	//DMAKICK,
	FINALIZE } gpucmdmodetype;
gpucmdmodetype cmdmode = WCMD;

logic [31:0] gpucmd = 'd0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		cmdmode <= WCMD;
		cmdre <= 1'b0;
		palettewe <= 1'b0;
	end else begin

		cmdre <= 1'b0;
		palettewe <= 1'b0;

		case (cmdmode)
			WCMD: begin
				if (gpufifovalid && ~gpufifoempty) begin
					gpucmd <= gpufifodout;
					// Advance FIFO
					cmdre <= 1'b1;
					// Dispatch cmd
					cmdmode <= DISPATCH;
				end
			end

			DISPATCH: begin
				case (gpucmd)
					32'h00000000:	cmdmode <= SETVPAGE;	// Set the scanout start address (followed by 32bit cached memory address, 64 byte cache aligned)
					32'h00000001:	cmdmode <= SETPAL;		// Set 24 bit color palette entry (followed by 8bit address+24bit color in next word)
					32'h00000002:	cmdmode <= VMODE;		// Set up video mode or turn off scan logic (default is 320x240*8bit paletted)
					// TODO: Primitive binning/setup, sprite draw, LBVH hit tests or anything else that makes sense to have here
					//32'h000000??:	cmdmode <= DMASOURCE;	// Set up source address for DMA
					//32'h000000??:	cmdmode <= DMATARGET;	// Set up target address for DMA
					//32'h000000??:	cmdmode <= DMAKICK;		// Queue up a DMA operation (optionally zero-masked)
					default:		cmdmode <= FINALIZE;	// Invalid command, wait one clock and try next
				endcase
			end

			SETVPAGE: begin
				if (gpufifovalid && ~gpufifoempty) begin
					scanaddr <= gpufifodout;	// Set new video scanout address (64 byte cache aligned, as we read in bursts)
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			SETPAL: begin
				if (gpufifovalid && ~gpufifoempty) begin
					palettewe <= 1'b1;
					palettewa <= gpufifodout[31:24];	// 8 bit palette index
					palettedin <= gpufifodout[23:0];	// 24 bit color
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			VMODE: begin
				if (gpufifovalid && ~gpufifoempty) begin
					scanenable <= gpufifodout[0]; // [0]: video output enabled when high, otherwise contents of scanline cache repeats across screen
					burstlen <= gpufifodout[1] ? 'd40 : 'd20;		// [1]: 40 bursts when high, otherwise 20 bursts
					scanmode <= gpufifodout[1];
					// rgb_vs_pal <= gpufifodout[2]; // select 16bit BGR mode (5:6:5) when high vs 8bit paletted mode when low
					// ? <= gpufifodout[31:3] unused for now
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			/*DMASOURCE: begin
			end
			DMATARGET: begin
			end
			DMAKICK: begin
			end*/

			FINALIZE: begin
				cmdmode <= WCMD;
			end

		endcase
	end
end

// --------------------------------------------------
// Scan-out logic
// --------------------------------------------------

// domain cross
(* async_reg = "true" *) logic [8:0] scanlinepre = 'd0;
(* async_reg = "true" *) logic [8:0] scanline = 'd0;
(* async_reg = "true" *) logic [9:0] scanpixelpre = 'd0;
(* async_reg = "true" *) logic [9:0] scanpixel = 'd0;
logic [31:0] blankcounter = 32'd0;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		blankcounter <= 32'd0;
	end else begin
		scanlinepre <= video_y[8:0];
		scanline <= scanlinepre;
		scanpixelpre <= video_x[9:0];
		scanpixel <= scanpixelpre;
		// Increment vertical blank counter (mapped to word reads from gpu fifo address)
		blankcounter <= blankcounter + (scanline == 480 ? 32'd1 : 32'd0);
	end
end
assign vblankcount = blankcounter;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		scanstate <= DETECTSCANLINEEND;
	end else begin
		case (scanstate)
			DETECTSCANLINEEND: begin
				if (scanpixel == 638 && scanline <= 480 && (~scanline[0] || scanmode)) begin
					// Starting at pixel 640 (638 due to DC delay), we have 160 pixels worth of time to cache the next scanline
					// This usually completes within 5 to 10 pixel's worth of time, way before the next scanline starts scanning
					scanoffset <= scanmode ? scanline[8:0] : {1'b0,scanline[8:1]};
					scanstate <= scanenable ? STARTLOAD : DETECTSCANLINEEND;
				end else
					// NOTE: Below scanline 480 is a good time for pending raster write work to run
					scanstate <= DETECTSCANLINEEND;
			end
			STARTLOAD: begin
				// This has to be a 64 byte cache aligned address to match cache burst reads we're running
				// Each scanline is a multiple of 64 bytes, so no need to further align here unless we have an odd output size (320 and 640 work just fine)
				m_axi.arlen <= burstlen - 8'd1;
				m_axi.araddr <= scanaddr + ( scanmode ? scanoffset*640 : scanoffset*320);
				m_axi.arvalid <= 1;
				scanstate <= TRIGGERBURST;
			end
			TRIGGERBURST: begin
				if (/*m_axi.arvalid && */m_axi.arready) begin
					rdata_cnt <= 0;
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					scanstate <= DATABURST;
				end else begin
					scanstate <= TRIGGERBURST;
				end
			end
			DATABURST: begin
				if (m_axi.rvalid  /*&& m_axi.rready*/) begin
					// Load data into scanline cache in 128bit chunks (16 pixels at 8bpp, 20 of them)
					// NOTE: video mode control sets up burst length to either 40 or 80
					scanlinecache[rdata_cnt] <= m_axi.rdata;
					rdata_cnt <= rdata_cnt + 'd1;
					m_axi.rready <= ~m_axi.rlast;
					scanstate <= m_axi.rlast ? DETECTSCANLINEEND : DATABURST;
				end else begin
					scanstate <= DATABURST;
				end
			end
		endcase
	end
end

endmodule
