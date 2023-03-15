// --------------------------------------------------------------------
// SSD1306 OLED display controller commands and settings
// --------------------------------------------------------------------

// SSD1306 I2c address, 0x3C == (0x78>>1)
`define OLED_I2C_ADDRESS 7'h3C

`define I2CWRITE 1'b0
`define I2CREAD 1'b1

// Control command or data flow
`define SSD1306_COMMAND_STREAM							    	8'h00
`define SSD1306_SINGLE_COMMAND							    	8'h80
`define SSD1306_DATA_STREAM								    	8'h40
`define SSD1306_SINGLE_DATA								    	8'hC0

// Settings
`define SSD1306_EXTERNALVCC									    8'h01
`define SSD1306_SWITCHCAPVCC									8'h02

// Command groups

// 8'h00-8'h0F
`define SSD1306_SETLOWCOLUMN									8'h00
// 8'h10-8'h1F
`define SSD1306_SETHIGHCOLUMN									8'h10

`define SSD1306_MEMORYMODE										8'h20
`define SSD1306_COLUMNADDR										8'h21
`define SSD1306_PAGEADDR								    	8'h22
`define SSD1306_RIGHT_HORIZONTAL_SCROLL			         		8'h26
`define SSD1306_LEFT_HORIZONTAL_SCROLL					    	8'h27
`define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL	        8'h29
`define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL	            8'h2A
`define SSD1306_DEACTIVATE_SCROLL					    		8'h2E
`define SSD1306_ACTIVATE_SCROLL								    8'h2F

// 8'h40-8'h7F
`define SSD1306_SETSTARTLINE									8'h40

`define SSD1306_SETCONTRAST									    8'h81
`define SSD1306_CHARGEPUMP										8'h8D

// 8'hA0-8'hA1
`define SSD1306_SEGREMAP										8'hA0
`define SSD1306_SET_VERTICAL_SCROLL_AREA					    8'hA3
`define SSD1306_DISPLAYALLON_RESUME							    8'hA4
`define SSD1306_DISPLAYALLON									8'hA5
`define SSD1306_NORMALDISPLAY									8'hA6
`define SSD1306_INVERTDISPLAY									8'hA7
`define SSD1306_SETMULTIPLEX									8'hA8
`define SSD1306_DISPLAYOFF										8'hAE
`define SSD1306_DISPLAYON										8'hAF

// 8'hB0-8'hB7
`define SSD1306_SETPAGESTARTADDRESS							    8'hB0

`define SSD1306_COMSCANINC										8'hC0
`define SSD1306_COMSCANDEC										8'hC8

`define SSD1306_SETDISPLAYOFFSET								8'hD3
`define SSD1306_SETDISPLAYCLOCKDIV							    8'hD5
`define SSD1306_SETPRECHARGE									8'hD9
`define SSD1306_SETCOMPINS										8'hDA
`define SSD1306_SETVCOMDETECT									8'hDB

`define SSD1306_NOOP											8'hE3

`define SSD1306_LCDWIDTH 8'd128
`define SSD1306_LCDHEIGHT 8'd64

// Command Sequence - Initialize and power up the display (set to SSD1306_SEGREMAP | 8'h01 and SSD1306_COMSCANDEC for normal display orientation with pins below)
/*reg [7:0] i2c_command_sequence[64] = {
	// Device init sequence
	`SSD1306_COMMAND_STREAM,
	`SSD1306_DISPLAYOFF,
	`SSD1306_SETDISPLAYCLOCKDIV, 8'h80,
	`SSD1306_SETMULTIPLEX, `SSD1306_LCDHEIGHT - 1, // 8'h3F
	`SSD1306_SETDISPLAYOFFSET, 8'h00,
	`SSD1306_SETSTARTLINE | 8'h00,
	`SSD1306_CHARGEPUMP, 8'h14,							// Vcc (NOTE: only 0x14 works!)
	`SSD1306_MEMORYMODE, 8'h00,
	`SSD1306_SEGREMAP | 8'h00,							// 8'h00 for reverse horizontal (right to left when pins are above, 8'h01 for forward horizontal (left to right when pins are above)
	`SSD1306_COMSCANINC,								// SSD1306_COMSCANINC for forward vertical (start from pin side), SSD1306_COMSCANINC for reverse vertical (start from other side)
	`SSD1306_SETCOMPINS, 8'h12,							// 0x02 for 32pix, 0x12 for 64
	`SSD1306_SETCONTRAST, 8'hCF,
	`SSD1306_SETPRECHARGE, 8'hF1,
	`SSD1306_SETVCOMDETECT, 8'h40,
	`SSD1306_DISPLAYALLON_RESUME,
	`SSD1306_DEACTIVATE_SCROLL,
	`SSD1306_NORMALDISPLAY, // `SSD1306_INVERTDISPLAY for white display looks much better, keep it normal for blue/yellow displays
	`SSD1306_DISPLAYON,

	// Frame submit sequence (incoming data to follow is full width, all banks)
	`SSD1306_COMMAND_STREAM, //(index: 27)
	`SSD1306_COLUMNADDR, 8'h00, `SSD1306_LCDWIDTH - 1, // Start Column/End column
	`SSD1306_PAGEADDR, 8'h00, 8'h07, // Start/End Page

	// Unused instruction space (index: 34..63)
	`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP, `SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,
	`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP, `SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,
	`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP, `SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP,`SSD1306_NOOP
};*/