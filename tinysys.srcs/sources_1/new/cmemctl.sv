`timescale 1ns / 1ps
`default_nettype none

module cachedmemorycontroller (
	// Clock/reset
	input wire aclk,
	input wire aresetn,
	// Custom bus from cache controller
	input wire [31:0] addr,
	output data_t dout[0:3],
	input wire data_t din[0:3], // x4 128 bits
	input wire start_read,
	input wire start_write,
	output logic wdone,
	output logic rdone,
	// Memory device bus
	axi4if.master m_axi );

localparam burstlen = 4; // x4 128 bit reads or writes

// Write
assign m_axi.awlen = burstlen - 1;
assign m_axi.awsize = SIZE_16_BYTE; // 128bit write bus
assign m_axi.awburst = BURST_INCR;

typedef enum logic [2:0] {WINIT, WIDLE, WADDR, WDATA, WRESP} writestate_type;
writestate_type writestate = WINIT;

logic [2:0] wdata_cnt;

always_ff @(posedge aclk) begin

	if (~aresetn) begin
		m_axi.awvalid <= 0;
		m_axi.wvalid <= 0;
		m_axi.wstrb <= 16'h0000;
		m_axi.wlast <= 0;
		m_axi.bready <= 0;
		wdata_cnt <= 3'b000;
		writestate <= WINIT;
	end else begin
		wdone <= 1'b0;
	
		unique case(writestate)
			WINIT: begin
				//m_axi.wdata <= 0;
				writestate <= WIDLE;
			end
	
			WIDLE: begin
				if (start_write) begin
					m_axi.awaddr <= addr; // NOTE: MUST be 64 byte aligned! {[31:7], 6'd0}
					m_axi.awvalid <= 1;
				end
				writestate <= start_write ? WADDR : WIDLE;
			end
	
			WADDR: begin
				if (/*m_axi.awvalid &&*/ m_axi.awready) begin
					m_axi.awvalid <= 0;
					m_axi.wdata <= din[0];
					m_axi.wstrb <= 16'hFFFF;
					m_axi.wvalid <= 1;
					m_axi.wlast <= 1'b0;
					wdata_cnt <= 3'b001;
					writestate <= WDATA;
				end else begin
					writestate <= WADDR;
				end
			end
	
			WDATA: begin
				if (/*m_axi.wvalid &&*/ m_axi.wready) begin
					unique case (wdata_cnt)
						3'b001: m_axi.wdata <= din[1];
						3'b010: m_axi.wdata <= din[2];
						3'b100: m_axi.wdata <= din[3];
					endcase
					wdata_cnt <= {wdata_cnt[1:0], 1'b0};
					m_axi.wlast <= (wdata_cnt == 3'b100) ? 1'b1 : 1'b0;
					m_axi.bready <= (wdata_cnt == 3'b100) ? 1'b1 : 1'b0;
				end
				writestate <= (wdata_cnt == 3'b100) ? WRESP : WDATA;
			end
	
			WRESP: begin
				m_axi.wvalid <= 0;
				m_axi.wstrb <= 16'h0000;
				m_axi.wlast <= 0;
				if (m_axi.bvalid /*&& m_axi.bready*/) begin
					m_axi.bready <= 0;
					wdone <= 1'b1;
					writestate <= WIDLE;
				end else begin
					writestate <= WRESP;
				end
			end
		endcase
	end
end

// Read
assign m_axi.arlen = burstlen - 1;
assign m_axi.arsize = SIZE_16_BYTE; // 128bit read bus
assign m_axi.arburst = BURST_INCR;

typedef enum logic [2:0] {RINIT, RIDLE, RADDR, RDATA} readstate_type;
readstate_type readstate = RINIT;

logic [1:0] rdata_cnt;

always_ff @(posedge aclk) begin

	if (~aresetn) begin
		m_axi.arvalid <= 0;
		m_axi.rready <= 0;
		rdata_cnt <= 0;
		readstate <= RINIT;
	end else begin
		rdone <= 1'b0;
	
		unique case(readstate)
			RINIT: begin
				readstate <= RIDLE;
			end
	
			RIDLE : begin
				if (start_read) begin
					rdata_cnt <= 0;
					m_axi.araddr <= addr; // NOTE: MUST be 64 byte aligned! {[31:7], 6'd0}
					m_axi.arvalid <= 1;
				end
				readstate <= start_read ? RADDR : RIDLE;
			end
	
			RADDR : begin
				if (/*m_axi.arvalid && */m_axi.arready) begin
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					readstate <= RDATA;
				end else begin
					readstate <= RADDR;
				end
			end
	
			RDATA: begin
				if (m_axi.rvalid  /*&& m_axi.rready*/) begin
					dout[rdata_cnt] <= m_axi.rdata;
					rdata_cnt <= rdata_cnt + 1;
					m_axi.rready <= ~m_axi.rlast;
					rdone <= m_axi.rlast;
					readstate <= m_axi.rlast ? RIDLE : RDATA;
				end else begin
					readstate <= RDATA;
				end
			end
		endcase
	end
end

endmodule
