module axi4sram(
	input wire aclk,
	input wire clk100,
	input wire aresetn,
	axi4if.slave s_axi,
	sramwires.def sramconn);

logic sramwe = 1'b0;
logic sramre = 1'b0;
logic [17:0] sramwaddr;
logic [17:0] sramraddr;
logic [15:0] sramdin;

assign sramconn.sram_we = sramwe ? 1'b0 : 1'b1;
assign sramconn.sram_oe = sramre ? 1'b0 : 1'b1;
assign sramconn.sram_ce = (sramwe || sramre) ? 1'b0 : 1'b1;
assign sramconn.sram_ub = (sramwe || sramre) ? 1'b0 : 1'b1;
assign sramconn.sram_lb = (sramwe || sramre) ? 1'b0 : 1'b1;
assign sramconn.sram_addr = sramwe ? sramwaddr : sramraddr;
assign sramconn.sram_data_inout = sramwe ? sramdin : 16'bz;

// ----------------------------------------------------------------------------
// Retimer to 100MHz SRAM clock
// ----------------------------------------------------------------------------

wire aresetm;
axi4if m_axi();
axi4retimer axi4retimerinst(
	.aresetn(aresetn),
	.srcclk(aclk),
	.srcbus(s_axi),
	.destclk(clk100),
	.destbus(m_axi),
	.destrst(aresetm) );

// ----------------------------------------------------------------------------
// Main state machine
// ----------------------------------------------------------------------------

logic writestate = 1'b0;
logic [1:0] raddrstate = 2'b00;

always @(posedge clk100) begin

	m_axi.awready <= 1'b0;

	if (m_axi.awvalid) begin
		m_axi.awready <= 1'b1;
	end

	if (~aresetn) begin
		m_axi.awready <= 1'b0;
	end
end

always @(posedge clk100) begin

	m_axi.wready <= 1'b0;
	m_axi.bvalid <= 1'b0;
	m_axi.bresp <= 2'b00;
	sramwe <= 1'b0;

	unique case (writestate)
		1'b0: begin
			if (m_axi.wvalid) begin
				sramwaddr <= m_axi.awaddr[18:1]; // 2 byte aligned
				sramdin <= m_axi.wdata[15:0]; // always assume 16bits (don't use m_axi.wstrb[1:0])
				sramwe <= 1'b1;
				m_axi.wready <= 1'b1;
				writestate <= 1'b1;
			end
		end
		1'b1: begin
			if(m_axi.bready) begin
				m_axi.bvalid <= 1'b1;
				writestate <= 1'b0;
			end
		end
	endcase

	if (~aresetm) begin
		writestate <= 1'b0;
	end
end

// Captured data out on clock following request
logic [15:0] datacap;

always @(posedge clk100) begin

	m_axi.arready <= 1'b0;
	m_axi.rvalid <= 1'b0;
	m_axi.rlast <= 1'b0;
	m_axi.rresp <= 2'b00;
	sramre <= 1'b0;

	if (sramre)
		datacap <= sramconn.sram_data_inout;

	// read address
	unique case (raddrstate)
		2'b00: begin
			if (m_axi.arvalid) begin
				sramraddr <= m_axi.araddr[18:1]; // 2 byte aligned
				sramre <= 1'b1;
				m_axi.arready <= 1'b1;
				raddrstate <= 2'b01;
			end
		end
		2'b01: begin
			if (m_axi.rready) begin
				m_axi.rdata <= {16'd0, datacap};
				m_axi.rvalid <= 1'b1;
				m_axi.rlast <= 1'b1;
				raddrstate <= 2'b00;
			end
		end
	endcase

	if (~aresetm) begin
		raddrstate <= 2'b00;
	end
end

endmodule
