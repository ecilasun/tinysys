`timescale 1ns / 1ps

module axi4CSRFile(
	input wire aclk,
	input wire aresetn,
	input wire [63:0] cpuclocktime,
	input wire [63:0] wallclocktime,
	input wire [63:0] retired,
	axi4if.slave s_axi );

// --------------------------------------------------
// CSR RAM
// --------------------------------------------------

logic csrwe = 1'b0;
logic csrre = 1'b0;
logic [11:0] cswraddr = 12'h000;
logic [11:0] csrraddr = 12'h000;
logic [31:0] csrdin = 32'd0;
logic [31:0] csrdout = 32'd0;

logic [31:0] csrmemory[0:4095];

initial begin
	int ri;
	for (ri=0; ri<4096; ri=ri+1) begin
		csrmemory[ri]  = 32'h00000000;
	end
end

always @(posedge aclk) begin
	if (csrre) begin
		unique case (csrraddr)
			`CSR_MHARTID:	csrdout <= 0;//HARTID; // Immutable
			`CSR_RETIHI:	csrdout <= retired[63:32];
			`CSR_TIMEHI:	csrdout <= wallclocktime[63:32];
			`CSR_CYCLEHI:	csrdout <= cpuclocktime[63:32];
			`CSR_RETILO:	csrdout <= retired[31:0];
			`CSR_TIMELO:	csrdout <= wallclocktime[31:0];
			`CSR_CYCLELO:	csrdout <= cpuclocktime[31:0];
			`CSR_MISA:		csrdout <= 32'h00001100; // rv32i(bit8), Zmmul(bit12), machine level
			default:		csrdout <= csrmemory[csrraddr];
		endcase
	end

	if (csrwe)
		csrmemory[cswraddr] <= csrdin;
end

// --------------------------------------------------
// AXI4 interface
// --------------------------------------------------

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
	end else begin
		// write data
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		csrwe <= 1'b0;
		unique case(writestate)
			2'b00: begin
				if (s_axi.wvalid) begin
					csrdin <= s_axi.wdata[31:0];
					cswraddr <= s_axi.awaddr[11:0];
					csrwe <= 1'b1;
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
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		csrre <= 1'b0;
		unique case(raddrstate)
			2'b00: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					csrraddr <= s_axi.araddr[11:0];
					csrre <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
			default/*2'b01*/: begin
				if (s_axi.rready) begin
					s_axi.rdata[31:0] <= csrdout;
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
