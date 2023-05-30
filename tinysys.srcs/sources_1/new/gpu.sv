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
// - Three video output modes (320x240 and 640x480 indexed, and 320x240 16bit)
// - Framebuffer scan-out from any cache aligned memory location in mapped device memory
// - Frame counter support for vsync implementations
// - Memory mapped command buffer interface

// TODO:
// - Allow command data to be read from any cache aligned memory location in 16 byte bursts
// - Add sprite caching support at end of each scanline, and sprite compositing
// - Add rasterization support

// --------------------------------------------------
// Scanline and scan pixel cdc registers
// --------------------------------------------------

(* async_reg = "true" *) logic [8:0] scanlinepre = 'd0;
(* async_reg = "true" *) logic [8:0] scanline = 'd0;
(* async_reg = "true" *) logic [9:0] scanpixelpre = 'd0;
(* async_reg = "true" *) logic [9:0] scanpixel = 'd0;

// --------------------------------------------------
// Scan setup
// --------------------------------------------------

// NOTE: First, set up the scanout address, then enable video scanout
logic [31:0] scanaddr = 32'h00000000;
logic [31:0] scanoffset = 0;
logic [31:0] scaninc = 0;
logic scanenable = 1'b0;

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
// Character RAM
// --------------------------------------------------

// 80*60 for 640*480
logic [15:0] charRAM[0:4799];

initial begin
	for (int i=0;i<4800;i++)
		charRAM[i] = 16'h0200; // Green text by default
end

wire [6:0] charblockx = scanpixel[9:3];
wire [5:0] charblocky = scanline[8:3];
wire [12:0] charaddr = charblockx + charblocky*80;

logic charwe = 1'b0;
logic [15:0] charin;
logic [12:0] charWaddr;

always @(posedge aclk) begin
	if (charwe)
		charRAM[charWaddr] <= charin;
end

wire [7:0] charOut = charRAM[charaddr][7:0];
wire [7:0] charPalOut = charRAM[charaddr][15:8];

// --------------------------------------------------
// Font ROM
// --------------------------------------------------

// 256*24 1bit grid
logic [0:0] fontROM[0:6143];

initial begin
	$readmemh("font.mem", fontROM);
end

wire [5:0] charrow = {charOut[7:5],3'b0}; // (currentchar/32)*8
wire [7:0] charcol = {charOut[4:0],3'b0}; // (currentchar%32)*8

logic [15:0] fontAddrs;
assign fontAddrs[7:0] = charcol + scanpixel[2:0]; 
assign fontAddrs[15:8] = charrow + scanline[2:0];	// (charrow+(y%8))*256 + (charcol+(x%8))

wire fontBit = fontROM[fontAddrs];
wire fontPixel = fontBit;
wire [7:0] fontColor = charPalOut;

// --------------------------------------------------
// Setup
// --------------------------------------------------

logic [7:0] burstlen = 'd20;	// 20 reads for 320*240, 40 reads for 640*480 in paletted mode (each read is 128bits)
logic scanwidth = 1'b0;			// 0:320 pixel wide, 1:640 pixel wide
logic colormode = 1'b0;			// 0:indexed color, 1:16bit color

// --------------------------------------------------
// Scanline cache and output address selection
// --------------------------------------------------

// The scanline cache
logic [127:0] scanlinecache [0:127];

logic [3:0] colorpixel;
logic [6:0] colorblock;

always_comb begin
	unique case ({scanwidth, colormode})
		2'b00: begin colorpixel = video_x[4:1];			colorblock = {1'b0,video_x[10:5]}; end	// 320*240 8bpp
		2'b01: begin colorpixel = {1'b0,video_x[3:1]};	colorblock = video_x[10:4]; end			// 320*240 16bpp
		2'b10: begin colorpixel = video_x[3:0];			colorblock = {1'b0, video_x[9:4]}; end	// 640*480 8bpp
		2'b11: begin colorpixel = {1'b0,video_x[2:0]};	colorblock = video_x[9:3]; end			// 640*480 16bpp
	endcase
