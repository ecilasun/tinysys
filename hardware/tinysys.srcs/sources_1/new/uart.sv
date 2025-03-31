`timescale 1ns / 1ps
`default_nettype none

////////////////////////////////////////////////////////
// RS-232 RX and TX module
// (c) fpga4fun.com & KNJN LLC - 2003 to 2016

// The RS-232 settings are fixed
// TX: 8-bit data, 2 stop, no-parity
// RX: 8-bit data, 1 stop, no-parity (the receiver can accept more stop bits of course)

////////////////////////////////////////////////////////
module async_transmitter(
	input wire clk,
	input wire resetn,
	input wire TxD_start,
	input wire [7:0] TxD_data,
	output wire TxD,
	output wire TxD_busy
);

// Assert TxD_start for (at least) one clock cycle to start transmission of TxD_data
// TxD_data is latched so that it doesn't have to stay valid while it is being sent

parameter ClkFrequency = 10000000;	// 10MHz
parameter Baud = 115200;

////////////////////////////////
wire BitTick;
BaudTickGen #(ClkFrequency, Baud) tickgen(.clk(clk), .resetn(resetn), .enable(TxD_busy), .tick(BitTick));

reg [3:0] TxD_state;
wire TxD_ready = (TxD_state==0);
assign TxD_busy = ~TxD_ready;

reg [7:0] TxD_shift;
always @(posedge clk) begin
	if(~resetn) begin
		TxD_shift <= 8'h00;
		TxD_state <= 4'b0000;
	end else begin
		if(TxD_ready & TxD_start)
			TxD_shift <= TxD_data;
		else
		if(TxD_state[3] & BitTick)
			TxD_shift <= (TxD_shift >> 1);
		case(TxD_state)
			4'b0000: if(TxD_start) TxD_state <= 4'b0100;
			4'b0100: if(BitTick) TxD_state <= 4'b1000;  // start bit
			4'b1000: if(BitTick) TxD_state <= 4'b1001;  // bit 0
			4'b1001: if(BitTick) TxD_state <= 4'b1010;  // bit 1
			4'b1010: if(BitTick) TxD_state <= 4'b1011;  // bit 2
			4'b1011: if(BitTick) TxD_state <= 4'b1100;  // bit 3
			4'b1100: if(BitTick) TxD_state <= 4'b1101;  // bit 4
			4'b1101: if(BitTick) TxD_state <= 4'b1110;  // bit 5
			4'b1110: if(BitTick) TxD_state <= 4'b1111;  // bit 6
			4'b1111: if(BitTick) TxD_state <= 4'b0010;  // bit 7
			4'b0010: if(BitTick) TxD_state <= 4'b0011;  // stop1
			4'b0011: if(BitTick) TxD_state <= 4'b0000;  // stop2
			default: if(BitTick) TxD_state <= 4'b0000;
		endcase
	end
end

assign TxD = (TxD_state<4) | (TxD_state[3] & TxD_shift[0]);  // put together the start, data and stop bits
endmodule


////////////////////////////////////////////////////////
module async_receiver(
	input wire clk,
	input wire resetn,
	input wire RxD,
	output wire RxD_data_ready,
	output wire [7:0] RxD_data,  // data received, valid only (for one clock cycle) when RxD_data_ready is asserted

	// We also detect if a gap occurs in the received stream of characters
	// That can be useful if multiple characters are sent in burst
	//  so that multiple characters can be treated as a "packet"
	output wire RxD_idle,  // asserted when no data has been received for a while
	output wire RxD_endofpacket  // asserted for one clock cycle when a packet has been detected (i.e. RxD_idle is going high)
);

parameter ClkFrequency = 10000000;	// 10MHz
parameter Baud = 115200;

parameter Oversampling = 8;  // needs to be a power of 2
// we oversample the RxD line at a fixed rate to capture each RxD data bit at the "right" time
// 8 times oversampling by default, use 16 for higher quality reception

////////////////////////////////
reg [3:0] RxD_state;
reg RxD_data_ready_reg;
reg [7:0] RxD_data_reg;
reg RxD_endofpacket_reg;
assign RxD_data_ready = RxD_data_ready_reg;
assign RxD_data = RxD_data_reg;
assign RxD_endofpacket = RxD_endofpacket_reg;

wire OversamplingTick;
BaudTickGen #(ClkFrequency, Baud, Oversampling) tickgen(.clk(clk), .resetn(resetn), .enable(1'b1), .tick(OversamplingTick));

// synchronize RxD to our clk domain
reg [1:0] RxD_sync;
always @(posedge clk) begin
	if(~resetn) begin
		RxD_sync <= 2'b11;
	end else begin
		if(OversamplingTick)
			RxD_sync <= {RxD_sync[0], RxD};
	end
end

// and filter it
reg [1:0] Filter_cnt;
reg RxD_bit;

