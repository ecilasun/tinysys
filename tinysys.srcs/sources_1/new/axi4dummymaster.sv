`timescale 1ns / 1ps

// Dummy master generating dummy bus traffic

import axi4pkg::*;

module dummymaster #(
	parameter int MASTERID = 32'h20000000
) (
	input wire aclk,
	input wire aresetn,
	axi4if.master m_axi);

// --------------------------------------------------
// Common
// --------------------------------------------------

// Single 128 bit read or write (cache of 512 byte width would do 4 of these)
localparam burstlen = 1;

// --------------------------------------------------
// Read logic
// --------------------------------------------------

logic [31:0] raddr = 32'h00000000; // NOTE: Must be 16 byte (128bit) aligned
//logic [127:0] readdata;

assign m_axi.arlen = burstlen - 1;
assign m_axi.arsize = SIZE_16_BYTE; // 128bit read bus
assign m_axi.arburst = BURST_INCR;

// TEMP
assign m_axi.arvalid = m_axi.arready;
assign m_axi.araddr = 'd0;
assign m_axi.rready = 0;

/*typedef enum logic [1:0] {RINIT, RIDLE, RADDR, RDATA} readstate_type;
readstate_type readstate = RINIT;

always @(posedge aclk) begin

	case(readstate)
		RINIT: begin
			m_axi.arvalid <= 0;
			m_axi.araddr <= 'd0;
			m_axi.rready <= 0;
			readstate <= RIDLE;
		end

		RIDLE: begin
			m_axi.araddr <= raddr;
			raddr <= raddr + 32'd16;
			m_axi.arvalid <= 1'b1;
			readstate <= RADDR;
		end

		RADDR: begin
			if (m_axi.arready) begin // && m_axi.arvalid
				m_axi.arvalid <= 1'b0;
				m_axi.rready <= 1'b1;
				readstate <= RDATA;
			end
		end

		RDATA: begin
			if (m_axi.rvalid) begin // && m_axi.rready
				m_axi.rready <= 1'b0;
				readdata <= m_axi.rdata;
				readstate <= RIDLE;
			end
		end
	endcase

	if (~aresetn) begin
		readstate <= RINIT;
	end
end*/

// --------------------------------------------------
// Write logic
// --------------------------------------------------

logic [31:0] waddr = 32'h00008000; // NOTE: Must be 16 byte (128bit) aligned

assign m_axi.awlen = burstlen - 1;
assign m_axi.awsize = SIZE_16_BYTE; // 128bit write bus
assign m_axi.awburst = BURST_INCR;

// TEMP
assign m_axi.awvalid = 0;
assign m_axi.awaddr = 'd0;
assign m_axi.wvalid = 0;
assign m_axi.wstrb = 16'h0000;
assign m_axi.wlast = 0;
assign m_axi.wdata = 'd0;
assign m_axi.bready = 0;

/*typedef enum logic [2:0] {WINIT, WIDLE, WADDR, WDATA, WRESP} writestate_type;
writestate_type writestate = WINIT;

always @(posedge aclk) begin
	case(writestate)
		WINIT: begin
			m_axi.awvalid <= 0;
			m_axi.awaddr <= 'd0;
			m_axi.wvalid <= 0;
			m_axi.wstrb <= 16'h0000;
			m_axi.wlast <= 0;
			m_axi.wdata <= 'd0;
			m_axi.bready <= 0;
			writestate <= WIDLE;
		end

		WIDLE : begin
			m_axi.awaddr <= waddr;
			waddr <= waddr + 32'd16;
			m_axi.awvalid <= 1;
			writestate <= WADDR;
		end

		WADDR : begin
			if (m_axi.awready) begin // && m_axi.awvalid
				m_axi.awvalid <= 0;
				m_axi.wdata <= {MASTERID, MASTERID, MASTERID, MASTERID};//din[wdata_cnt];
				m_axi.wstrb <= 16'hFFFF;
				m_axi.wvalid <= 1;
				m_axi.wlast <= 1;
				writestate <= WDATA;
			end else begin
				writestate <= WADDR;
			end
		end

		WDATA : begin
			if (m_axi.wready) begin // && m_axi.wvalid
				m_axi.wvalid <= 0;
				m_axi.bready <= 1;
				writestate <= WRESP;
			end
		end

		WRESP: begin
			if (m_axi.bvalid) begin // && m_axi.bready
				m_axi.wvalid <= 0;
				m_axi.wstrb <= 16'h0000;
				m_axi.wlast <= 0;
				m_axi.bready <= 0;
				writestate <= WIDLE;
			end else begin
				writestate <= WRESP;
			end
		end
	endcase

	if (~aresetn) begin
		writestate <= WINIT;
	end
end*/

endmodule