end

logic [7:0] palettewa;
logic palettewe = 1'b0;
logic [7:0] palettera;
logic [15:0] rgbcolor;
logic [23:0] palettedin = 24'h000000;

// Generate palette read address from current pixel's color index
always_ff @(posedge clk25) begin
	// Color index
	unique case (colorpixel)
		4'b0000: palettera <= scanlinecache[colorblock][7 : 0];
		4'b0001: palettera <= scanlinecache[colorblock][15 : 8];
		4'b0010: palettera <= scanlinecache[colorblock][23 : 16];
		4'b0011: palettera <= scanlinecache[colorblock][31 : 24];
		4'b0100: palettera <= scanlinecache[colorblock][39 : 32];
		4'b0101: palettera <= scanlinecache[colorblock][47 : 40];
		4'b0110: palettera <= scanlinecache[colorblock][55 : 48];
		4'b0111: palettera <= scanlinecache[colorblock][63 : 56];
		4'b1000: palettera <= scanlinecache[colorblock][71 : 64];
		4'b1001: palettera <= scanlinecache[colorblock][79 : 72];
		4'b1010: palettera <= scanlinecache[colorblock][87 : 80];
		4'b1011: palettera <= scanlinecache[colorblock][95 : 88];
		4'b1100: palettera <= scanlinecache[colorblock][103 : 96];
		4'b1101: palettera <= scanlinecache[colorblock][111 : 104];
		4'b1110: palettera <= scanlinecache[colorblock][119 : 112];
		4'b1111: palettera <= scanlinecache[colorblock][127 : 120];
	endcase
end

// Generate actual RGB color for 16bit mode
always_ff @(posedge clk25) begin
	// Color index
	unique case (colorpixel[2:0])
		3'b000: rgbcolor <= scanlinecache[colorblock][15 : 0];
		3'b001: rgbcolor <= scanlinecache[colorblock][31 : 16];
		3'b010: rgbcolor <= scanlinecache[colorblock][47 : 32];
		3'b011: rgbcolor <= scanlinecache[colorblock][63 : 48];
		3'b100: rgbcolor <= scanlinecache[colorblock][79 : 64];
		3'b101: rgbcolor <= scanlinecache[colorblock][95 : 80];
		3'b110: rgbcolor <= scanlinecache[colorblock][111 : 96];
		3'b111: rgbcolor <= scanlinecache[colorblock][127 : 112];
	endcase
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
	if (palettewe)
		paletteentries[palettewa] <= palettedin;
end

// Read port
logic [23:0] paletteout = 0;
always @(posedge clk25) begin // Tied to GPU clock
	// NOTE: This currently uses MASK mode
	// All possible modes:
	// MASK / MAX / REPLACE / BLEND
	unique case ({scanenable && (scanline < 480), colormode})
		2'b10: paletteout <= fontPixel ? paletteentries[fontColor] : paletteentries[palettera];
		2'b11: paletteout <= fontPixel ? paletteentries[fontColor] : {
			rgbcolor[5:0],2'd0,		// G
			rgbcolor[10:6],3'd0,	// R
			rgbcolor[15:11],3'd0};	// B
		default: paletteout <= 0;
	endcase
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

   .in_vga_red(paletteout[15:8]),		// TODO: Have other options for console compositing
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

typedef enum logic [2:0] {SINIT, DETECTSCANLINEEND, STARTLOAD, TRIGGERBURST, DATABURST} scanstatetype;
scanstatetype scanstate = SINIT;

logic [6:0] rdata_cnt = 'd0;

// --------------------------------------------------
// Command FIFO
// --------------------------------------------------

typedef enum logic [2:0] {
	CINIT,
	WCMD, DISPATCH,
	SETVPAGE,
	SETPAL,
	VMODE,
	PUTCHAR,
	FINALIZE } gpucmdmodetype;
