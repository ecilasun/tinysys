`timescale 1ns / 1ps

module axi4opl2(
	input wire aclk,
	input wire aresetn,
	input wire audioclock,
	output wire [15:0] sampleout,
	axi4if.slave s_axi);

// TODO: CDC?

// ------------------------------------------------------------------------------------
// OPL2 hardware
// ------------------------------------------------------------------------------------

// Control & output wires

logic opl2csn = 1'b1;
logic opl2addr = 1'b0;
logic opl2wen = 1'b1;
logic [7:0] opl2din = 8'd0;
wire [7:0] opl2dout;
wire [15:0] opl2sndout;

localparam [27:0] opl2_clk_rate = 28'd50000000;

logic opl2ce = 1'b0;
logic [27:0] sum = 0;

always @(posedge audioclock) begin

	sum <= sum + 28'd3579545;
	opl2ce <= 0;

	if(sum >= opl2_clk_rate) begin
		sum <= sum - opl2_clk_rate;
		opl2ce <= 1;
	end
end

// OPL2 device

(* async_reg = "true" *) logic opl2dwencdc = 1'b1;
(* async_reg = "true" *) logic opl2addrcdc = 1'b0;
(* async_reg = "true" *) logic opl2dcsncdc = 1'b1;
(* async_reg = "true" *) logic [7:0] opl2dincdc = 1'b0;

always @(posedge audioclock) begin
	opl2dincdc <= opl2din;
	opl2addrcdc <= opl2addr;
	opl2dcsncdc <= opl2csn;
	opl2dwencdc <= opl2wen;
end

(* async_reg = "true" *) logic [7:0] opl2doutcdc = 1'b0;
always @(posedge aclk) begin
	opl2doutcdc <= opl2dout;
end

jtopl2 jtopl2_inst(
	.rst(~aresetn),
	.clk(audioclock),
	.cen(opl2ce),
	.din(opl2dincdc),
	.dout(opl2dout),
	.addr(opl2addrcdc),
	.cs_n(opl2dcsncdc),
	.wr_n(opl2dwencdc),
	.irq_n(),
	.snd(opl2sndout),
	.sample() );

assign sampleout = opl2sndout;

// ------------------------------------------------------------------------------------
// AXI4 interface
// ------------------------------------------------------------------------------------

logic waddrstate = 1'b0;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
		waddrstate <= 1'b0;
	end else begin
		// write address
		unique case (waddrstate)
			1'b0: begin
				if (s_axi.awvalid) begin
					s_axi.awready <= 1'b1;
					waddrstate <= 1'b1;
				end
			end
			1'b1: begin
				s_axi.awready <= 1'b0;
				waddrstate <= 1'b0;
			end
		endcase
	end
end

logic writestate = 1'b0;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.bresp <= 2'b00; // okay
		s_axi.bvalid <= 1'b0;
		s_axi.wready <= 1'b0;
		writestate <= 1'b0;
	end else begin

		opl2csn <= 1'b1;	// Release bus
		opl2wen <= 1'b1;	// Read
		opl2din <= 8'dz;	// Manual states we should set data to high impedance 

		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;

		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid) begin // && opl2ce) begin
					unique case (s_axi.awaddr[3:0])
						4'h0: begin
							// CSn RDn WRn A0
							// 0   1   0   0	// write register address
							opl2csn <= 1'b0;
							opl2wen <= 1'b0;
							opl2addr <= 1'b0;
							opl2din <= s_axi.wdata[7:0];
						end
						4'h4: begin
							// CSn RDn WRn A0
							// 0   1   0   1	// write register value
							opl2csn <= 1'b0;
							opl2wen <= 1'b0;
							opl2addr <= 1'b1;
							opl2din <= s_axi.wdata[7:0];
						end
					endcase
					writestate <= 1'b1;
					s_axi.wready <= 1'b1;
				end
			end
			1'b1: begin
				if (s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 1'b0;
				end
			end
		endcase
	end
end

logic raddrstate = 1'b0;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.rlast <= 1'b1;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
		raddrstate <= 1'b0;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		unique case (raddrstate)
			1'b0: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					raddrstate <= 1'b1;
				end
			end
			1'b1: begin
				if (s_axi.rready) begin
					// OPL2 state
					s_axi.rdata[31:0] <= {24'd0, opl2doutcdc};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 1'b0;
				end
			end
		endcase
	end
end

endmodule
