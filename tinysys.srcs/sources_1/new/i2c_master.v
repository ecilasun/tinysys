

/*
 * An i2c master controller implementation. 7-bit address 8-bit data, r/w.
 *
 * Copyright (c) 2015 Joel Fernandes <joel@linuxinternals.org>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

`timescale 1ns / 1ps

module i2c_master(
	input [7:0] i_addr_data,		// Address and Data
	input i_cmd,					// Command (r/w)
	input i_strobe,					// Latch inputs
	input i_clk,
	inout io_sda,
	output io_scl,
	output reg [7:0] o_data,		// Output data on reads
	output wire [2:0] o_status );	// Request status

parameter ST_CMD_ADDR = 1,
	ST_CMD_DATA = 2,
	ST_SEND_START = 3,
	ST_RD_DATA = 4,
	ST_WR_DATA = 5,
	ST_SEND_STOP = 6,
	ST_CHECK_WR_ACK = 7,
	ST_CHECK_ADDR_ACK = 8,
	ST_SEND_ACK = 9,
	ST_WR_ADDR = 10;

reg [6:0] addr;	// Address
reg cmd;
wire [7:0] addr_cmd;
assign addr_cmd = {addr, cmd};

reg status_err_nack_addr, status_err_nack_data, status_data_ready;
assign o_status = {status_err_nack_addr, status_err_nack_data, status_data_ready };

assign io_sda = reg_sda;
reg reg_sda;
	 
reg [3:0] state;
reg [3:0] pos_state;
reg [3:0] pos_count;
reg [3:0] neg_state;
reg [3:0] neg_count;

reg wr_sda_neg;
reg wr_sda_pos;	/* Write enable for positive or negative edge of clock  */
reg reg_sda_pos;
reg reg_sda_neg;	/* Data register for SDA for positive or negative clock */
reg [7:0] in_data;/* Data latched from input for write or Buffer for read*/

assign io_scl = i_clk;
initial
begin
	wr_sda_neg = 0;
	wr_sda_pos = 0;
	reg_sda_neg = 0;
	reg_sda_pos = 0;
	status_err_nack_addr = 0;
	status_err_nack_data = 0;
	status_data_ready = 0;
	pos_state = ST_CMD_ADDR;
	neg_state = 0;
	pos_count = 8;
	neg_count = 9;
end

/*
 * There are 2 state variables triggered on pos and neg edge.
 * Always set the state sampled on the next edge based
 * on what the state changed to on the previous edge.
 * This will prevent the following case:
 * on falling edge, state change to check ack. on rising edge,
 * ack is checked and state is changed to write-data. then on
 * next following edge, state should be write-data and not
 * (write-data | check-ack). So we use a priority logic. The or
 * logic wont work. */

always @*
begin
	case (i_clk)
		1:
		begin
			if (pos_state != 0)
				state <= pos_state;
			else
				state <= neg_state;

			/*
			 * Carry forward the bit being written in the previous edge
			 * (pos/neg) if nothing is to be written on the current edge.
			 * This will make sure data remains written for a complete
			 * clock cycle.
			 */
			if (wr_sda_pos == 1'b1)
				reg_sda = reg_sda_pos;
			else if(wr_sda_neg == 1'b1)
				reg_sda = reg_sda_neg;
			else
				reg_sda = 1'bZ;
		end
		0:
		begin
			if (neg_state != 0)
					state <= neg_state;
			else
					state <= pos_state;

			if (wr_sda_neg == 1'b1)
				reg_sda = reg_sda_neg;
			else if(wr_sda_pos == 1'b1)
				reg_sda = reg_sda_pos;
			else
				reg_sda = 1'bZ;
		end
		endcase;
end

always @(posedge i_clk)
begin
	pos_state <= 0;
	wr_sda_pos <= 0;

	case(state)
	ST_CMD_ADDR:
	if (i_strobe)
	begin
		addr <= i_addr_data[6:0];
		cmd <= i_cmd;
		if (i_cmd == 1)				// Read
			pos_state <= ST_SEND_START;
		else
			pos_state <= ST_CMD_DATA;
	end
	else
			pos_state <= ST_CMD_ADDR;

	ST_CMD_DATA:
	if (i_strobe)
	begin
		in_data <= i_addr_data;
		pos_state <= ST_SEND_START;
	end

	ST_SEND_START:
	begin
		wr_sda_pos <= 1;
		reg_sda_pos <= 0;
		pos_state <= ST_WR_ADDR;
	end
	ST_SEND_STOP:
	begin
		wr_sda_pos <= 1;
		reg_sda_pos <= 1;
		pos_state <= ST_CMD_ADDR;
		status_data_ready <= 1;
	end
	
	ST_SEND_ACK:
	begin
	/* Ack was sent last falling edge, and the slave is sampling it.
	 * Prepare for Stop command next falling edge */
		pos_state <= ST_SEND_STOP;
		o_data <= in_data;
	end
	endcase

	if (state == ST_CHECK_WR_ACK || state == ST_CHECK_ADDR_ACK)
		if (io_sda != 0)				/* Its a write NACK */
		begin
			if (state == ST_CHECK_ADDR_ACK)
				status_err_nack_addr <= 1;
			else
				status_err_nack_data <= 1;
			pos_state <= ST_CMD_ADDR;	/* Back to default state; */
		end
		else
		begin
			if (state == ST_CHECK_ADDR_ACK)
				if (cmd == 1)
					pos_state <= ST_RD_DATA;
				else
					pos_state <= ST_WR_DATA;
			else
				pos_state <= ST_SEND_STOP;
		end

	if (state == ST_RD_DATA)
	begin
		if (pos_count > 0)
		begin
			in_data[pos_count-1] <= io_sda;	// pos_count = 8..1
			pos_count <= pos_count - 1'b1;
		end
		if (pos_count == 1)
		begin
			/*
			 * We just read the last data bit. Prepare to put Ack on the bus on the
			 * next falling edge.
			 */
			 pos_state <= ST_SEND_ACK;
			 pos_count <= 8;
		end
		else
			/*
			 * More bits to go, continue to read
			 */
			pos_state <= ST_RD_DATA;
	end
end

always @(negedge i_clk)
begin
	neg_state <= 0;
	wr_sda_neg <= 0; /* Always release bus on next falling edge by default */

	if (state == ST_WR_DATA || state == ST_WR_ADDR)
	begin
		if (neg_count == 1)
		begin
			neg_count <= 9;			/* Reset counter for future use */
			if (state == ST_WR_DATA)
				neg_state <= ST_CHECK_WR_ACK;
			else
				neg_state <= ST_CHECK_ADDR_ACK;
		end
		else
		begin
			wr_sda_neg <= 1;
			if (state == ST_WR_DATA)
				reg_sda_neg <= in_data[neg_count - 2]; /* neg_count = 9..2 */
			else
				reg_sda_neg <= addr_cmd[neg_count - 2];
			neg_count <= neg_count - 1'b1;
			neg_state <= state;		/* Continue to write data or address */
		end
	end

	if (state == ST_SEND_STOP || state == ST_SEND_ACK)
	begin
		/*
		 * Before next rising edge, pull the Data line low.
		 * Incase of stop command, next rising edge will pull line high.
		 * Incase of Ack, next rising edge will keep line low to signal Ack.
		 */
		wr_sda_neg <= 1;
		reg_sda_neg <= 0;
	end
end
endmodule
