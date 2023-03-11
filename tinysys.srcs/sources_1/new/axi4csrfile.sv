`timescale 1ns / 1ps

module axi4CSRFile(
	input wire aclk,
	input wire aresetn,
	input wire [63:0] cpuclocktime,
	input wire [63:0] wallclocktime,
	input wire [63:0] retired,
	// IRQ lines to fetch unit
	input wire irqHold,
	output wire [1:0] irqReq,
	// Incoming hardware interrupt requests
	input wire uartrcvempty,
	// Expose certain registers to fetch unit
	output wire [31:0] mepc,
	output wire [31:0] mtvec,
	output wire sie,
	// Bus
	axi4if.slave s_axi );

// --------------------------------------------------
// CSR RAM
// --------------------------------------------------

// Shadows
logic [63:0] timecmpshadow = 64'hFFFFFFFFFFFFFFFF;
logic [31:0] mepcshadow = 32'd0;
logic [2:0] mieshadow = 3'b000; // Machine interrupt enable states
logic mstatusIEshadow = 1'b0; // Global interrupt enable
logic [31:0] mtvecshadow = 32'd0;
logic dummyshadow = 1'b0;

logic csrwe = 1'b0;
logic [11:0] cswraddr; // NOTE: Vivado seems to have trouble synthesizing BRAM when this is initialized
logic [11:0] csrraddr;
logic [31:0] csrdin = 32'd0;
logic [31:0] csrdout = 32'd0;

logic [31:0] csrmemory[0:4095];

initial begin
	int ri;
	for (ri=0; ri<4096; ri=ri+1) begin
		csrmemory[ri] = 32'h00000000;
	end
	// Start same as timecmpshadow
	csrmemory[`CSR_TIMECMPLO] = 32'hFFFFFFFF;
	csrmemory[`CSR_TIMECMPHI] = 32'hFFFFFFFF;
end

// Write
always @(posedge aclk) begin
	csrdout <= csrmemory[csrraddr];
	if (csrwe)
		csrmemory[cswraddr] <= csrdin;
end

assign mepc = mepcshadow;
assign mtvec = mtvecshadow;
assign sie = mieshadow[0] && mstatusIEshadow && (~irqHold);

// --------------------------------------------------
// Interrupt generator
// --------------------------------------------------

logic timerInterrupt = 1'b0;
logic uartInterrupt = 1'b0;

always @(posedge aclk) begin
	if (~aresetn) begin
		timerInterrupt <= 1'b0;
		uartInterrupt <= 1'b0;
	end else begin
		// Common condition to fire an IRQ: Fetch isn't holding an IRQ, specific interrups are enabled, and global machine interrupts are enabled
		// Timer interrupt
		timerInterrupt <= mieshadow[1] && mstatusIEshadow && (~irqHold) && (wallclocktime >= timecmpshadow);
		// Machine external interrupt (UART)
		uartInterrupt <= mieshadow[2] && mstatusIEshadow && (~irqHold) && (~uartrcvempty);
	end
end

assign irqReq = {uartInterrupt, timerInterrupt};

// --------------------------------------------------
// AXI4 interface
// --------------------------------------------------

logic waddrstate = 1'b0;
logic raddrstate = 1'b0;
logic [1:0] writestate = 2'b00;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		unique case(waddrstate)
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
		timecmpshadow <= 64'hFFFFFFFFFFFFFFFF;
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
			2'b01: begin
				if (s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b10;
				end
			end
			2'b10: begin // Generate a shadow copy of some registers
				unique case (cswraddr)
					`CSR_TIMECMPLO:	timecmpshadow[31:0] <= csrdin;
					`CSR_TIMECMPHI:	timecmpshadow[63:32] <= csrdin;
					`CSR_MEPC:		mepcshadow <= csrdin;
					`CSR_MIE:		mieshadow <= {csrdin[11], csrdin[7], csrdin[3]};	// Only store MEIE, MTIE and MSIE bits
					`CSR_MSTATUS:	mstatusIEshadow <= csrdin[3];			// Global MIE bit
					`CSR_MTVEC:		mtvecshadow <= csrdin;
					default:		dummyshadow <= csrdin[0];				// Unused
				endcase
				writestate <= 2'b00;
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
		unique case(raddrstate)
			1'b0: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					csrraddr <= s_axi.araddr[11:0];
					raddrstate <= 1'b1;
				end
			end
			1'b1: begin
				if (s_axi.rready) begin
					unique case (csrraddr)
						`CSR_MHARTID:	s_axi.rdata[31:0] <= 0;//HARTID; // Immutable
						`CSR_RETIHI:	s_axi.rdata[31:0] <= retired[63:32];
						`CSR_TIMEHI:	s_axi.rdata[31:0] <= wallclocktime[63:32];
						`CSR_CYCLEHI:	s_axi.rdata[31:0] <= cpuclocktime[63:32];
						`CSR_RETILO:	s_axi.rdata[31:0] <= retired[31:0];
						`CSR_TIMELO:	s_axi.rdata[31:0] <= wallclocktime[31:0];
						`CSR_CYCLELO:	s_axi.rdata[31:0] <= cpuclocktime[31:0];
						`CSR_MISA:		s_axi.rdata[31:0] <= 32'h00001100; // rv32i(bit8), Zmmul(bit12), machine level
						default:		s_axi.rdata[31:0] <= csrdout;
					endcase
					s_axi.rvalid <= 1'b1;
					raddrstate <= 1'b0;
				end
			end
		endcase
	end
end

endmodule
