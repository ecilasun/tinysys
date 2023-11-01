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
logic [15:0] sramdout;
assign sramconn.sram_data_inout = sramwe ? sramdin : 16'bz;

// ----------------------------------------------------------------------------
// Main state machine
// ----------------------------------------------------------------------------

always @(posedge aclk) begin

	s_axi.awready <= 1'b0;

	if (s_axi.awvalid) begin
		s_axi.awready <= 1'b1;
	end

	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end
end

always @(posedge aclk) begin

	sramwe <= 1'b0;
	s_axi.wready <= 1'b0;
	s_axi.bvalid <= 1'b0;
	s_axi.bresp <= 2'b00;

	unique case (writestate)
		1'b0: begin
			if (s_axi.wvalid) begin
				sramwaddr <= s_axi.awaddr[17:0];
				sramdin <= s_axi.wdata[15:0];
				sramwe <= 1'b1;
				s_axi.wready <= 1'b1;
				writestate <= 1'b1;
			end
		end
		1'b1: begin
			if(s_axi.bready) begin
				s_axi.bvalid <= 1'b1;
				writestate <= 1'b0;
			end
		end
	endcase

	if (~aresetn) begin
		writestate <= 1'b0;
	end
end

always @(posedge aclk) begin

	sramre <= 1'b0;
	s_axi.arready <= 1'b0;
	s_axi.rvalid <= 1'b0;
	s_axi.rlast <= 1'b0;
	s_axi.rresp <= 2'b00;

	// read address
	unique case (raddrstate)
		2'b00: begin
			if (s_axi.arvalid) begin
				sramraddr <= s_axi.araddr[17:0];
				sramre <= 1'b1;
				s_axi.arready <= 1'b1;
				raddrstate <= 2'b01;
			end
		end
		2'b01: begin
			if (s_axi.rready) begin
				s_axi.rdata <= {16'd0, sramdout};
				s_axi.rvalid <= 1'b1;
				s_axi.rlast <= 1'b1;
				raddrstate <= 2'b00;
			end
		end
	endcase

	if (~aresetn) begin
		raddrstate <= 2'b00;
	end
end

endmodule


// --------------------------------------------------
// --------------------------------------------------

/*sramwires.def sramconn,

logic sramwe = 1'b0;
logic [15:0] sramout;
assign sramconn.sram_data_inout = sramwe ? sramout : 16'bz;

logic [1:0] sramstate = 2'b00;
logic [23:0] sramdout;

always @(posedge clk100) begin
	case (sramstate)
		2'b00: begin
			sramconn.sram_oe <= 1'b1;
			sramconn.sram_cen <= 1'b1;
			sramconn.sram_we <= 1'b1;
			{sramconn.sram_ub, sramconn.sram_lb} <= 2'b11;
			sramout <= 16'h0000;
			sramstate <= 2'b01;
		end
		2'b01: begin
			sramconn.sram_addr <= scanaddr[17:0] + scanoffset[17:0] + {8'd0, scanpixel};
			sramconn.sram_oe <= 1'b0;
			sramconn.sram_cen <= 1'b0;
			{sramconn.sram_ub, sramconn.sram_lb} <= 2'b00;
			sramstate <= 2'b10;
		end
		2'b10: begin
			sramconn.sram_oe <= 1'b1;
			sramconn.sram_cen <= 1'b1;
			{sramconn.sram_ub, sramconn.sram_lb} <= 2'b11;
			sramdout[15:0] <= sramconn.sram_data_inout;
			sramdout[23:16] <= 0;
			sramstate <= 2'b01;
		end
	endcase

	if (~aresetn) begin
		sramstate <= 2'b00;
	end
end*/
