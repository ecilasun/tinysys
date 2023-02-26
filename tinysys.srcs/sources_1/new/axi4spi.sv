`timescale 1ns / 1ps

import axi4pkg::*;

module axi4spi(
	input wire aclk,
	input wire aresetn,
	input wire spibaseclock,
    sdwires.def sdconn,
	axi4if.slave s_axi);

assign sdconn.spi_cs_n = 1'b0; // Keep attached spi device selected (TODO: Drive via control register)

logic [1:0] writestate = 2'b00;
logic [1:0] raddrstate = 2'b00;

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

spi_master #(.spi_mode(0), .clks_per_half_bit(2)) spi(
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
   .o_spi_clk(sdconn.spi_clk),
   .i_spi_miso(sdconn.spi_miso),
   .o_spi_mosi(sdconn.spi_mosi) );

wire infifofull, infifoempty, infifovalid;
logic infifowe = 1'b0, infifore = 1'b0;
logic [7:0] infifodin = 8'h00;
wire [7:0] infifodout;

spimasterinfifo spiinputfifo(
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
logic [7:0] outfifodin = 8'h00;
wire [7:0] outfifodout;

spimasteroutfifo spioutputfifo(
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

	if ((~outfifoempty) & cansend) begin
		outfifore <= 1'b1;
	end

	if (outfifovalid) begin
		writedata <= outfifodout;
		we <= 1'b1;
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
		// write data
		outfifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		case (writestate)
			2'b00: begin
				if (s_axi.wvalid & (~outfifofull)) begin
					outfifodin <= s_axi.wdata[7:0];
					outfifowe <= 1'b1; // (|s_axi.wstrb)
					s_axi.wready <= 1'b1;
					writestate <= 2'b01;
				end
			end
			default/*2'b01*/: begin
				if(s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b00;
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
		case (raddrstate)
			2'b00: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
			2'b01: begin
				// master ready to accept and fifo has incoming data
				if (s_axi.rready & (~infifoempty)) begin
					infifore <= 1'b1;
					raddrstate <= 2'b10;
				end
			end
			default/*2'b10*/: begin
				if (infifovalid) begin
					s_axi.rdata <= {infifodout, infifodout, infifodout, infifodout};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
