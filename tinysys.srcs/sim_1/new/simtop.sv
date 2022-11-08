`timescale 1ns / 1ps

module simtop();

logic boardresetn;
logic boardclock;

initial begin
	boardresetn = 1'bz;
	boardclock = 1'bz;
	#20;
	boardclock = 1'b0;
	boardresetn = 1'b1;
end

wire [1:0] ledout;
tophat main(
    .sys_clk(boardclock),
    .sys_rst_n(boardresetn),
    .leds(ledout) );

always begin
	#10
	boardclock = ~boardclock;
end

endmodule
