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
// - Four video output modes (320x240 or 640x480, indexed or 16bpp)
// - Framebuffer scan-out from any cache aligned memory location in mapped device memory
// - Frame counter support for vsync implementations
// - Memory mapped command buffer interface

// TODO:
// - Allow command data to be read from any cache aligned memory location in 16 byte bursts
// - Add sprite support

// --------------------------------------------------
// Scanline and scan pixel cdc registers
// --------------------------------------------------

(* async_reg = "true" *) logic [9:0] scanlinepre = 'd0;
(* async_reg = "true" *) logic [9:0] scanline = 'd0;
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
		2'b00: begin colorpixel = video_x[4:1];			colorblock = {1'b0, video_x[10:5]}; end	// 320*240 8bpp
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
		2'b10: paletteout <= paletteentries[palettera];
		2'b11: paletteout <= {
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

typedef enum logic [2:0] {SINIT, DETECTFRAMESTART, STARTLOAD, TRIGGERBURST, DATABURST, ADVANCESCANLINEADDRESS} scanstatetype;
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
	FINALIZE } gpucmdmodetype;
gpucmdmodetype cmdmode = CINIT;

logic [31:0] gpucmd = 'd0;

always_ff @(posedge aclk) begin
	cmdre <= 1'b0;
	palettewe <= 1'b0;

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

wire startofrowp = video_x[9:0] == 10'd0;
wire endofcolumnp = video_y[9:0] == 10'd490;
wire vsyncnow = startofrowp && endofcolumnp;

logic [31:0] blankcnt = 32'd0;

always_ff @(posedge clk25) begin
	// Increment vertical blank counter (mapped to word reads from gpu fifo address)
	blankcnt <= blankcnt + (vsyncnow ? 32'd1 : 32'd0);
	if (~aresetn) begin
		blankcnt <= 0;
	end
end

(* async_reg = "true" *) logic [31:0] blankcounterpre = 32'd0;
(* async_reg = "true" *) logic [31:0] blankcounter = 32'd0;

// Vertical blanking counter
logic smode = 1'b0;
always_ff @(posedge aclk) begin

	smode <= 1'b1;

	case (smode)
		1'b0: begin
			scanline <= 10'd0;
			scanlinepre <= 10'd0;
			scanpixelpre <= 10'd0;
			scanpixel <= 10'd0;
			blankcounter <= 32'd0;
        end
        1'b1: begin
			scanlinepre <= video_y[9:0];
			scanline <= scanlinepre;
			scanpixelpre <= video_x[9:0];
			scanpixel <= scanpixelpre;
			blankcounterpre <= blankcnt;
			blankcounter <= blankcounterpre;
        end
    endcase

	if (~aresetn) begin
        smode <= 1'b0;
	end
end
assign vblankcount = blankcounter;

always_ff @(posedge aclk) begin
	case (scanstate)
		SINIT: begin
			m_axi.arvalid <= 0;
			m_axi.rready <= 0;
			scanstate <= DETECTFRAMESTART;
		end

		DETECTFRAMESTART: begin
			// When we reach the very last scanline, start loading the cache
			if (scanenable && scanline == 524) begin
				scanoffset <= scanaddr;
				scanstate <= STARTLOAD;
			end else begin
				scanstate <= DETECTFRAMESTART;
			end
		end

		STARTLOAD: begin
			// TODO: Only read on even lines in 320-wide, or every other line in 640-wide mode
			if (scanpixel == 640 && (~scanline[0] || scanwidth)) begin
				// This has to be a 64 byte cache aligned address to match cache burst reads we're running
				// Each scanline is a multiple of 64 bytes, so no need to further align here unless we have an odd output size (320 and 640 work just fine)
				m_axi.arlen <= burstlen;
				m_axi.araddr <= scanoffset;
				m_axi.arvalid <= (scanline == 480) ? 1'b0 : 1'b1;
				// Keep reading as long as we're not on last line
				scanstate <= scanline == 480 ? DETECTFRAMESTART : TRIGGERBURST;
			end else begin
				scanstate <= STARTLOAD;
			end
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
				scanstate <= m_axi.rlast ? ADVANCESCANLINEADDRESS : DATABURST;
			end else begin
				scanstate <= DATABURST;
			end
		end

		ADVANCESCANLINEADDRESS: begin
			// Wait for and load next scanline
			scanoffset <= scanoffset + (colormode ? scaninc : {1'b0,scaninc[31:1]});
			scanstate <= STARTLOAD;
		end

	endcase

	if (~aresetn) begin
		scanstate <= SINIT;
	end
end

endmodule
