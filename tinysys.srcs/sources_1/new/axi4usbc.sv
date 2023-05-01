`timescale 1ns / 1ps

import axi4pkg::*;

module axi4usbc(
	input wire aclk,
	input wire spibaseclock,
	input wire aresetn,
	output wire usbirq,
    max3420wires.def usbcconn,
	axi4if.slave s_axi);

logic csn = 1'b1;
assign usbcconn.cs_n = csn;
assign usbcconn.resn = aresetn;	// Low during reset
assign usbirq = ~usbcconn.irq;

logic writestate = 1'b0;
logic raddrstate = 1'b0;

logic [7:0] writedata = 7'd0;
wire [7:0] readdata;
logic we = 1'b0;

// ----------------------------------------------------------------------------
// spi master device
// ----------------------------------------------------------------------------

wire cansend;

wire hasvaliddata;
wire [7:0] spiincomingdata;

// If base clock is @100mhz, we're running at 25mhz (2->2x2 due to 'half'->50/4==12.5MHz)
// If base clock is @50mhz, we're running at 12.5mhz (2->2x2 due to 'half'->50/4==6.25MHz)
// If base clock is @25mhz, we're running at 6.25mhz (2->2x2 due to 'half'->25/4==3.125MHz)

spi_master #(.spi_mode(0), .clks_per_half_bit(2)) usbcspi(
   // control/data signals,
   .i_rst_l(aresetn),
   .i_clk(spibaseclock),

   // tx (mosi) signals
   .i_tx_byte(writedata),
   .i_tx_dv(we),
   .o_tx_ready(cansend),

   // rx (miso) signals
   .o_rx_dv(hasvaliddata),
   .o_rx_byte(spiincomingdata),

   // spi interface
   .o_spi_clk(usbcconn.clk),
   .i_spi_miso(usbcconn.miso),
   .o_spi_mosi(usbcconn.mosi) );

wire infifofull, infifoempty, infifovalid;
logic infifowe = 1'b0, infifore = 1'b0;
logic [7:0] infifodin = 8'h00;
wire [7:0] infifodout;

spimasterinfifo usbcspiinputfifo(
	.wr_clk(spibaseclock),
	.full(infifofull),
	.din(infifodin),
	.wr_en(infifowe),

	.rd_clk(aclk),
	.empty(infifoempty),
	.dout(infifodout),
	.rd_en(infifore),
	.valid(infifovalid),

	.rst(~aresetn) );

always @(posedge spibaseclock) begin
	infifowe <= 1'b0;
	if (hasvaliddata & (~infifofull)) begin // make sure to drain the fifo!
		// stash incoming byte in fifo
		infifowe <= 1'b1;
		infifodin <= spiincomingdata;
	end
end

wire outfifofull, outfifoempty, outfifovalid;
logic outfifowe = 1'b0, outfifore = 1'b0;
logic [8:0] outfifodin = 9'h00;
wire [8:0] outfifodout;

usbcspififo usbcoutputfifo(
	.wr_clk(aclk),
	.full(outfifofull),
	.din(outfifodin),
	.wr_en(outfifowe),

	.rd_clk(spibaseclock),
	.empty(outfifoempty),
	.dout(outfifodout),
	.rd_en(outfifore),
	.valid(outfifovalid),

	.rst(~aresetn) );

always @(posedge spibaseclock) begin
	outfifore <= 1'b0;
	we <= 1'b0;
	if ((~outfifoempty) && outfifovalid && cansend) begin
		if (outfifodout[8]) begin // CS control
			csn <= outfifodout[0]; // Only set when 'cansend' is high i.e. comms ended
		end else begin
			writedata <= outfifodout;
			we <= 1'b1;
		end
		// Advance FIFO
		outfifore <= 1'b1;
	end
end

// ----------------------------------------------------------------------------
// main state machine
// ----------------------------------------------------------------------------

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		s_axi.awready <= 1'b0;
		if (s_axi.awvalid) begin
			s_axi.awready <= 1'b1;
		end
	end
end

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.bresp = 2'b00;
	end else begin

		outfifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		outfifodin <= 9'h00;

		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && (~outfifofull)) begin
					unique case (s_axi.awaddr[3:0])
						4'h0: begin
							outfifodin <= {1'b0,s_axi.wdata[7:0]}; // Control command
							outfifowe <= 1'b1;
						end
						4'h4: begin
							outfifodin <= {8'b10000000,s_axi.wdata[0]}; // Control CSn
							outfifowe <= 1'b1;
						end
					endcase
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
	end
end

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.rlast <= 1'b1;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
		s_axi.rdata <= 32'd0;
	end else begin

		infifore <= 1'b0;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;

		// read address
		unique case (raddrstate)
			1'b0: begin
				if (s_axi.arvalid) begin
					raddrstate <= 1'b1;
					s_axi.arready <= 1'b1;
				end
			end
			1'b1: begin
				// master ready to accept and fifo has incoming data
				if (s_axi.rready && (~infifoempty) && infifovalid) begin
					s_axi.rdata <= {infifodout, infifodout, infifodout, infifodout};
					s_axi.rvalid <= 1'b1;
					// Advance FIFO
					infifore <= 1'b1;
					raddrstate <= 1'b0;
				end
			end
		endcase
	end
end

endmodule
