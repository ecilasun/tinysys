`timescale 1ns / 1ps

module axi4uart (
	input wire aclk,
	input wire uartclk,
	input wire aresetn,
	axi4if.slave s_axi,
	output wire uart_rxd_out,
	input wire uart_txd_in,
	output wire uartrcvempty );

// 115200 / 10MHz for serial terminal
// 400000 / 25MHz for USB serial interface
parameter BAUDRATE = 115200;
parameter FREQ = 10000000;

logic [1:0] waddrstate = 2'b00;
logic [1:0] writestate = 2'b00;
logic [2:0] raddrstate = 3'b000;

//logic [31:0] writeaddress = 32'd0;
logic [7:0] din = 8'h00;
logic [3:0] we = 4'h0;

// ----------------------------------------------------------------------------
// uart transmitter
// ----------------------------------------------------------------------------

bit transmitbyte = 1'b0;
bit [7:0] datatotransmit = 8'h00;
bit uartwritemode = 1'b0;
wire uarttxbusy;
wire [7:0] uartsenddout;
wire uartsendfull, uartsendempty, uartsendvalid;

always @(posedge uartclk) begin
	transmitbyte <= 1'b0;
	unique case(uartwritemode)
		1'b0: begin // idle
			if (~uartsendempty && uartsendvalid && (~uarttxbusy)) begin
				datatotransmit <= uartsenddout;
				uartwritemode <= 1'b1; // finalize
				// Advance fifo / send
				transmitbyte <= 1'b1;
			end
		end
		default/*1'b1*/: begin // finalize
			// need to give uarttx one clock to
			// kick 'busy' for any adjacent
			// requests which didn't set busy yet
			uartwritemode <= 1'b0; // idle
		end
	endcase
end

async_transmitter #(.clkfrequency(FREQ), .baud(BAUDRATE)) uart_transmit(
	.clk(uartclk),
	.txd_start(transmitbyte),
	.txd_data(datatotransmit),
	.txd(uart_rxd_out),
	.txd_busy(uarttxbusy) );

uartoutfifo UARTOut(
	.full(uartsendfull),
	.din(din),
	.wr_en( (|we) ),
	.empty(uartsendempty),
	.dout(uartsenddout),
	.rd_en(transmitbyte),
	.valid(uartsendvalid),
	.wr_clk(aclk),
	.rd_clk(uartclk),
	.rst(~aresetn) );

// ----------------------------------------------------------------------------
// uart receiver
// ----------------------------------------------------------------------------

wire uartbyteavailable;
wire [7:0] uartbytein;

async_receiver #(.clkfrequency(FREQ), .baud(BAUDRATE)) uart_receive(
	.clk(uartclk),
	.rxd(uart_txd_in),
	.rxd_data_ready(uartbyteavailable),
	.rxd_data(uartbytein),
	.rxd_idle(),
	.rxd_endofpacket() );

wire uartrcvfull, uartrcvvalid;
bit [7:0] uartrcvdin = 8'h00;
wire [7:0] uartrcvdout;
bit uartrcvre = 1'b0, uartrcvwe = 1'b0;

uartinfifo UARTIn(
	.full(uartrcvfull),
	.din(uartrcvdin),
	.wr_en(uartrcvwe),
	.wr_clk(uartclk),
	.rd_clk(aclk),
	.empty(uartrcvempty),
	.dout(uartrcvdout),
	.rd_en(uartrcvre),
	.valid(uartrcvvalid),
	.rst(~aresetn) );

always @(posedge uartclk) begin
	uartrcvwe <= 1'b0;
	// NOTE: Any byte that won't fit into the fifo will be dropped
	// make sure to consume them quickly on arrival!
	if (uartbyteavailable & (~uartrcvfull)) begin
		uartrcvwe <= 1'b1;
		uartrcvdin <= uartbytein;
	end
end

// IO_UARTRX     0x80000000
// IO_UARTTX     0x80000004
// IO_UARTStatus 0x80000008
// IO_UARTCtl    0x8000000C

// main state machine
always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.awready <= 1'b0;
	end else begin
		// write address
		unique case(waddrstate)
			2'b00: begin
				if (s_axi.awvalid) begin
					s_axi.awready <= 1'b1;
					//writeaddress <= s_axi.awaddr; // todo: select subdevice using some bits of address
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
		writestate <= 2'b00;
		din <= 8'd0;
	end else begin
		// write data
		we <= 4'h0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		unique case(writestate)
			2'b00: begin
				if (s_axi.wvalid) begin
					unique case(s_axi.awaddr[3:0])
						4'h0: begin // rx data
							// Cannot write here, skip
							writestate <= 2'b01;
							s_axi.wready <= 1'b1;
						end
						4'h4: begin // tx data
							if (~uartsendfull) begin
								din <= s_axi.wdata[7:0];
								we <= s_axi.wstrb[3:0];
								writestate <= 2'b01;
								s_axi.wready <= 1'b1;
							end
						end
						4'h8: begin // status register
							// Cannot write here, skip
							writestate <= 2'b01;
							s_axi.wready <= 1'b1;
						end
						default/*2'hC*/: begin // control register
							// Cannot write here (yet), skip
							writestate <= 2'b01;
							s_axi.wready <= 1'b1;
						end
					endcase
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
		s_axi.rdata <= 'd0;
	end else begin
		uartrcvre <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		// read address
		unique case(raddrstate)
			3'b000: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					unique case(s_axi.araddr[3:0])
						4'h0: raddrstate <= 3'b001; // RX
						4'h4: raddrstate <= 3'b010; // TX
						4'h8: raddrstate <= 3'b011; // Status
						4'hC: raddrstate <= 3'b100; // Control
						//default: raddrstate <= 3'b101; // Misc
					endcase
				end
			end
			
			3'b001: begin
				// RX
				if (s_axi.rready && uartrcvvalid /*&& ~uartrcvempty*/) begin
					s_axi.rdata[31:0] <= {uartrcvdout, uartrcvdout, uartrcvdout, uartrcvdout};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 3'b000;
					// Advance fifo
					uartrcvre <= 1'b1;
				end
			end

			3'b010: begin
				// TX
				// cannot read this port (output), skip
				s_axi.rdata[31:0] <= 32'd0;
				s_axi.rvalid <= 1'b1;
				raddrstate <= 3'b000;
			end

			3'b011: begin
				// Rx status register
				s_axi.rdata[31:0] <= {30'd0, uartrcvfull, ~uartrcvempty};
				s_axi.rvalid <= 1'b1;
				raddrstate <= 3'b000;
			end

			3'b100: begin
				// Control register
				// cannot read this (yet), skip
				s_axi.rdata[31:0] <= 32'd0;
				s_axi.rvalid <= 1'b1;
				raddrstate <= 3'b000;
			end

			/*3'b101: begin
				// All others
				s_axi.rdata[31:0] <= 32'd0;
				s_axi.rvalid <= 1'b1;
				raddrstate <= 3'b000;
			end*/
		endcase
	end
end

endmodule