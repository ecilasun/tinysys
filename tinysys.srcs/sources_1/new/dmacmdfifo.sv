`timescale 1ns / 1ps

module dmacommanddevice(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	// Internal comms channel for DMA
	output wire fifoempty,
	output wire [31:0] fifodout,
	input wire fifore,
	output wire fifovalid,
    input wire dmabusy);

wire fifofull;
logic fifowe = 1'b0;
logic [31:0] fifodin = 'd0;

// Same size and shape as the GPU command ring
gpucmdring dmacmdfifoinst(
	.full(fifofull),
	.din(fifodin),
	.wr_en(fifowe),
	.empty(fifoempty),
	.dout(fifodout),
	.rd_en(fifore),
	.valid(fifovalid),
	.clk(aclk),
	.rst(~aresetn) );

logic waddrstate = 1'b0;
logic writestate = 1'b0;
logic raddrstate = 1'b0;

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

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.bresp <= 2'b00; // okay
		s_axi.bvalid <= 1'b0;
		s_axi.wready <= 1'b0;
		fifodin <= 32'd0;
		writestate <= 1'b0;
	end else begin
		fifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		unique case (writestate)
			1'b0: begin
				if (s_axi.wvalid && ~fifofull) begin
					fifodin <= s_axi.wdata[31:0];
					fifowe <= 1'b1;
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

// Reads from DMA command fifo will return DMA busy status

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
					s_axi.rdata[31:0] <= {31'd0, dmabusy};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 1'b0;
				end
			end
		endcase
	end
end

endmodule
