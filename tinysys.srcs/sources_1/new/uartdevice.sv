`timescale 1ns / 1ps

////////////////////////////////////////////////////////
// rs-232 rx and tx module
// (c) fpga4fun.com & knjn llc - 2003 to 2016

// the rs-232 settings are fixed
// tx: 8-bit data, 2 stop, no-parity
// rx: 8-bit data, 1 stop, no-parity (the receiver can accept more stop bits of course)

////////////////////////////////////////////////////////
module async_transmitter(
	input wire clk,
	input wire txd_start,
	input wire [7:0] txd_data,
	output wire txd,
	output wire txd_busy
);

// assert txd_start for (at least) one clock cycle to start transmission of txd_data
// txd_data is latched so that it doesn't have to stay valid while it is being sent

parameter clkfrequency = 10000000;	// 10mhz
parameter baud = 115200;

////////////////////////////////
wire bittick;
baudtickgen #(clkfrequency, baud) tickgen(.clk(clk), .enable(txd_busy), .tick(bittick));

reg [3:0] txd_state = 0;
wire txd_ready = (txd_state==0);
assign txd_busy = ~txd_ready;

reg [7:0] txd_shift = 0;
always @(posedge clk)
begin
	if(txd_ready & txd_start)
		txd_shift <= txd_data;
	else
	if(txd_state[3] & bittick)
		txd_shift <= (txd_shift >> 1);

	case(txd_state)
		4'b0000: if(txd_start) txd_state <= 4'b0100;
		4'b0100: if(bittick) txd_state <= 4'b1000;  // start bit
		4'b1000: if(bittick) txd_state <= 4'b1001;  // bit 0
		4'b1001: if(bittick) txd_state <= 4'b1010;  // bit 1
		4'b1010: if(bittick) txd_state <= 4'b1011;  // bit 2
		4'b1011: if(bittick) txd_state <= 4'b1100;  // bit 3
		4'b1100: if(bittick) txd_state <= 4'b1101;  // bit 4
		4'b1101: if(bittick) txd_state <= 4'b1110;  // bit 5
		4'b1110: if(bittick) txd_state <= 4'b1111;  // bit 6
		4'b1111: if(bittick) txd_state <= 4'b0010;  // bit 7
		4'b0010: if(bittick) txd_state <= 4'b0011;  // stop1
		4'b0011: if(bittick) txd_state <= 4'b0000;  // stop2
		default: if(bittick) txd_state <= 4'b0000;
	endcase
end

assign txd = (txd_state<4) | (txd_state[3] & txd_shift[0]);  // put together the start, data and stop bits
endmodule


////////////////////////////////////////////////////////
module async_receiver(
	input wire clk,
	input wire rxd,
	output reg rxd_data_ready = 0,
	output reg [7:0] rxd_data = 0,  // data received, valid only (for one clock cycle) when rxd_data_ready is asserted

	// we also detect if a gap occurs in the received stream of characters
	// that can be useful if multiple characters are sent in burst
	//  so that multiple characters can be treated as a "packet"
	output wire rxd_idle,  // asserted when no data has been received for a while
	output reg rxd_endofpacket = 0  // asserted for one clock cycle when a packet has been detected (i.e. rxd_idle is going high)
);

parameter clkfrequency = 10000000;	// 10mhz
parameter baud = 115200;

parameter oversampling = 8;  // needs to be a power of 2
// we oversample the rxd line at a fixed rate to capture each rxd data bit at the "right" time
// 8 times oversampling by default, use 16 for higher quality reception

////////////////////////////////
reg [3:0] rxd_state = 0;

wire oversamplingtick;
baudtickgen #(clkfrequency, baud, oversampling) tickgen(.clk(clk), .enable(1'b1), .tick(oversamplingtick));

// synchronize rxd to our clk domain
reg [1:0] rxd_sync = 2'b11;
always @(posedge clk) if(oversamplingtick) rxd_sync <= {rxd_sync[0], rxd};

// and filter it
reg [1:0] filter_cnt = 2'b11;
reg rxd_bit = 1'b1;

always @(posedge clk)
if(oversamplingtick)
begin
	if(rxd_sync[1]==1'b1 && filter_cnt!=2'b11) filter_cnt <= filter_cnt + 1'd1;
	else 
	if(rxd_sync[1]==1'b0 && filter_cnt!=2'b00) filter_cnt <= filter_cnt - 1'd1;

	if(filter_cnt==2'b11) rxd_bit <= 1'b1;
	else
	if(filter_cnt==2'b00) rxd_bit <= 1'b0;
end

// and decide when is the good time to sample the rxd line
function integer log2(input integer v); begin log2=0; while(v>>log2) log2=log2+1; end endfunction
localparam l2o = log2(oversampling);
reg [l2o-2:0] oversamplingcnt = 0;
always @(posedge clk) if(oversamplingtick) oversamplingcnt <= (rxd_state==0) ? 1'd0 : oversamplingcnt + 1'd1;
wire samplenow = oversamplingtick && (oversamplingcnt==oversampling/2-1);

// now we can accumulate the rxd bits in a shift-register
always @(posedge clk)
case(rxd_state)
	4'b0000: if(~rxd_bit) rxd_state <= 4'b0001;  // start bit found?
	4'b0001: if(samplenow) rxd_state <= 4'b1000;  // sync start bit to samplenow
	4'b1000: if(samplenow) rxd_state <= 4'b1001;  // bit 0
	4'b1001: if(samplenow) rxd_state <= 4'b1010;  // bit 1
	4'b1010: if(samplenow) rxd_state <= 4'b1011;  // bit 2
	4'b1011: if(samplenow) rxd_state <= 4'b1100;  // bit 3
	4'b1100: if(samplenow) rxd_state <= 4'b1101;  // bit 4
	4'b1101: if(samplenow) rxd_state <= 4'b1110;  // bit 5
	4'b1110: if(samplenow) rxd_state <= 4'b1111;  // bit 6
	4'b1111: if(samplenow) rxd_state <= 4'b0010;  // bit 7
	4'b0010: if(samplenow) rxd_state <= 4'b0000;  // stop bit
	default: rxd_state <= 4'b0000;
endcase

always @(posedge clk)
if(samplenow && rxd_state[3]) rxd_data <= {rxd_bit, rxd_data[7:1]};

//reg rxd_data_error = 0;
always @(posedge clk)
begin
	rxd_data_ready <= (samplenow && rxd_state==4'b0010 && rxd_bit);  // make sure a stop bit is received
	//rxd_data_error <= (samplenow && rxd_state==4'b0010 && ~rxd_bit);  // error if a stop bit is not received
end

reg [l2o+1:0] gapcnt = 0;
always @(posedge clk) if (rxd_state!=0) gapcnt<=0; else if(oversamplingtick & ~gapcnt[log2(oversampling)+1]) gapcnt <= gapcnt + 1'h1;
assign rxd_idle = gapcnt[l2o+1];
always @(posedge clk) rxd_endofpacket <= oversamplingtick & ~gapcnt[l2o+1] & &gapcnt[l2o:0];

endmodule


////////////////////////////////////////////////////////
module baudtickgen(
	input wire clk,
	input wire enable,
	output wire tick  // generate a tick at the specified baud rate * oversampling
);

parameter clkfrequency = 10000000;	// 10mhz
parameter baud = 115200;
parameter oversampling = 1;

function integer log2(input integer v); begin log2=0; while(v>>log2) log2=log2+1; end endfunction
localparam accwidth = log2(clkfrequency/baud)+8;  // +/- 2% max timing error over a byte
reg [accwidth:0] acc = 0;
localparam shiftlimiter = log2(baud*oversampling >> (31-accwidth));  // this makes sure inc calculation doesn't overflow
localparam inc = ((baud*oversampling << (accwidth-shiftlimiter))+(clkfrequency>>(shiftlimiter+1)))/(clkfrequency>>shiftlimiter);
always @(posedge clk) if(enable) acc <= acc[accwidth-1:0] + inc[accwidth:0]; else acc <= inc[accwidth:0];
assign tick = acc[accwidth];
endmodule


////////////////////////////////////////////////////////