`timescale 1ns / 1ps

module gpucommanddevice(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	// Internal comms channel for GPU
	output wire fifoempty,
	output wire [31:0] fifodout,
	input wire fifore,
	output wire fifovalid,
	input wire [31:0] vblankcount);

wire fifofull;
logic fifowe = 1'b0;
logic [31:0] fifodin = 'd0;

gpucmdring gpucmdfifoinst(
	.full(fifofull),
	.din(fifodin),
	.wr_en(fifowe),
	.empty(fifoempty),
	.dout(fifodout),
	.rd_en(fifore),
	.valid(fifovalid),
	.clk(aclk),
	.rst(~aresetn) );

logic [1:0] waddrstate = 2'b00;
logic [1:0] writestate = 2'b00;
logic [1:0] raddrstate = 2'b00;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		// write address
		case (waddrstate)
			2'b00: begin
				if (s_axi.awvalid) begin
					s_axi.awready <= 1'b1;
					waddrstate <= 2'b01;
				end
			end
			default/*2'b01*/: begin
				s_axi.awready <= 1'b0;
				waddrstate <= 2'b00;
			end
		endcase
	end
end

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.bresp <= 2'b00; // okay
		s_axi.bvalid <= 1'b0;
		s_axi.wready <= 1'b0;
	end else begin
		fifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		case (writestate)
			2'b00: begin
				if (s_axi.wvalid && ~fifofull) begin
					fifodin <= s_axi.wdata[31:0];
					fifowe <= 1'b1;
					writestate <= 2'b01;
					s_axi.wready <= 1'b1;
				end
			end
			default/*2'b01*/: begin
				if (s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b00;
				end
			end
		endcase
	end
end

// Reads from GPU command fifo will return vblankcount
// This can be used to implement a vsync mechanism by
// comparing a first read to successive reads until they differ.

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.rlast <= 1'b1;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		case (raddrstate)
			2'b00: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
			default/*2'b01*/: begin
				if (s_axi.rready) begin
					s_axi.rdata[31:0] <= vblankcount;
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
