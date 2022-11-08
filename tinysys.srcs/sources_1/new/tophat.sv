`timescale 1ns / 1ps

module tophat(
    input sys_clk,
    input sys_rst_n,
    output wire [1:0] leds);

parameter DLY_CNT = 32'd25000000;
parameter HALF_DLY_CNT = 32'd12500000;

logic ledreg = 1'b0;
logic [31:0]count = 32'd0;

wire clk_locked;
wire core_clk;
coreclockgen clkinst(
	.clk_in1(sys_clk),
	.core(core_clk),
	.locked(clk_locked),
	.reset(~sys_rst_n));

always@(posedge core_clk or negedge sys_rst_n)
begin
	if(!sys_rst_n) begin
		count <= 32'd0;
	end else if(count == DLY_CNT) begin
		count <= 32'd0;
	end else begin
		count <= count + 32'd1;
	end
end

//led output register control
always@(posedge core_clk or negedge sys_rst_n)
begin
	if(!sys_rst_n) begin
		ledreg <= 1'b0;
	end	else begin
		ledreg <= count < HALF_DLY_CNT ? 1'b1 : 1'b0;
	end
end

assign leds[1] = ledreg;
assign leds[0] = ~ledreg;

endmodule
