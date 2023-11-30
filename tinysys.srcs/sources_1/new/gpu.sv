`timescale 1ns / 1ps

import axi4pkg::*;

module gpucore(
	input wire aclk,
	input wire clk25,
	input wire clk125,
	input wire aresetn,
	axi4if.master m_axi,
	output wire vvsync,
	output wire vhsync,
	output wire vclk,
	output wire vde,
	output wire [23:0] vdat,
	input wire gpufifoempty,
	input wire [31:0] gpufifodout,
	output wire gpufifore,
	input wire gpufifovalid,
	output wire [31:0] gpustate);

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

wire hsync, vsync;
wire [11:0] video_x;
wire [11:0] video_y;

logic cmdre = 1'b0;
assign gpufifore = cmdre;

// --------------------------------------------------
// Setup
// --------------------------------------------------

logic [7:0] burstlen;
logic scanwidth = 1'b0;			// 0:320 pixel wide, 1:640 pixel wide
logic colormode = 1'b0;			// 0:indexed color, 1:16bit color
logic [9:0] lastscanline;

// --------------------------------------------------
// Scanline cache and output address selection
// --------------------------------------------------

// The scanline cache
logic [127:0] scanlinecache [0:127];

logic [3:0] colorpixel;
logic [6:0] colorblock;

always_comb begin
	unique case ({scanwidth, colormode})
		2'b00: begin colorpixel = video_x[4:1];			colorblock = {2'b0, video_x[9:5]}; end	// 320*240 8bpp
		2'b01: begin colorpixel = {1'b0,video_x[3:1]};	colorblock = {1'b0, video_x[9:4]}; end	// 320*240 16bpp
		2'b10: begin colorpixel = video_x[3:0];			colorblock = {1'b0, video_x[9:4]}; end	// 640*480 8bpp
		2'b11: begin colorpixel = {1'b0,video_x[2:0]};	colorblock = video_x[9:3]; end			// 640*480 16bpp
	endcase
end

logic [127:0] newblock;
always_comb begin
	newblock = scanlinecache[colorblock];
end

logic [23:0] rgbcolor;
logic [7:0] paletteindex;

// Generate actual RGB color for 16bit mode
always_comb begin
	unique case (colorpixel[2:0])
		//                   B                        G                        R
		3'b000: rgbcolor = { newblock[4:0],     3'd0, newblock[10:5],    2'd0, newblock[15:11],   3'd0 };
		3'b001: rgbcolor = { newblock[20:16],   3'd0, newblock[26:21],   2'd0, newblock[31:27],   3'd0 };
		3'b010: rgbcolor = { newblock[36:32],   3'd0, newblock[42:37],   2'd0, newblock[47:43],   3'd0 };
		3'b011: rgbcolor = { newblock[52:48],   3'd0, newblock[58:53],   2'd0, newblock[63:59],   3'd0 };
		3'b100: rgbcolor = { newblock[68:64],   3'd0, newblock[74:69],   2'd0, newblock[79:75],   3'd0 };
		3'b101: rgbcolor = { newblock[84:80],   3'd0, newblock[90:85],   2'd0, newblock[95:91],   3'd0 };
		3'b110: rgbcolor = { newblock[100:96],  3'd0, newblock[106:101], 2'd0, newblock[111:107], 3'd0 };
		3'b111: rgbcolor = { newblock[116:112], 3'd0, newblock[122:117], 2'd0, newblock[127:123], 3'd0 };
	endcase
end

always_comb begin
	unique case (colorpixel)
		4'b0000: paletteindex = newblock[7:0];
		4'b0001: paletteindex = newblock[15:8];
		4'b0010: paletteindex = newblock[23:16];
		4'b0011: paletteindex = newblock[31:24];
		4'b0100: paletteindex = newblock[39:32];
		4'b0101: paletteindex = newblock[47:40];
		4'b0110: paletteindex = newblock[55:48];
		4'b0111: paletteindex = newblock[63:56];
		4'b1000: paletteindex = newblock[71:64];
		4'b1001: paletteindex = newblock[79:72];
		4'b1010: paletteindex = newblock[87:80];
		4'b1011: paletteindex = newblock[95:88];
		4'b1100: paletteindex = newblock[103:96];
		4'b1101: paletteindex = newblock[111:104];
		4'b1110: paletteindex = newblock[119:112];
		4'b1111: paletteindex = newblock[127:120];
	endcase
end

// --------------------------------------------------
// Palette RAM
// --------------------------------------------------

logic [7:0] palettewa;
logic palettewe;
logic [23:0] palettedin;

logic [23:0] paletteentries[0:255];

initial begin
	$readmemh("colorpalette.mem", paletteentries);
end

// Write port
always @(posedge aclk) begin
	if (palettewe)
		paletteentries[palettewa] <= palettedin;
end

// Read port
logic [23:0] paletteout = 0;
always @(posedge clk25) begin
	case ({scanenable, colormode})
		2'b10: paletteout <= paletteentries[paletteindex];
		2'b11: paletteout <= rgbcolor;
		default: paletteout <= 0;
	endcase
end

// --------------------------------------------------
// Video signal
// --------------------------------------------------

vgatimer VideoScanout(
	.rst_i(~aresetn),
	.clk_i(clk25),
	.hsync_o(vhsync),
	.vsync_o(vvsync),
	.counter_x(video_x),
	.counter_y(video_y),
	.vsynctrigger_o(),
	.vsynccounter() );

wire [3:0] vidG = paletteout[7:4];
wire [3:0] vidB = paletteout[15:12];
wire [3:0] vidR = paletteout[23:20];

assign vde = (video_x < 12'd640) && (video_y < 12'd480);
assign vdat = vde ? {12'd0, vidR, vidG, vidB} : 24'd0;
assign vclk = clk25;

// --------------------------------------------------
// AXI4 defaults
// --------------------------------------------------

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
			burstlen <= 'd19;
			lastscanline <= 10'd0;
			palettedin <= 24'd0;
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
				lastscanline <= gpufifodout[1] ? 10'd524 : 10'd523;
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

wire startofrowp = video_x == 12'd0;
wire endofcolumnp = video_y == 12'd490;
wire vsyncnow = startofrowp && endofcolumnp;

logic blankt = 1'b0;

always_ff @(posedge clk25) begin
	blankt <= vsyncnow ? ~blankt : blankt;

	if (~aresetn) begin
		blankt <= 1'b0;
	end
end

(* async_reg = "true" *) logic blanktogglepre = 1'b0;
(* async_reg = "true" *) logic blanktoggle = 1'b0;

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
        end
        1'b1: begin
			scanlinepre <= video_y;
			scanline <= scanlinepre;
			scanpixelpre <= video_x;
			scanpixel <= scanpixelpre;
			blanktogglepre <= blankt;
			blanktoggle <= blanktogglepre;
        end
    endcase

	if (~aresetn) begin
        smode <= 1'b0;
	end
end

assign gpustate = {31'd0, blanktoggle};

always_ff @(posedge aclk) begin
	case (scanstate)
		SINIT: begin
			m_axi.arvalid <= 0;
			m_axi.rready <= 0;
			scanstate <= DETECTFRAMESTART;
		end

		DETECTFRAMESTART: begin
			// When we reach the last odd scanline, start loading the cache
			if (scanenable && (scanline == lastscanline)) begin
				scanoffset <= scanaddr;
				scanstate <= STARTLOAD;
			end else begin
				scanstate <= DETECTFRAMESTART;
			end
		end

		STARTLOAD: begin
			// Only read on odd lines in 320-wide, or every other line in 640-wide mode
			// The trick here: we'll initially hit odd-even-even-odd sequence which means
			// the cache line we loaded on line 523 will only reload on line 1 after it's been displayed twice in 320 mode
			if ((scanpixel == 640) && (scanline[0] || scanwidth)) begin
				// This has to be a 64 byte cache aligned address to match cache burst reads we're running
				// Each scanline is a multiple of 64 bytes, so no need to further align here unless we have an odd output size (320 and 640 work just fine)
				m_axi.arlen <= burstlen;
				m_axi.araddr <= scanoffset;
				m_axi.arvalid <= (scanline == 10'd479) ? 1'b0 : 1'b1;
				// Keep reading as long as we're not on last line
				scanstate <= scanline == 10'd479 ? DETECTFRAMESTART : TRIGGERBURST;
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
