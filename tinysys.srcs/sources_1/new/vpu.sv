`timescale 1ns / 1ps

import axi4pkg::*;

module videocore(
	input wire aclk,
	input wire clk25,
	input wire aresetn,
	axi4if.master m_axi,
	output wire vvsync,
	output wire vhsync,
	output wire vclk,
	output wire vde,
	output wire [11:0] vdat,
	input wire vpufifoempty,
	input wire [31:0] vpufifodout,
	output wire vpufifore,
	input wire vpufifovalid,
	output wire [31:0] vpustate);

// A simple video unit with the following features:
// - Four video output modes (320x240 or 640x480, indexed or 16bpp)
// - Framebuffer scan-out from any cache aligned memory location in mapped device memory
// - Frame counter support for vsync implementations
// - Memory mapped command buffer interface

// --------------------------------------------------
// Scan setup
// --------------------------------------------------

// NOTE: First, set up the scanout address, then enable video scanout
logic [31:0] scanaddr;
logic [31:0] scanoffset;
logic [10:0] scaninc;
logic scanenable;

// --------------------------------------------------
// Common
// --------------------------------------------------

wire [11:0] video_x;
wire [11:0] video_y;

logic cmdre;
assign vpufifore = cmdre;

// --------------------------------------------------
// Setup
// --------------------------------------------------

logic [7:0] burstlen;
logic scanwidth = 1'b0;			// 0:320 pixel wide, 1:640 pixel wide
logic colormode = 1'b0;			// 0:indexed color, 1:16bit color
logic [9:0] lastscanline;


// --------------------------------------------------
// Scanline cache
// --------------------------------------------------

logic [127:0] scanlinecache [0:127];

wire [127:0] scanlinedout;
logic [127:0] scanlinedin;
logic scanlinewe = 1'b0;
logic [6:0] scanlinewa;
logic [6:0] scanlinera;

always @(posedge aclk) begin
	if (scanlinewe)
		scanlinecache[scanlinewa] <= scanlinedin;

	// if (~aresetn) begin
	// end
end
assign scanlinedout = scanlinecache[scanlinera];

// --------------------------------------------------
// Output address selection
// --------------------------------------------------

logic [3:0] pixelscanaddr;

always_comb begin
	unique case ({scanwidth, colormode})
		2'b00: begin pixelscanaddr = video_x[4:1];			scanlinera = {2'b0, video_x[9:5]}; end	// 320*240 8bpp
		2'b01: begin pixelscanaddr = {1'b0,video_x[3:1]};	scanlinera = {1'b0, video_x[9:4]}; end	// 320*240 16bpp
		2'b10: begin pixelscanaddr = video_x[3:0];			scanlinera = {1'b0, video_x[9:4]}; end	// 640*480 8bpp
		2'b11: begin pixelscanaddr = {1'b0,video_x[2:0]};	scanlinera = video_x[9:3]; end			// 640*480 16bpp
	endcase
end

// --------------------------------------------------
// Output color
// --------------------------------------------------

logic [11:0] rgbcolor;
logic [7:0] paletteindex;

// Generate actual RGB color for 16bit mode
always_comb begin
	// Pixel data is 12 bits but stored as 16 bits (the spare 4 bits are unused for now, could be utilized as alpha etc
	unique case (pixelscanaddr[2:0])
		//                   R:G:B                    UNUSED
		3'b000: rgbcolor = { scanlinedout[11:0]     }; // scanlinedout[15:12]
		3'b001: rgbcolor = { scanlinedout[27:16]    }; // scanlinedout[31:28]
		3'b010: rgbcolor = { scanlinedout[43:32]    }; // scanlinedout[47:44]
		3'b011: rgbcolor = { scanlinedout[59:48]    }; // scanlinedout[63:60] 
		3'b100: rgbcolor = { scanlinedout[75:64]    }; // scanlinedout[79:76]
		3'b101: rgbcolor = { scanlinedout[91:80]    }; // scanlinedout[95:92]
		3'b110: rgbcolor = { scanlinedout[107:96]   }; // scanlinedout[111:108]
		3'b111: rgbcolor = { scanlinedout[123:112]  }; // scanlinedout[127:124]
	endcase
end

always_comb begin
	unique case (pixelscanaddr)
		4'b0000: paletteindex = scanlinedout[7:0];
		4'b0001: paletteindex = scanlinedout[15:8];
		4'b0010: paletteindex = scanlinedout[23:16];
		4'b0011: paletteindex = scanlinedout[31:24];
		4'b0100: paletteindex = scanlinedout[39:32];
		4'b0101: paletteindex = scanlinedout[47:40];
		4'b0110: paletteindex = scanlinedout[55:48];
		4'b0111: paletteindex = scanlinedout[63:56];
		4'b1000: paletteindex = scanlinedout[71:64];
		4'b1001: paletteindex = scanlinedout[79:72];
		4'b1010: paletteindex = scanlinedout[87:80];
		4'b1011: paletteindex = scanlinedout[95:88];
		4'b1100: paletteindex = scanlinedout[103:96];
		4'b1101: paletteindex = scanlinedout[111:104];
		4'b1110: paletteindex = scanlinedout[119:112];
		4'b1111: paletteindex = scanlinedout[127:120];
	endcase
end

// --------------------------------------------------
// Palette RAM
// --------------------------------------------------

logic [7:0] palettewa;
logic palettewe;
logic [11:0] palettedin;

logic [11:0] paletteentries[0:255];

initial begin
	$readmemh("colorpalette.mem", paletteentries);
end

// Write port
always @(posedge aclk) begin
	if (palettewe)
		paletteentries[palettewa] <= palettedin;
end

// Read port
logic [11:0] paletteout = 0;
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

wire hsync, vsync;
vgatimer VideoScanout(
	.rst_i(~aresetn),
	.clk_i(clk25),
	.hsync_o(hsync),
	.vsync_o(vsync),
	.counter_x(video_x),
	.counter_y(video_y),
	.vsynctrigger_o(),
	.vsynccounter() );

logic hsync_d, vsync_d, vde_d;
logic [11:0] paletteout_d;
wire notblank = (video_x < 12'd640) && (video_y < 12'd480);
always @(posedge clk25) begin
	hsync_d <= hsync;
	vsync_d <= vsync;
	vde_d <= notblank;
	paletteout_d <= notblank ? paletteout : 12'd0;
end

assign vhsync = hsync_d;
assign vvsync = vsync_d;
assign vde = vde_d;
assign vdat = paletteout_d;
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

// --------------------------------------------------
// Command FIFO
// --------------------------------------------------

typedef enum logic [2:0] {
	WCMD, DISPATCH,
	SETVPAGE,
	SETPAL,
	VMODE, VSCANSIZE,
	FINALIZE } vpucmdmodetype;
vpucmdmodetype cmdmode = WCMD;

logic [31:0] vpucmd;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		vpucmd <= 32'd0;
		scanaddr <= 32'd0;
		burstlen <= 'd19;
		lastscanline <= 10'd0;
		palettedin <= 24'd0;
		scanenable <= 1'b0;
		cmdre <= 1'b0;
		palettewe <= 1'b0;
		cmdmode <= WCMD;
	end else begin
		cmdre <= 1'b0;
		palettewe <= 1'b0;

		case (cmdmode)
			WCMD: begin
				if (vpufifovalid && ~vpufifoempty) begin
					vpucmd <= vpufifodout;
					// Advance FIFO
					cmdre <= 1'b1;
					// Dispatch cmd
					cmdmode <= DISPATCH;
				end
			end

			DISPATCH: begin
				case (vpucmd)
					32'h00000000:	cmdmode <= SETVPAGE;	// Set the scanout start address (followed by 32bit cached memory address, 64 byte cache aligned)
					32'h00000001:	cmdmode <= SETPAL;		// Set 24 bit color palette entry (followed by 8bit address+24bit color in next word)
					32'h00000002:	cmdmode <= VMODE;		// Set up video mode or turn off scan logic (default is 320x240*8bit paletted)
					default:		cmdmode <= FINALIZE;	// Invalid command, wait one clock and try next
				endcase
			end

			SETVPAGE: begin
				if (vpufifovalid && ~vpufifoempty) begin
					scanaddr <= vpufifodout;	// Set new video scanout address (64 byte cache aligned, as we read in bursts)
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			SETPAL: begin
				if (vpufifovalid && ~vpufifoempty) begin
					palettewe <= 1'b1;
					palettewa <= vpufifodout[31:24];	// 8 bit palette index
					//??? <= vpufifodout[23:16];		// 8 bit extra data, unused
					//??? <= vpufifodout[15:11];		// 4 bit extra data, alpha perhaps?
					palettedin <= vpufifodout[11:0];	// 12 bit color
					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= FINALIZE;
				end
			end

			VMODE: begin
				if (vpufifovalid && ~vpufifoempty) begin
					scanenable <= vpufifodout[0];	// 0:video output disabled, 1:video output enabled
					scanwidth <= vpufifodout[1];	// 0:320-wide, 1:640-wide
					colormode <= vpufifodout[2];	// 0:8bit indexed, 1:16bit rgb
					lastscanline <= vpufifodout[1] ? 10'd524 : 10'd523;
					// ? <= vpufifodout[31:3] unused for now

					// Set up burst count to 20 / 40 / 80 depending on video mode
					unique case ({vpufifodout[2], vpufifodout[1]})
						2'b00: burstlen <= 'd19;	// 320*240 8bpp
						2'b01: burstlen <= 'd39;	// 640*480 8bpp
						2'b10: burstlen <= 'd39;	// 320*240 16bpp
						2'b11: burstlen <= 'd79;	// 640*480 16bpp
					endcase

					// Advance FIFO
					cmdre <= 1'b1;
					cmdmode <= VSCANSIZE;
				end
			end

			VSCANSIZE: begin
				unique case ({vpufifodout[2], vpufifodout[1]})
					2'b00: scaninc <= 11'd320;	// 320*240 8bpp
					2'b01: scaninc <= 11'd640;	// 640*480 8bpp
					2'b10: scaninc <= 11'd640;	// 320*240 16bpp
					2'b11: scaninc <= 11'd1280;	// 640*480 16bpp
				endcase
				cmdmode <= FINALIZE;
			end

			FINALIZE: begin
				cmdmode <= WCMD;
			end
		endcase

	end
end

// --------------------------------------------------
// Scan-out logic
// --------------------------------------------------

wire startofrowp = video_x == 12'd0;
wire endofcolumnp = video_y == 12'd490;
wire vsyncnow = startofrowp && endofcolumnp;

logic blankt;
always_ff @(posedge clk25) begin
	if (~aresetn) begin
		blankt <= 1'b0;
	end else begin
		blankt <= vsyncnow ? ~blankt : blankt;
	end
end

(* async_reg = "true" *) logic [9:0] scanlinepre;
(* async_reg = "true" *) logic [9:0] scanline;
(* async_reg = "true" *) logic [9:0] scanpixelpre;
(* async_reg = "true" *) logic [9:0] scanpixel;
(* async_reg = "true" *) logic blanktogglepre;
(* async_reg = "true" *) logic blanktoggle;

// Vertical blanking and pixel tracking
always_ff @(posedge aclk) begin
	if (~aresetn) begin
		scanline <= 10'd0;
		scanlinepre <= 10'd0;
		scanpixelpre <= 10'd0;
		scanpixel <= 10'd0;
		blanktogglepre <= 1'b0;
		blanktoggle <= 1'b0;
	end else begin
		scanlinepre <= video_y;
		scanline <= scanlinepre;
		scanpixelpre <= video_x;
		scanpixel <= scanpixelpre;
		blanktogglepre <= blankt;
		blanktoggle <= blanktogglepre;
	end
end

assign vpustate = {31'd0, blanktoggle};

typedef enum logic [2:0] {DETECTFRAMESTART, STARTLOAD, TRIGGERBURST, DATABURST, ADVANCESCANLINEADDRESS} scanstatetype;
scanstatetype scanstate = DETECTFRAMESTART;

logic [6:0] rdata_cnt;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		scanlinewe <= 1'b0;
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		rdata_cnt <= 7'd0;
		scanstate <= DETECTFRAMESTART;
	end else begin
		scanlinewe <= 1'b0;
		case (scanstate)
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
					scanlinewe <= 1'b1;
					scanlinewa <= rdata_cnt;
					scanlinedin <= m_axi.rdata;
					rdata_cnt <= rdata_cnt + 'd1;
					m_axi.rready <= ~m_axi.rlast;
					scanstate <= m_axi.rlast ? ADVANCESCANLINEADDRESS : DATABURST;
				end else begin
					scanstate <= DATABURST;
				end
			end

			ADVANCESCANLINEADDRESS: begin
				// Wait for and load next scanline
				scanoffset <= scanoffset + scaninc;
				scanstate <= STARTLOAD;
			end

		endcase
	end
end

endmodule
