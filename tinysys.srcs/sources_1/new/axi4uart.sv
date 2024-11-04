`timescale 1ns / 1ps

module axi4uart(
	input wire aclk,
	input wire uartbaseclock,
	input wire aresetn,
	input wire rst10n,
	input wire uartrx,
	output wire uarttx,
	output wire uartirq,
	axi4if.slave s_axi );

logic [1:0] writestate;
logic [1:0] raddrstate;
logic [4:0] controlregister;

// TX

wire [7:0] outfifoout;
wire uarttxbusy, outfifoempty, outfifovalid;
logic [7:0] datatotransmit;
logic transmitbyte;
logic outfifore;

async_transmitter UART_transmit(
	.clk(uartbaseclock),
	.TxD_start(transmitbyte),
	.TxD_data(datatotransmit),
	.TxD(uarttx),
	.TxD_busy(uarttxbusy) );

logic [7:0] outfifodin;
logic outfifowe;
wire outfifofull;
uartoutfifo UART_out_fifo(
    // In
    .full(outfifofull),
    .din(outfifodin),		// Data latched from bus
    .wr_en(outfifowe),		// Bus controls write, high for one clock
    // Out
    .empty(outfifoempty),	// Nothing to read
    .dout(outfifoout),		// To transmitter
    .rd_en(outfifore),		// Transmitter can send
    .wr_clk(aclk),			// Bus write clock
    .rd_clk(uartbaseclock),	// Transmitter clock runs much slower
    .valid(outfifovalid),	// Read result valid
    // Ctl
    .rst(~aresetn) );

// Fifo output serializer
always @(posedge uartbaseclock) begin
	if (~rst10n) begin
		outfifore <= 1'b0;
		transmitbyte <= 1'b0;
		datatotransmit <= 8'h00;
	end else begin
		outfifore <= 1'b0;
		transmitbyte <= 1'b0;
		if (~uarttxbusy && (transmitbyte == 1'b0) && (~outfifoempty) && outfifovalid) begin
			datatotransmit <= outfifoout;
			transmitbyte <= 1'b1;
			outfifore <= 1'b1; // advance fifo
		end
	end
end

// RX

wire [7:0] uartbytein;
wire infifofull, infifovalid, uartbyteavailable;
logic [7:0] inuartbyte;
logic infifowe;

async_receiver UART_receive(
	.clk(uartbaseclock),
	.RxD(uartrx),
	.RxD_data_ready(uartbyteavailable),
	.RxD_data(uartbytein),
	.RxD_idle(),
	.RxD_endofpacket() );

// Input FIFO
logic infifore;
wire uartfifoempty;
wire [7:0] infifodout;
uartinfifo UART_in_fifo(
    // In
    .full(infifofull),
    .din(inuartbyte),
    .wr_en(infifowe),
    // Out
    .empty(uartfifoempty),
    .dout(infifodout),
    .rd_en(infifore),
    .wr_clk(uartbaseclock),
    .rd_clk(aclk),
    .valid(infifovalid),
    // Ctl
    .rst(~aresetn) );

// Fifo input control
always @(posedge uartbaseclock) begin
	if (~rst10n) begin
		inuartbyte <= 8'd0;
		infifowe <= 1'b0;
	end else begin
		infifowe <= 1'b0;
		if (~infifofull && uartbyteavailable) begin
			infifowe <= 1'b1;
			inuartbyte <= uartbytein;
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
		outfifowe <= 1'b0;
		outfifodin <= 8'h00;
		writestate <= 2'b00;
		controlregister <= 5'b10000; // [... : intena : reserved1 : reserved0 : resetrxfifo : resettxfifo]
		s_axi.bresp = 2'b00;
	end else begin
		outfifowe <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
	
		unique case (writestate)
			2'b00: begin
				if (s_axi.wvalid) begin
					case (s_axi.awaddr[3:0])
						4'h4:		writestate <= 2'b01;	// Transmit
						4'hC:		writestate <= 2'b10;	// Control
						default:	writestate <= 2'b11;	// Ignore
					endcase
					s_axi.wready <= 1'b1;
				end
			end
			2'b01: begin
				if(s_axi.bready && (~outfifofull)) begin
					outfifodin <= s_axi.wdata[7:0];
					outfifowe <= 1'b1;
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b00;
				end
			end
			2'b10: begin
				if(s_axi.bready) begin
					controlregister <= s_axi.wdata[4:0];
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b00;
				end
			end
			2'b11: begin
				if(s_axi.bready) begin
					// NOOP
					s_axi.bvalid <= 1'b1;
					writestate <= 2'b00;
				end
			end
		endcase
		end
end

always @(posedge aclk) begin
	if (~aresetn) begin
		infifore <= 1'b0;
		raddrstate <= 2'b00;
	end else begin
		infifore <= 1'b0;
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
					case (s_axi.araddr[3:0])
						4'h0:		raddrstate <= 2'b10;	// Receive
						4'h8:		raddrstate <= 2'b11;	// Status
						default:	raddrstate <= 2'b01;	// Ignore
					endcase
					s_axi.arready <= 1'b1;
				end
			end
			2'b10: begin
				if (s_axi.rready && (~uartfifoempty) && infifovalid) begin
					s_axi.rdata <= {infifodout, infifodout, infifodout, infifodout};
					s_axi.rvalid <= 1'b1;
					// Advance FIFO
					infifore <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
			2'b11: begin
				if (s_axi.rready) begin
					s_axi.rdata <= {27'd0, controlregister[4], outfifofull, outfifoempty, infifofull, (~uartfifoempty) && infifovalid};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b01;
				end
			end
		endcase
		
	end
end

(* async_reg = "true" *) logic uartirqA;
(* async_reg = "true" *) logic uartirqB;

always_ff @(posedge aclk) begin
	if (~aresetn) begin
		uartirqA <= 1'b0;
		uartirqB <= 1'b0;
	end else begin
		uartirqA <= (~uartfifoempty) && infifovalid;
		uartirqB <= uartirqA;
	end
end

assign uartirq = uartirqB & controlregister[4];

endmodule
