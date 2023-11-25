`timescale 1ns / 1ps

module hwdebug(
	input wire aresetn,
	input wire aclk,
	input wire clk100,
	debugbusif.master m_dbg,
	input wire rx,
	output wire tx );

uart_if rxif();
uart_if txif();

assign rxif.sig = rx;
assign tx = txif.sig;

// Receive
/*wire rcvfull, rcvempty, rcvvalid;
wire [7:0] rcvdout;
logic rcvre = 1'b0;
uartfifo uartincoming (
  .rst(~aresetn),
  .wr_clk(clk100),
  .rd_clk(aclk),
  .din(rxif.data),		// UART controls writes, 1024 incoming bytes max
  .wr_en(rxif.valid),
  .rd_en(rcvre),
  .dout(rcvdout),
  .full(rcvfull),
  .empty(rcvempty),
  .valid(rcvvalid) );

// UART input to debug bus command
always @(posedge aclk) begin
	rcvre <= 1'b0;
	m_dbg.wen <= 1'b0;
	if (~rcvempty && rcvvalid) begin
		m_dbg.din <= rcvdout;
		m_dbg.wen <= 1'b1;
		rcvre <= 1'b1;
	end
end

// Send
wire sndfull, sndempty, sndvalid;
logic [7:0] snddin;
logic sndwe = 1'b0;
uartfifo uartoutgoing (
  .rst(~aresetn),
  .wr_clk(aclk),
  .rd_clk(clk100),
  .din(snddin),
  .wr_en(sndwe),
  .rd_en(txif.ready),
  .dout(txif.data),
  .full(sndfull),
  .empty(sndempty),
  .valid(sndvalid) );

// Debug bus response to UART output
always @(posedge aclk) begin
	sndwe <= 1'b0;
	if (~sndempty && sndvalid && m_dbg.drdy) begin
		snddin <= m_dbg.dout;
		sndwe <= 1'b1;
	end
end*/

uart uartinst(
	.rxif(rxif),
    .txif(txif),
    .clk(clk100),
    .rstn(aresetn) );

endmodule
