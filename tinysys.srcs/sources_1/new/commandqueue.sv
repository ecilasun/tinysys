`timescale 1ns / 1ps

module commandqueue(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	// Internal comms channel for device
	output wire fifoempty,
	output wire [31:0] fifodout,
	input wire fifore,
	output wire fifovalid,
	// Busy state or a sync counter &c
    input wire [31:0] devicestate);

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// Device command FIFO
// --------------------------------------------------

wire fifofull;
logic fifowe;
logic [31:0] fifodin;

commandring cmdfifoinst(
	.full(fifofull),
	.din(fifodin),
	.wr_en(fifowe),
	.empty(fifoempty),
	.dout(fifodout),
	.rd_en(fifore),
	.valid(fifovalid),
	.clk(aclk),
	.rst(~delayedresetn) );

logic [1:0] waddrstate;
logic [1:0] writestate;
logic [1:0] raddrstate;

always @(posedge aclk) begin
	if (~delayedresetn) begin
		waddrstate <= 2'b00;
	end else begin
		unique case (waddrstate)
			2'b00: begin
				s_axi.awready <= 1'b0;
				waddrstate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.awvalid) begin
					s_axi.awready <= 1'b1;
					waddrstate <= 2'b10;
				end
			end
			2'b10: begin
				s_axi.awready <= 1'b0;
				waddrstate <= 2'b01;
			end
		endcase
	end
end

// Writes will queue up a command in the command FIFO the device can read

always @(posedge aclk) begin

	if (~delayedresetn) begin
		fifowe <= 1'b0;
		writestate <= 2'b00;
		fifodin <= 32'd0;
	end else begin
		fifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
	
		unique case (writestate)
			2'b00: begin
				s_axi.bresp <= 2'b00; // okay
				s_axi.bvalid <= 1'b0;
				s_axi.wready <= 1'b0;
				writestate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.wvalid && ~fifofull) begin
					fifodin <= s_axi.wdata[31:0];
					fifowe <= 1'b1;
					writestate <= 2'b10;
					s_axi.wready <= 1'b1;
				end
			end
			2'b10: begin
				if (s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b01;
				end
			end
		endcase
	end
end

// Reads from command fifo will return externally supplied device status

always @(posedge aclk) begin
	if (~delayedresetn) begin
		raddrstate <= 2'b00;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;

		unique case (raddrstate)
			2'b00: begin
				s_axi.rlast <= 1'b1;
				s_axi.arready <= 1'b0;
				s_axi.rvalid <= 1'b0;
				s_axi.rresp <= 2'b00;
				raddrstate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					raddrstate <= 2'b10;
				end
			end
			2'b10: begin
				if (s_axi.rready) begin
					s_axi.rdata[31:0] <= devicestate;
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
		endcase
	end
end

endmodule
