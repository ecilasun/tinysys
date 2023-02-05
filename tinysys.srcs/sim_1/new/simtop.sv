`timescale 1ns / 1ps

module simtop();

logic boardresetn;
logic boardclock;

initial begin
	boardresetn = 1'bz;
	boardclock = 1'bz;
	#80;
	boardclock = 1'b0;
	boardresetn = 1'b1;
end

wire [1:0] ledout;
wire uart_rxd_out;
wire uart_txd_in = 1'b1;

tophat main(
    .sys_clk(boardclock),
    .sys_rst_n(boardresetn),
    .leds(ledout),
    .uart_rxd_out(uart_rxd_out),
    .uart_txd_in(uart_txd_in) );

always begin
	#10
	boardclock = ~boardclock;
end

endmodule
