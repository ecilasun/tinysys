`timescale 1ns / 1ps

module axi4opl2(
	input wire aclk,
	input wire aresetn,
	input wire audioclock,
	output wire [15:0] sampleout,
	axi4if.slave s_axi);

// ------------------------------------------------------------------------------------
// OPL2 fifo
// ------------------------------------------------------------------------------------

logic [8:0] opl2fifodin = 9'd0;
wire [8:0] opl2fifodout;
logic opl2fifowe = 1'b0;
logic opl2fifore = 1'b0;
wire opl2fifofull, opl2fifoempty, opl2fifovalid;

opl2fifo opl2fifoinst(
	.full(opl2fifofull),
	.din(opl2fifodin),
	.wr_en(opl2fifowe),
	.empty(opl2fifoempty),
	.dout(opl2fifodout),
	.rd_en(opl2fifore),
	.rst(~aresetn),
	.wr_clk(aclk),
	.rd_clk(audioclock),
	.valid(opl2fifovalid) );

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

// Output cdc
(* async_reg = "true" *) logic [7:0] opl2doutcdc = 1'b0;
always @(posedge aclk) begin
	opl2doutcdc <= opl2dout;
end

always @(posedge audioclock) begin

	sum <= sum + 28'd3579545;	// OPL2 uses 3.579545MHz, OPL3 uses 14.318180 MHz
	opl2ce <= 0;

	if(sum >= opl2_clk_rate) begin
		sum <= sum - opl2_clk_rate;
		opl2ce <= 1;
 	end
end

// OPL2 device

jtopl2 jtopl2_inst(
	.rst(~aresetn),
	.clk(audioclock),	// Master clock at 50MHz 
	.cen(opl2ce),		// OPL2 clock at 3.579545MHz
	.din(opl2din),
	.dout(opl2dout),
	.addr(opl2addr),
	.cs_n(opl2csn),
	.wr_n(opl2wen),
	.irq_n(),
	.snd(opl2sndout),
	.sample() );

assign sampleout = opl2sndout;

// Drive through fifo

always @(posedge audioclock) begin
	if (~aresetn) begin
		//
	end else begin

		opl2csn <= 1'b1;	// Release bus
		opl2wen <= 1'b1;	// Read
		opl2din <= 8'dz;	// Manual states we should set data to high impedance

		opl2fifore <= 1'b0;

		if (~opl2fifoempty && opl2fifovalid) begin// && opl2ce) begin
			opl2csn <= 1'b0;
			opl2wen <= 1'b0;
			opl2addr <= opl2fifodout[8];
			opl2din <= opl2fifodout[7:0];
			// Advance fifo
			opl2fifore <= 1'b1;
		end
	end
end

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

		opl2fifowe <= 1'b0;

		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;

		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && ~opl2fifofull) begin // && opl2ce) begin
					unique case (s_axi.awaddr[3:0])
						4'h0: begin
							// Will be latched on next CE
							// DO NO
							// CSn RDn WRn A0
							// 0   1   0   0	// write register address
							// Only stores A0 since the other two are zeroes
							opl2fifodin <= {1'b0,s_axi.wdata[7:0]};
							opl2fifowe <= 1'b1; 
						end
						4'h4: begin
							// CSn RDn WRn A0
							// 0   1   0   1	// write register value
							// Only stores A0 since the other two are zeroes
							opl2fifodin <= {1'b1,s_axi.wdata[7:0]};
							opl2fifowe <= 1'b1;
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
