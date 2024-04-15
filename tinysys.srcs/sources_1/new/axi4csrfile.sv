`timescale 1ns / 1ps

// NOTE: Each HART has a different base address for their CSR
// This currently houses 1x4K CSR unit for one HART

module axi4CSRFile #(
	parameter int HARTID = 4'h0
) (
	input wire aclk,
	input wire aresetn,
	input wire [63:0] cpuclocktime,
	input wire [63:0] wallclocktime,
	input wire [63:0] retired,
	// IRQ lines to fetch unit
	output wire [1:0] irqReq,
	// Incoming hardware interrupt requests
	input wire keyirq,
	input wire usbirq,
	input wire gpioirq,
	input wire uartirq,
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
logic [63:0] timecmpshadow;
logic [31:0] mepcshadow;
logic [2:0] mieshadow; 			// Machine interrupt enable states
logic mstatusIEshadow;   		// Global interrupts disabled on by default
logic [31:0] mtvecshadow;
logic dummyshadow;

logic csrwe = 1'b0;
logic [11:0] cswraddr; // NOTE: Vivado seems to have trouble synthesizing BRAM when this is initialized
logic [11:0] csrraddr;
logic [31:0] csrdin;
logic [31:0] csrdout;

logic [31:0] csrmemory[0:4095];

initial begin
	int ri;
	for (ri=0; ri<4096; ri=ri+1) begin
		csrmemory[ri] = 32'h00000000;
	end
	// Start same as timecmpshadow
	csrmemory[`CSR_TIMECMPLO] = 32'hFFFFFFFF;
	csrmemory[`CSR_TIMECMPHI] = 32'hFFFFFFFF;
	// Machine ISA: rv32i/mul-div/nonstandarextensions
	csrmemory[`CSR_MISA] = 32'h00801100; // 0000 0000 1000 0000 0001 0001 0000 0000
	// Top bit must be set, can't contain zero (need a reserved ID from RISC-V international)
	csrmemory[`CSR_MARCHID] = 32'h80000000;
end

// Read/Write CSR
always @(posedge aclk) begin
	if (~aresetn) begin
		csrdout <= 32'd0;
	end else begin
		csrdout <= csrmemory[csrraddr];
		if (csrwe)
			csrmemory[cswraddr] <= csrdin;
	end
end

assign mepc = mepcshadow;
assign mtvec = mtvecshadow;

// --------------------------------------------------
// Interrupt generator
// --------------------------------------------------

logic timerInterrupt;
logic hwInterrupt;
logic softInterruptEna;

// Last bit of resetcpu CSR triggers a reset irq.
// Handler has to clear this bit to recover from reset, then branch to address in mscratch register. 
logic resetirq;

always @(posedge aclk) begin
	if (~aresetn) begin
		softInterruptEna <= 0;
		timerInterrupt <= 0;
		hwInterrupt <= 0;
	end else begin
		// Common condition to fire an IRQ:
		// Fetch isn't holding an IRQ, specific interrups are enabled, and global machine interrupts are enabled
		softInterruptEna <= mieshadow[0] && mstatusIEshadow; // Software interrupt (The rest of this condition is in fetch unit based on instruction)
		timerInterrupt <= mieshadow[1] && mstatusIEshadow && (wallclocktime >= timecmpshadow); // Timer interrupt
		hwInterrupt <= mieshadow[2] && mstatusIEshadow && (uartirq || gpioirq || keyirq || usbirq || resetirq); // Machine external interrupts
	end
end

assign sie = softInterruptEna;
assign irqReq = {hwInterrupt, timerInterrupt};

// --------------------------------------------------
// AXI4 interface
// --------------------------------------------------

logic [1:0] waddrstate;
logic [1:0] raddrstate;
logic [1:0] writestate;

always @(posedge aclk) begin
	if (~aresetn) begin
		waddrstate <= 2'b00;
	end else begin
		unique case(waddrstate)
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

always @(posedge aclk) begin

	if (~aresetn) begin
		csrdin <= 32'd0;
		writestate <= 2'b00;
		mtvecshadow <= 32'd0;
		mepcshadow <= 32'd0;
		dummyshadow <= 1'b0;
		mstatusIEshadow <= 1'b0;
		timecmpshadow <= 64'hFFFFFFFFFFFFFFFF;
		resetirq <= 1'b0;
		mieshadow <= 3'b000;
		csrwe <= 1'b0;
	end else begin
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		csrwe <= 1'b0;
	
		unique case(writestate)
			2'b00: begin
				s_axi.bresp <= 2'b00; // okay
				s_axi.bvalid <= 1'b0;
				s_axi.wready <= 1'b0;
				writestate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.wvalid) begin
					csrdin <= s_axi.wdata[31:0];
					cswraddr <= s_axi.awaddr[13:2]; // 4 byte aligned
					csrwe <= 1'b1;
					writestate <= 2'b10;
					s_axi.wready <= 1'b1;
				end
			end
			2'b10: begin
				if (s_axi.bready) begin
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b11;
				end
			end
			2'b11: begin // Generate a shadow copy of some registers as well as their true version in the CSR file
				unique case (cswraddr)
					`CSR_TIMECMPLO:	timecmpshadow[31:0] <= csrdin;
					`CSR_TIMECMPHI:	timecmpshadow[63:32] <= csrdin;
					`CSR_MEPC:		mepcshadow <= csrdin;
					`CSR_MIE:		mieshadow <= {csrdin[11], csrdin[7], csrdin[3]};	// Only store MEIE, MTIE and MSIE bits
					`CSR_MSTATUS:	mstatusIEshadow <= csrdin[3];						// Global interrupt enable (MIE) bit
					`CSR_MTVEC:		mtvecshadow <= csrdin;								// Interrupt vector
					`CSR_CPURESET:	resetirq <= csrdin[0];								// Set reset IRQ state
					default:		dummyshadow <= csrdin[0];							// Unused - sink
				endcase
				writestate <= 2'b01;
			end
		endcase
	
	end
end

always @(posedge aclk) begin
	if (~aresetn) begin
		raddrstate <= 2'b00;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		unique case(raddrstate)
			2'b00: begin
				s_axi.rlast <= 1'b1;
				s_axi.rresp <= 2'b00;
				raddrstate <= 2'b01;
			end
			2'b01: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					csrraddr <= s_axi.araddr[13:2]; // 4 byte aligned
					raddrstate <= 2'b10;
				end
			end
			2'b10: begin
				if (s_axi.rready) begin
					s_axi.rdata[127:32] <= 0;
					// Some values such as timers and h/w states are dynamic and never end up in the CSR file, so make up a dynamic version for those
					unique case (csrraddr)
						// Immutables
						`CSR_MHARTID:	s_axi.rdata[31:0] <= {28'd0, HARTID}; // 0..15
						`CSR_RETIHI:	s_axi.rdata[31:0] <= retired[63:32];
						`CSR_TIMEHI:	s_axi.rdata[31:0] <= wallclocktime[63:32];
						`CSR_CYCLEHI:	s_axi.rdata[31:0] <= cpuclocktime[63:32];
						`CSR_RETILO:	s_axi.rdata[31:0] <= retired[31:0];
						`CSR_TIMELO:	s_axi.rdata[31:0] <= wallclocktime[31:0];
						`CSR_CYCLELO:	s_axi.rdata[31:0] <= cpuclocktime[31:0];
						// Interrupt states of all hardware devices
						`CSR_HWSTATE:	s_axi.rdata[31:0] <= {27'd0, resetirq, uartirq, gpioirq, keyirq, usbirq};
						// Pass through actual data
						default:		s_axi.rdata[31:0] <= csrdout;
					endcase
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
		endcase
	end
end

endmodule
