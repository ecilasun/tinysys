`timescale 1ns / 1ps

import axi4pkg::*;

module axi4sdcard(
	input wire aclk,
	input wire clk10,
	input wire spibaseclock,
	input wire aresetn,
	input wire rst50n,
	output wire keyirq,
	sdcardwires.def sdconn,
	axi4if.slave s_axi);

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// Internal
// --------------------------------------------------

// Keep attached spi device selected (TODO: Drive via control register)
assign sdconn.cs_n = 1'b0;

logic [1:0] writestate;
logic [1:0] raddrstate;

logic [7:0] writedata;
logic we;

// --------------------------------------------------
// SDCard insert/remove switch CDC, debounce and FIFO
// --------------------------------------------------

logic prevswtch;
wire stableswtch;
debounce sdswtchdebounce(
	.clk(clk10),
	.reset(~delayedresetn),
	.bouncy(sdconn.swtch),
	.stable(stableswtch) );

(* async_reg = "true" *) logic swtchA = 1'b1;
(* async_reg = "true" *) logic swtchB = 1'b1;
always @(posedge clk10) begin
	swtchA <= stableswtch;
	swtchB <= swtchA;
end

// Reading this fifo will clear the sd card switch interrupt
wire keyfifofull, keyfifovalid;
wire keyfifoempty;
wire keyfifodout;
logic keyfifodin;
logic keyfifowe = 1'b0;
logic keyfifore = 1'b0;
bitfifo keyfifo(
	.full(keyfifofull),
	.din(keyfifodin),
	.wr_en(keyfifowe),
	.empty(keyfifoempty),
	.dout(keyfifodout),
	.rd_en(keyfifore),
	.rst(~delayedresetn),
	.wr_clk(clk10),
	.rd_clk(aclk),
	.valid(keyfifovalid));

// Store one enry only when switch state changes 
always @(posedge clk10) begin
	if (~delayedresetn) begin
		keyfifowe <= 1'b0;
		prevswtch = 1'b0;
	end else begin
		keyfifowe <= 1'b0;
		if (swtchB != prevswtch) begin
			keyfifodin <= swtchB;
			keyfifowe <= 1'b1;
		end
		prevswtch <= swtchB;
	end
end

// ----------------------------------------------------------------------------
// spi master device
// ----------------------------------------------------------------------------

wire cansend;

wire hasvaliddata;
wire [7:0] spiincomingdata;

SPI_Master sdcardspi(
   // control/data signals,
   .i_Rst_L(rst50n),
   .i_Clk(spibaseclock),

   // tx (mosi) signals
   .i_TX_Byte(writedata),
   .i_TX_DV(we),
   .o_TX_Ready(cansend),

   // rx (miso) signals
   .o_RX_DV(hasvaliddata),
   .o_RX_Byte(spiincomingdata),

   // spi interface
   .o_SPI_Clk(sdconn.clk),
   .i_SPI_MISO(sdconn.miso),
   .o_SPI_MOSI(sdconn.mosi));

wire infifofull, infifoempty, infifovalid;
logic infifowe = 1'b0, infifore = 1'b0;
logic [7:0] infifodin = 8'h00;
wire [7:0] infifodout;

spimasterinfifo sdcardspiinputfifo(
	.wr_clk(spibaseclock),
	.full(infifofull),
	.din(infifodin),
	.wr_en(infifowe),

	.rd_clk(aclk),
	.empty(infifoempty),
	.dout(infifodout),
	.rd_en(infifore),
	.valid(infifovalid),

	.rst(~delayedresetn) );

always @(posedge spibaseclock) begin
	if (~rst50n) begin
		infifowe <= 1'b0;
	end else begin
		infifowe <= 1'b0;
		if (hasvaliddata & (~infifofull)) begin // make sure to drain the fifo!
			// stash incoming byte in fifo
			infifowe <= 1'b1;
			infifodin <= spiincomingdata;
		end
	end
end

wire outfifofull, outfifoempty, outfifovalid;
logic outfifowe;
logic outfifore = 1'b0;
logic [7:0] outfifodin;
wire [7:0] outfifodout;

spimasteroutfifo sdcardoutputfifo(
	.wr_clk(aclk),
	.full(outfifofull),
	.din(outfifodin),
	.wr_en(outfifowe),

	.rd_clk(spibaseclock),
	.empty(outfifoempty),
	.dout(outfifodout),
	.rd_en(outfifore),
	.valid(outfifovalid),

	.rst(~delayedresetn) );

always @(posedge spibaseclock) begin
	if (~rst50n) begin
		outfifore <= 1'b0;
		we <= 1'b0;
		writedata = 8'd0;
	end else begin
		outfifore <= 1'b0;
		we <= 1'b0;
		if ((~outfifoempty) && outfifovalid && cansend) begin
			writedata <= outfifodout;
			we <= 1'b1;
			// Advance FIFO
			outfifore <= 1'b1;
		end
	end
end

// ----------------------------------------------------------------------------
// main state machine
// ----------------------------------------------------------------------------

always @(posedge aclk) begin
	if (~delayedresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		s_axi.awready <= 1'b0;
		if (s_axi.awvalid) begin
			s_axi.awready <= 1'b1;
		end
	end
end

always @(posedge aclk) begin
	if (~delayedresetn) begin
		writestate <= 2'b00;
	end else begin
		outfifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		outfifodin <= 8'h00;
	
		unique case (writestate)
			2'b00: begin
				s_axi.bresp = 2'b00;
				outfifodin <= 8'd0;
				outfifowe <= 1'b0;
				writestate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.wvalid && (~outfifofull)) begin
					outfifodin <= s_axi.wdata[7:0];
					outfifowe <= 1'b1; // (|s_axi.wstrb)
					s_axi.wready <= 1'b1;
					writestate <= 2'b10;
				end
			end
			2'b10: begin
				if(s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b01;
				end
			end
		endcase
	end
end

always @(posedge aclk) begin
	if (~delayedresetn) begin
		raddrstate <= 2'b00;
	end else begin
		infifore <= 1'b0;
		keyfifore <= 1'b0;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
	
		// read address
		unique case (raddrstate)
			2'b00: begin
				s_axi.rlast <= 1'b1;
				s_axi.arready <= 1'b0;
				s_axi.rvalid <= 1'b0;
				s_axi.rresp <= 2'b00;
				s_axi.rdata <= 32'd0;
				raddrstate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.arvalid) begin
					unique case (s_axi.araddr[2])
						2'b0: raddrstate <= 2'b11;	// SPI i/o at offset 0x0
						2'b1: raddrstate <= 2'b10;	// Switch state at offset 0x4
					endcase
					s_axi.arready <= 1'b1;
				end
			end
			2'b10: begin
				if (s_axi.rready && ~keyfifoempty && keyfifovalid) begin
					s_axi.rdata <= {31'd0, ~keyfifodout};
					s_axi.rvalid <= 1'b1;
					// Advance FIFO
					keyfifore <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
			2'b11: begin
				// master ready to accept and fifo has incoming data
				if (s_axi.rready && (~infifoempty) && infifovalid) begin
					s_axi.rdata <= {infifodout, infifodout, infifodout, infifodout};
					s_axi.rvalid <= 1'b1;
					// Advance FIFO
					infifore <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
		endcase
	end
end

(* async_reg = "true" *) logic keyirqA = 1'b0;
(* async_reg = "true" *) logic keyirqB = 1'b0;

always_ff @(posedge aclk) begin
	keyirqA <= (~keyfifoempty) && keyfifovalid;
	keyirqB <= keyirqA;
end

assign keyirq = keyirqB;

endmodule
