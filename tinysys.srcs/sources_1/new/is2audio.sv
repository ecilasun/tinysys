`timescale 1ns / 1ps

module i2saudio(
    input wire audioclock,	// 22.591MHz master clock

	input wire abempty,
	input wire abvalid,
	output wire audiore,
    input wire [31:0] leftrightchannels,	// Audio input is 16bits in our device

    output wire tx_mclk,		// Audio bus output
    output wire tx_lrck,		// L/R select
    output wire tx_sclk,		// Stream clock
    output logic tx_sdout );	// Stream out
    
// Counter for generating other divided clocks
logic [8:0] count = 9'd0;

always @(posedge audioclock)
	count <= count + 1;

wire lrck = count[8]; // Channel select
wire sclk = count[2]; // Sample clock
assign tx_lrck = lrck;
assign tx_sclk = sclk;
assign tx_mclk = audioclock;

// Inernal L/R copies to stream out
logic [31:0] tx_data_l = 0;
logic [31:0] tx_data_r = 0;
logic re = 1'b0;
assign audiore = re;

always@(posedge audioclock) begin
	re <= 1'b0;
	// Trigger new sample copy just before we select L channel again out of the LR pair
	if ((~abempty) && abvalid && (count==9'h0ff)) begin
		tx_data_r <= {8'd0, leftrightchannels[31:16], 8'd0};
		tx_data_l <= {8'd0, leftrightchannels[15:0], 8'd0};
		// Advance FIFO 
		re <= 1'b1;
	end
end

logic [23:0] tx_data_l_shift = 24'b0;
logic [23:0] tx_data_r_shift = 24'b0;

always@(posedge audioclock)
	if (count == 3'b000000111) begin
		tx_data_l_shift <= tx_data_l[23:0];
		tx_data_r_shift <= tx_data_r[23:0];
	end else if (count[2:0] == 3'b111 && count[7:3] >= 5'd1 && count[7:3] <= 5'd24) begin
		if (count[8] == 1'b1)
			tx_data_r_shift <= {tx_data_r_shift[22:0], 1'b0};
		else
			tx_data_l_shift <= {tx_data_l_shift[22:0], 1'b0};
	end

always@(count, tx_data_l_shift, tx_data_r_shift) begin
	if (count[7:3] <= 5'd24 && count[7:3] >= 4'd1)
		if (count[8] == 1'b1)
			tx_sdout = tx_data_r_shift[23];
		else
			tx_sdout = tx_data_l_shift[23];
	else
		tx_sdout = 1'b0;
end

endmodule