gpucmdmodetype cmdmode = CINIT;

logic [31:0] gpucmd = 'd0;

always_ff @(posedge aclk) begin
	cmdre <= 1'b0;
	palettewe <= 1'b0;
	charWaddr <= 13'dz;
	charwe <= 1'b0;

	case (cmdmode)
		CINIT: begin
			cmdmode <= WCMD;
		end

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
				32'h00000003:	cmdmode <= PUTCHAR;		// Write one character and its color attribute at given x/y
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
				scanenable <= gpufifodout[0];	// 0:video output disabled, 1:video output enabled
				scanwidth <= gpufifodout[1];	// 0:320-wide, 1:640-wide
				colormode <= gpufifodout[2];	// 0:8bit indexed, 1:16bit rgb
				// ? <= gpufifodout[31:3] unused for now
				// Set up burst count to 20 / 40 / 80 depending on video mode
				unique case ({gpufifodout[1], gpufifodout[2]})
					2'b00: burstlen <= 'd19;	// 320*240 8bpp
					2'b01: burstlen <= 'd39;	// 320*240 16bpp
					2'b10: burstlen <= 'd39;	// 640*480 8bpp
					2'b11: burstlen <= 'd79;	// 640*480 16bpp
				endcase
				unique case (gpufifodout[1])
					2'b0: scaninc <= 640;	// 320*240
					2'b1: scaninc <= 1280;	// 640*480
				endcase
				// Advance FIFO
				cmdre <= 1'b1;
				cmdmode <= FINALIZE;
			end
		end
		
		PUTCHAR: begin
			if (gpufifovalid && ~gpufifoempty) begin
				// bits[31:29]	-> unused for now, could be blink etc (2 bits)
				// bits[28:16]	-> x+y*80 (13 bits)
				// bits[15:0]	-> char palette index + ASCII code (16 bits)
				charWaddr <= gpufifodout[28:16];
				charwe <= 1'b1;
				charin <= gpufifodout[15:0];
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
		cmdmode <= CINIT;
	end
end

// --------------------------------------------------
// Scan-out logic
// --------------------------------------------------

// Vertical blanking counter
logic [31:0] blankcounter = 32'd0;

always_ff @(posedge aclk) begin
	scanlinepre <= video_y[8:0];
	scanline <= scanlinepre;
	scanpixelpre <= video_x[9:0];
	scanpixel <= scanpixelpre;
	// Increment vertical blank counter (mapped to word reads from gpu fifo address)
	blankcounter <= blankcounter + (scanline == 480 ? 32'd1 : 32'd0);

	if (~aresetn) begin
		//scanline <= 9'd0;
		scanlinepre <= 9'd0;
		scanpixelpre <= 9'd0;
		scanpixel <= 9'd0;
		blankcounter <= 32'd0;
	end
end
assign vblankcount = blankcounter;

always_ff @(posedge aclk) begin
	case (scanstate)
		SINIT: begin
			m_axi.arvalid <= 0;
			m_axi.rready <= 0;
			scanstate <= DETECTSCANLINEEND;
		end

		DETECTSCANLINEEND: begin
			if (scanenable && scanpixel == 638 && scanline < 480 && (~scanline[0] || scanwidth)) begin
				scanoffset <= scanoffset + (colormode ? scaninc : {1'b0,scaninc[31:1]});
				if (scanline == 0)
					scanoffset <= scanaddr;
				scanstate <= STARTLOAD;
			end else
				scanstate <= DETECTSCANLINEEND;
		end

		STARTLOAD: begin
			// This has to be a 64 byte cache aligned address to match cache burst reads we're running
			// Each scanline is a multiple of 64 bytes, so no need to further align here unless we have an odd output size (320 and 640 work just fine)
			m_axi.arlen <= burstlen;
			m_axi.araddr <= scanoffset;
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

	if (~aresetn) begin
		scanstate <= SINIT;
	end
end

endmodule
