`timescale 1ns / 1ps

module axi4CSRFile(
	input wire aclk,
	input wire aresetn,
	input wire [63:0] cpuclocktime,
	input wire [63:0] wallclocktime,
	input wire [63:0] retired,
	axi4if.slave s_axi );

// CSR offsets (4K region between 80004000 - 80004FFF)
// 12'hF15: `CSR_UNUSED;	// mconfigptr, defaults to zero, no exception
// 12'h300: `CSR_MSTATUS;	// r/w
// 12'h304: `CSR_MIE;		// r/w
// 12'h305: `CSR_MTVEC;		// r/w [1:0]==2'b00->direct, ==2'b01->vectored
// 12'h341: `CSR_MEPC;		// r/w [1:0] always 2'b00 (or [0] always 1'b0)
// 12'h342: `CSR_MCAUSE;		// r/w
// 12'h343: `CSR_MTVAL;		// r/w excpt specific info such as faulty instruction
// 12'h344: `CSR_MIP;		// r/w
// 12'h340: 				// scratch register for machine trap mscratch
// 12'h301: 				// isa / extension type misa
// 12'hF14: `CSR_MHARTID;	// r
// 12'h800: `CSR_TIMECMPLO;	// r/w
// 12'h801: `CSR_TIMECMPHI;	// r/w
// 12'hC00,
// 12'hB00: `CSR_CYCLELO;	// r/w
// 12'hC80,
// 12'hB80: `CSR_CYCLEHI;	// r/w
// 12'hC02,
// 12'hB02: `CSR_RETILO;	// r
// 12'hC82,
// 12'hB82: `CSR_RETIHI;	// r
// 12'hC01: `CSR_TIMELO;	// r
// 12'hC81: `CSR_TIMEHI;	// r
// 12'h7B0: 				// debug control and status register dcsr
// 12'h7B1: 				// debug pc dpc
// 12'h7B2: 				// debug scratch register dscratch0
// 12'h7B3: 				// debug scratch register dscratch1

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
			`CSR_CYCLELO:	csrdout <= cpuclocktime[31:0];
			`CSR_CYCLEHI:	csrdout <= cpuclocktime[63:32];
			`CSR_TIMELO:	csrdout <= wallclocktime[31:0];
			`CSR_TIMEHI:	csrdout <= wallclocktime[63:32];
			`CSR_RETILO:	csrdout <= retired[31:0];
			`CSR_RETIHI:	csrdout <= retired[63:32];
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
					// Only the lower byte contains valid data
					csrdin <= s_axi.wdata[1:0];
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
				// Only the lower byte contains valid data
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
