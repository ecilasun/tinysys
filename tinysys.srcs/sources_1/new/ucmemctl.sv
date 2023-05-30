`timescale 1ns / 1ps

module uncachedmemorycontroller (
	// Clock/reset
	input wire aclk,
	input wire aresetn,
	// Custom bus from cache controller
	input wire [31:0] addr,
	output logic [31:0] dout,
	input wire [31:0] din,
	input wire [3:0] wstrb,
	input wire re,
	output logic wdone,
	output logic rdone,
	// Memory mapped device bus
	axi4if.master m_axi );

	// Write
	assign m_axi.awlen = 0;
	assign m_axi.awsize = SIZE_4_BYTE;
	assign m_axi.awburst = BURST_FIXED;
	logic [3:0] wbsel = 4'h0;

	typedef enum logic [2:0] {WINIT, WIDLE, WADDR, WDATA, WRESP} writestate_type;
	writestate_type wstate = WINIT;

	always_ff @(posedge aclk) begin
		wdone <= 1'b0;

		unique case(wstate)
			WINIT: begin
				m_axi.awvalid <= 0;
				m_axi.wvalid <= 0;
				m_axi.wstrb <= 16'h0000;
				m_axi.wlast <= 0;
				m_axi.bready <= 0;
				wstate <= WIDLE;
			end

			WIDLE: begin
				if (|wstrb) begin
					wbsel <= wstrb;
					m_axi.awaddr <= addr;
					m_axi.awvalid <= 1;
				end
				wstate <= (|wstrb) ? WADDR : WIDLE;
			end

			WADDR: begin
				if (/*m_axi.awvalid &&*/ m_axi.awready) begin
					m_axi.awvalid <= 0;
					m_axi.wdata <= {96'd0, din};
					m_axi.wstrb <= {12'd0, wbsel};
					m_axi.wvalid <= 1;
					m_axi.wlast <= 1;
					wstate <= WDATA;
				end else begin
					wstate <= WADDR;
				end
			end

			WDATA: begin
				if (/*m_axi.wvalid &&*/ m_axi.wready) begin
					m_axi.bready <= 1;
					m_axi.wvalid <= 0;
					m_axi.wstrb <= 16'h0000;
					m_axi.wlast <= 0;
					wstate <= WRESP;
				end else begin
					wstate <= WDATA;
				end
			end

			WRESP: begin
				if (m_axi.bvalid /*&& m_axi.bready*/) begin
					m_axi.bready <= 0;
					wdone <= 1'b1;
					wstate <= WIDLE;
				end else begin
					wstate <= WRESP;
				end
			end
		endcase

		if (~aresetn) begin
			wstate <= WINIT;
		end
	end

	// Read
	assign m_axi.arlen = 0;
	assign m_axi.arsize = SIZE_4_BYTE;
	assign m_axi.arburst = BURST_FIXED;

	typedef enum logic [2:0] {RINIT, RIDLE, RADDR, RDATA} readstate_type;
	readstate_type rstate = RINIT;

	always_ff @(posedge aclk) begin
		rdone <= 1'b0;

		unique case(rstate)
			RINIT: begin
				m_axi.arvalid <= 0;
				m_axi.rready <= 0;
				rstate <= RIDLE;
			end

			RIDLE: begin
				if (re) begin
					m_axi.araddr  <= addr;
					m_axi.arvalid <= 1;
				end
				rstate <= re ? RADDR : RIDLE;
			end

			RADDR: begin
				if (/*m_axi.arvalid && */m_axi.arready) begin
					m_axi.arvalid <= 0;
					m_axi.rready <= 1;
					rstate <= RDATA;
				end else begin
					rstate <= RADDR;
				end
			end

			RDATA: begin
				if (m_axi.rvalid  /*&& m_axi.rready*/ && m_axi.rlast) begin
					dout <= m_axi.rdata[31:0];
					m_axi.rready <= 0;
					rdone <= 1'b1;
					rstate <= RIDLE;
				end else begin
					rstate <= RDATA;
				end
			end

		endcase

		if (~aresetn) begin
			rstate <= RINIT;
		end
	end
endmodule
