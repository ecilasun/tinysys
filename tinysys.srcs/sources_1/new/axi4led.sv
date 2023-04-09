`timescale 1ns / 1ps

module axi4led(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	output wire [4:0] led );

logic ledwe = 1'b0;
logic [4:0] ledstate = 5'd0;

logic [4:0] ledbits = 5'd0;
always @(posedge aclk) begin
	if (ledwe) ledbits <= ledstate;
end

assign led = ledbits;

logic [1:0] waddrstate = 2'b00;
logic [1:0] writestate = 2'b00;
logic [1:0] raddrstate = 2'b00;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		// write address
		unique case(waddrstate)
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
		ledwe <= 1'b0;
	end else begin
		// write data
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		ledwe <= 1'b0;
		unique case(writestate)
			2'b00: begin
				if (s_axi.wvalid) begin
					// Only the lower byte contains valid data
					ledwe <= 1'b1;
					ledstate <= s_axi.wdata[4:0];
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

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.rlast <= 1'b1;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
		raddrstate <= 2'b00;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		unique case(raddrstate)
			2'b00: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
			default/*2'b01*/: begin
				// Only the lower byte contains valid data
				if (s_axi.rready) begin
					s_axi.rdata[31:0] <= {30'd0, ledstate};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