always @(posedge clk) begin
	if (~resetn) begin
		Filter_cnt <= 2'b11;
		RxD_bit <= 1'b1;
	end else begin
		if(OversamplingTick) begin
			if(RxD_sync[1]==1'b1 && Filter_cnt!=2'b11)
				Filter_cnt <= Filter_cnt + 1'd1;
			else if(RxD_sync[1]==1'b0 && Filter_cnt!=2'b00)
				Filter_cnt <= Filter_cnt - 1'd1;
			if(Filter_cnt==2'b11)
				RxD_bit <= 1'b1;
			else if(Filter_cnt==2'b00)
				RxD_bit <= 1'b0;
		end
	end
end

// and decide when is the good time to sample the RxD line
function integer log2(input integer v); begin log2=0; while(v>>log2) log2=log2+1; end endfunction
localparam l2o = log2(Oversampling);
reg [l2o-2:0] OversamplingCnt;

always @(posedge clk) begin
	if (~resetn) begin
		OversamplingCnt <= 0;
	end else begin
		if(OversamplingTick)
			OversamplingCnt <= (RxD_state==0) ? 1'd0 : OversamplingCnt + 1'd1;
	end
end
wire sampleNow = OversamplingTick && (OversamplingCnt==Oversampling/2-1);

// now we can accumulate the RxD bits in a shift-register
always @(posedge clk) begin
	if (~resetn) begin
		RxD_state <= 4'b0000;
	end else begin
		case(RxD_state)
			4'b0000: if(~RxD_bit) RxD_state <= 4'b0001;  // start bit found?
			4'b0001: if(sampleNow) RxD_state <= 4'b1000;  // sync start bit to sampleNow
			4'b1000: if(sampleNow) RxD_state <= 4'b1001;  // bit 0
			4'b1001: if(sampleNow) RxD_state <= 4'b1010;  // bit 1
			4'b1010: if(sampleNow) RxD_state <= 4'b1011;  // bit 2
			4'b1011: if(sampleNow) RxD_state <= 4'b1100;  // bit 3
			4'b1100: if(sampleNow) RxD_state <= 4'b1101;  // bit 4
			4'b1101: if(sampleNow) RxD_state <= 4'b1110;  // bit 5
			4'b1110: if(sampleNow) RxD_state <= 4'b1111;  // bit 6
			4'b1111: if(sampleNow) RxD_state <= 4'b0010;  // bit 7
			4'b0010: if(sampleNow) RxD_state <= 4'b0000;  // stop bit
			default: RxD_state <= 4'b0000;
		endcase
	end
end

always @(posedge clk) begin
	if (~resetn) begin
		RxD_data_reg <= 8'h00;
	end else begin
		if(sampleNow && RxD_state[3])
			RxD_data_reg <= {RxD_bit, RxD_data_reg[7:1]};
	end
end

//reg RxD_data_error = 0;
always @(posedge clk) begin
	if (~resetn) begin
		RxD_data_ready_reg <= 1'b0;
		//RxD_data_error <= 1'b0;
	end else begin
		RxD_data_ready_reg <= (sampleNow && RxD_state==4'b0010 && RxD_bit);  // make sure a stop bit is received
		//RxD_data_error <= (sampleNow && RxD_state==4'b0010 && ~RxD_bit);  // error if a stop bit is not received
	end
end

reg [l2o+1:0] GapCnt;
always @(posedge clk) begin
	if (~resetn) begin
		GapCnt <= 0;
	end else begin
		if (RxD_state!=0)
			GapCnt<=0;
		else if(OversamplingTick & ~GapCnt[log2(Oversampling)+1])
			GapCnt <= GapCnt + 1'h1;
	end
end

assign RxD_idle = GapCnt[l2o+1];
always @(posedge clk) begin
	if(~resetn) begin
		RxD_endofpacket_reg <= 1'b0;
	end else begin
		RxD_endofpacket_reg <= OversamplingTick & ~GapCnt[l2o+1] & &GapCnt[l2o:0];
	end
end

endmodule


////////////////////////////////////////////////////////
module BaudTickGen(
	input wire clk,
	input wire resetn,
	input wire enable,
	output wire tick  // generate a tick at the specified baud rate * oversampling
);

parameter ClkFrequency = 10000000;	// 10MHz
parameter Baud = 115200;
parameter Oversampling = 1;

function integer log2(input integer v); begin log2=0; while(v>>log2) log2=log2+1; end endfunction
localparam AccWidth = log2(ClkFrequency/Baud)+8;  // +/- 2% max timing error over a byte
reg [AccWidth:0] Acc = 0;
localparam ShiftLimiter = log2(Baud*Oversampling >> (31-AccWidth));  // this makes sure Inc calculation doesn't overflow
localparam Inc = ((Baud*Oversampling << (AccWidth-ShiftLimiter))+(ClkFrequency>>(ShiftLimiter+1)))/(ClkFrequency>>ShiftLimiter);

always @(posedge clk) begin
	if (~resetn) begin
		Acc <= 0;
	end else begin
		if(enable)
			Acc <= Acc[AccWidth-1:0] + Inc[AccWidth:0];
		else
			Acc <= Inc[AccWidth:0];
	end
end

assign tick = Acc[AccWidth];
endmodule


////////////////////////////////////////////////////////
