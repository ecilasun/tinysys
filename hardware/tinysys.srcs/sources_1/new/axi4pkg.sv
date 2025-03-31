`timescale 1ns / 1ps

package axi4pkg;

	localparam ADDR_WIDTH = 32;
	localparam DATA_WIDTH = 128;
	localparam STRB_WIDTH = DATA_WIDTH / 8;

	localparam SIZE_1_BYTE   = 3'b000;
	localparam SIZE_2_BYTE   = 3'b001;
	localparam SIZE_4_BYTE   = 3'b010;
	localparam SIZE_8_BYTE   = 3'b011;
	localparam SIZE_16_BYTE  = 3'b100;
	localparam SIZE_32_BYTE  = 3'b101;
	localparam SIZE_64_BYTE  = 3'b110;
	localparam SIZE_128_BYTE = 3'b111;

	localparam BURST_FIXED = 2'b00;
	localparam BURST_INCR  = 2'b01;
	localparam BURST_WRAP  = 2'b10;

	localparam RESP_OKAY   = 2'b00;
	localparam RESP_EXOKAY = 2'b01;
	localparam RESP_SLVERR = 2'b10;
	localparam RESP_DECERR = 2'b11;

	typedef logic [ADDR_WIDTH - 1 : 0] addr_t;
	typedef logic [DATA_WIDTH - 1 : 0] data_t;
	typedef logic [STRB_WIDTH - 1 : 0] strb_t;
	typedef logic [7 : 0] len_t;
	typedef logic [2 : 0] size_t;
	typedef logic [1 : 0] burst_t;
	typedef logic [1 : 0] resp_t;

endpackage
