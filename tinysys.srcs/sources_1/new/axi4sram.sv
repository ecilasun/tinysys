// https://github.com/IceNature/sram_controller

/*BSD 3-Clause License

Copyright (c) 2018, IceNature
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

// Used with minor modifications to cope with IS61WV25616EDBLL instead of IS61WV12816BLL 

`timescale 1ns / 1ps

module sram_controller(
	input wire clk100,
	// Control i/o
	input wire rw,
	input wire en,
	input wire [17:0] address,
	input wire [15:0] data_in,
	output reg [15:0] data_out,
	output wire read_finish,
	output wire write_finish,
	// To SRAM pins
	sramwires.def sramconn);
	
typedef enum logic [2:0] {INIT, IDLE, ADDR, BEGIN, WAIT, OPERATE, END} sramstatetype;
sramstatetype sramstate = INIT;

reg [15:0] data_out_reg = 16'b0;

assign read_finish = ((!rw && sramstate == END) ? 1'b1 : 1'b0);
assign write_finish = ((rw && sramstate == END) ? 1'b1 : 1'b0);
assign sramconn.sram_data_inout = rw ? data_out_reg : 16'bz;

always @(posedge clk100) begin
	case (sramstate)
		INIT: begin
			sramstate <= IDLE;
		end
		IDLE: begin
			sramstate <= en ? ADDR : IDLE;
		end
		ADDR:
			sramstate <= BEGIN;
		BEGIN:
			sramstate <= WAIT;
		WAIT:
			sramstate <= OPERATE;
		OPERATE:
			sramstate <= END;
		END:
			sramstate <= IDLE;
		default:
			sramstate <= IDLE;
	endcase
	
	/*if (~aresetn) begin
		sramstate <= IDLE;
	end*/
end

always @(posedge clk100) begin
	case (sramstate)
		ADDR:
			sramconn.sram_addr <= address;
		END:
			sramconn.sram_addr <= 18'd0;
		default:
			sramconn.sram_addr <= sramconn.sram_addr;
	endcase

	/*if (~aresetn) begin
		sramconn.sram_addr <= 18'd0;
	end*/
end

always @(posedge clk100) begin
	case (sramstate)
		BEGIN: begin
			sramconn.sram_cen <= 1'b0;
			sramconn.sram_ub <= 1'b0;
			sramconn.sram_lb <= 1'b0;
		end
		END: begin
			sramconn.sram_cen <= 1'b1;
			sramconn.sram_ub <= 1'b1;
			sramconn.sram_lb <= 1'b1;
		end
		default: begin
			sramconn.sram_cen <= sramconn.sram_cen;
			sramconn.sram_ub <= sramconn.sram_ub;
			sramconn.sram_lb <= sramconn.sram_lb;
		end
	endcase

	/*if (~aresetn) begin
		sramconn.sram_cen <= 1'b0;
		sramconn.sram_ub <= 1'b0;
		sramconn.sram_lb <= 1'b0;
	end*/
end

always @(posedge clk100) begin
	if (rw && sramstate == BEGIN)
		sramconn.sram_we <= 1'b0;
	else if (sramstate == END)
		sramconn.sram_we <= 1'b1;
	else
		sramconn.sram_we <= sramconn.sram_we;

	/*if (~aresetn) begin
		sramconn.sram_we <= 1'b0;
	end*/
end

always @(posedge clk100) begin
	if (!rw && sramstate == BEGIN)
		sramconn.sram_oe <= 1'b0;
	else if (sramstate == END)
		sramconn.sram_oe <= 1'b1;
	else
		sramconn.sram_oe <= sramconn.sram_oe;

	/*if (~aresetn) begin
		sram_oe <= 1'b0;
	end*/
end

always @(posedge clk100) begin
	if (sramstate == OPERATE)
		if (rw) begin
			data_out_reg <= data_in;
			data_out <= sramconn.data_out;
		end else begin
			data_out_reg <= data_out_reg;
			data_out <= sramconn.sram_data_inout;
		end
	else
	begin
		data_out_reg <= data_out_reg;
		data_out <= sramconn.data_out;
	end

	/*if (~aresetn) begin
		sramconn.data_out_reg <= 16'd0;
		sramconn.data_out <= 16'd0;
	end*/
end

endmodule

module axi4sram(
	input wire aclk,
	input wire clk100,
	input wire aresetn,
	axi4if.slave m_axi,
	sramwires.def sramconn);

logic rw = 1'b0;
logic en = 1'b0;
logic [17:0] address;
wire [15:0] data_out;
logic [15:0] data_in;
wire rdone, wdone;

sram_controller sraminst(
	.clk100(clk100),
	.rw(rw),
	.en(en),
	.address(),
	.data_in(data_in),
	.data_out(data_out),
	.read_finish(rdone),
	.write_finish(wdone),
	.sramconn(sramconn));

endmodule
