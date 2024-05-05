`timescale 1ns / 1ps

import axi4pkg::*;

module axi4usbc(
	input wire aclk,
	input wire spibaseclock,
	input wire aresetn,
	output wire usbirq,
    max3420wires.def usbcconn,
	axi4if.slave s_axi);

logic writestate;
logic [1:0] raddrstate;

logic [7:0] writedata;
logic we;

// ----------------------------------------------------------------------------
// GPX cdc from usb to aclk domain and other usb wiring
// ----------------------------------------------------------------------------

// cdc for usb interrupt line
(* async_reg = "true" *) logic usbgpxcdcA;
(* async_reg = "true" *) logic usbgpxcdcB;

// cdc for usb interrupt line
(* async_reg = "true" *) logic usbirqcdcA;
(* async_reg = "true" *) logic usbirqcdcB;

// cdc for outfifoempty line
(* async_reg = "true" *) logic outfifoemptycdcA;
(* async_reg = "true" *) logic outfifoemptycdcB;

wire outfifoempty; // Used after this section in output fifo

always @(posedge aclk) begin
	if (~aresetn) begin
		usbgpxcdcA <= 1'b0;
		usbgpxcdcB <= 1'b0;
		usbirqcdcA <= 1'b0;
		usbirqcdcB <= 1'b0;
		outfifoemptycdcA <= 1'b0;
		outfifoemptycdcB <= 1'b0;
	end else begin
		usbgpxcdcA <= usbcconn.gpx;
		usbgpxcdcB <= usbgpxcdcA;
		usbirqcdcA <= usbcconn.irq;
		usbirqcdcB <= usbirqcdcA;
		outfifoemptycdcA <= outfifoempty;
		outfifoemptycdcB <= outfifoemptycdcA;
	end
end

assign usbirq = ~usbirqcdcB;
assign usbcconn.resn = aresetn;	// Low during reset

// ----------------------------------------------------------------------------
// spi master device
// ----------------------------------------------------------------------------

wire cansend;

wire hasvaliddata;
wire [7:0] spiincomingdata;

logic cs_n;
assign usbcconn.cs_n = cs_n;

SPI_Master usbspi(
   // control/data signals,
   .i_Rst_L(aresetn),
   .i_Clk(spibaseclock),

   // tx (mosi) signals
   .i_TX_Byte(writedata),
   .i_TX_DV(we),
   .o_TX_Ready(cansend),

   // rx (miso) signals
   .o_RX_DV(hasvaliddata),
   .o_RX_Byte(spiincomingdata),

   // spi interface
   .o_SPI_Clk(usbcconn.clk),
   .i_SPI_MISO(usbcconn.miso),
   .o_SPI_MOSI(usbcconn.mosi) );

wire infifofull, infifoempty, infifovalid;
logic infifowe;
logic infifore;
logic [7:0] infifodin;
wire [7:0] infifodout;

spimasterinfifo usbspiinputfifo(
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
	if (~aresetn) begin
		infifowe <= 1'b0;
		infifodin <= 8'd0;
	end else begin
		infifowe <= 1'b0;
		if (hasvaliddata & (~infifofull)) begin // make sure to drain the fifo!
			// stash incoming byte in fifo
			infifowe <= 1'b1;
			infifodin <= spiincomingdata;
		end
	end
end

wire outfifofull, outfifovalid;
logic outfifowe, outfifore;
logic [8:0] outfifodin;
wire [8:0] outfifodout;

usbcspififo usboutputfifo(
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
	if (~aresetn) begin
		cs_n <= 1'b1;
		writedata <= 8'd0;
		outfifore <= 1'b0;
		we <= 1'b0;
	end else begin
		outfifore <= 1'b0;
		we <= 1'b0;
		if ((~outfifoempty) && outfifovalid && cansend) begin
			unique case (outfifodout[8])
				1'b0: begin
					// Control output data
					writedata <= outfifodout[7:0];
					we <= 1'b1;
				end
				1'b1: begin
					// Control CSn
					cs_n <= outfifodout[0];
				end
			endcase
			// Advance FIFO
			outfifore <= 1'b1;
		end
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
		writestate <= 1'b0;
		outfifowe <= 1'b0;
	end else begin
		outfifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		s_axi.bresp <= 2'b00;
		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && (~outfifofull)) begin
					// SPI 0x8000?000
					outfifodin <= s_axi.wdata[8:0];
					outfifowe <= 1'b1;
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
		raddrstate <= 2'b00;
		infifore <= 1'b0;
	end else begin
		infifore <= 1'b0;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rlast <= 1'b0;
		s_axi.rresp <= 2'b00;
		// read address
		unique case (raddrstate)
			2'b00: begin
				if (s_axi.arvalid) begin
					unique case (s_axi.araddr[3:0])
						4'h0: raddrstate <= 2'b01; // SPI        0x8000?000
						4'h4: raddrstate <= 2'b10; // FIFO state 0x8000?004
					endcase
					s_axi.arready <= 1'b1;
				end
			end
			2'b01: begin
				// master ready to accept and fifo has incoming data
				if (s_axi.rready && (~infifoempty) && infifovalid) begin
					s_axi.rdata <= {infifodout, infifodout, infifodout, infifodout};
					s_axi.rvalid <= 1'b1;
					s_axi.rlast <= 1'b1;
					// Advance FIFO
					infifore <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
			2'b10: begin
				if (s_axi.rready) begin
					s_axi.rdata <= {29'd0, usbgpxcdcB, ~outfifoemptycdcB, ~infifoempty};
					s_axi.rvalid <= 1'b1;
					s_axi.rlast <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
