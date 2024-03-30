`timescale 1ns / 1ps

module axi4gpio(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	inout wire [14:0] gpio );

logic gpiowe = 1'b0;
logic [14:0] gpiodout;

// Output and input values
logic [14:0] gpiooutstate;
logic [14:0] gpioinstate;

// Nothing is input or output by default (all pins floating)
logic [14:0] gpioinputmask;
logic [14:0] gpiooutputmask;

assign gpio[0]  = gpiooutputmask[0]==1'b1  ? gpiooutstate[0]  : 1'bz;
assign gpio[1]  = gpiooutputmask[1]==1'b1  ? gpiooutstate[1]  : 1'bz;
assign gpio[2]  = gpiooutputmask[2]==1'b1  ? gpiooutstate[2]  : 1'bz;
assign gpio[3]  = gpiooutputmask[3]==1'b1  ? gpiooutstate[3]  : 1'bz;
assign gpio[4]  = gpiooutputmask[4]==1'b1  ? gpiooutstate[4]  : 1'bz;
assign gpio[5]  = gpiooutputmask[5]==1'b1  ? gpiooutstate[5]  : 1'bz;
assign gpio[6]  = gpiooutputmask[6]==1'b1  ? gpiooutstate[6]  : 1'bz;
assign gpio[7]  = gpiooutputmask[7]==1'b1  ? gpiooutstate[7]  : 1'bz;
assign gpio[8]  = gpiooutputmask[8]==1'b1  ? gpiooutstate[8]  : 1'bz;
assign gpio[9]  = gpiooutputmask[9]==1'b1  ? gpiooutstate[9]  : 1'bz;
assign gpio[10] = gpiooutputmask[10]==1'b1 ? gpiooutstate[10] : 1'bz;
assign gpio[11] = gpiooutputmask[11]==1'b1 ? gpiooutstate[11] : 1'bz;
assign gpio[12] = gpiooutputmask[12]==1'b1 ? gpiooutstate[12] : 1'bz;
assign gpio[13] = gpiooutputmask[13]==1'b1 ? gpiooutstate[13] : 1'bz;
assign gpio[14] = gpiooutputmask[14]==1'b1 ? gpiooutstate[14] : 1'bz;

logic [3:0] ledbits = 4'd0;
always @(posedge aclk) begin
	if (~aresetn) begin
		gpioinstate <= 15'd0;
		gpiooutstate <= 15'd0;
	end else begin

		if (gpiowe)
			gpiooutstate <= gpiodout;

		gpioinstate[0]  <= gpioinputmask[0]==1'b1  ? gpio[0]  : 1'b0;
		gpioinstate[1]  <= gpioinputmask[1]==1'b1  ? gpio[1]  : 1'b0;
		gpioinstate[2]  <= gpioinputmask[2]==1'b1  ? gpio[2]  : 1'b0;
		gpioinstate[3]  <= gpioinputmask[3]==1'b1  ? gpio[3]  : 1'b0;
		gpioinstate[4]  <= gpioinputmask[4]==1'b1  ? gpio[4]  : 1'b0;
		gpioinstate[5]  <= gpioinputmask[5]==1'b1  ? gpio[5]  : 1'b0;
		gpioinstate[6]  <= gpioinputmask[6]==1'b1  ? gpio[6]  : 1'b0;
		gpioinstate[7]  <= gpioinputmask[7]==1'b1  ? gpio[7]  : 1'b0;
		gpioinstate[8]  <= gpioinputmask[8]==1'b1  ? gpio[8]  : 1'b0;
		gpioinstate[9]  <= gpioinputmask[9]==1'b1  ? gpio[9]  : 1'b0;
		gpioinstate[10] <= gpioinputmask[10]==1'b1 ? gpio[10] : 1'b0;
		gpioinstate[11] <= gpioinputmask[11]==1'b1 ? gpio[11] : 1'b0;
		gpioinstate[12] <= gpioinputmask[12]==1'b1 ? gpio[12] : 1'b0;
		gpioinstate[13] <= gpioinputmask[13]==1'b1 ? gpio[13] : 1'b0;
		gpioinstate[14] <= gpioinputmask[14]==1'b1 ? gpio[14] : 1'b0;
	end
end

assign led = ledbits;

always @(posedge aclk) begin
	if (~aresetn) begin
		gpiodout <= 15'd0;
		gpioinputmask <= 15'd0;
		gpiooutputmask <= 15'd0;
		s_axi.awready <= 1'b0;
		s_axi.arready <= 1'b0;
		s_axi.wready <= 1'b0;
		s_axi.bvalid <= 1'b0;
		s_axi.bresp <= 2'b00;
	end else begin

		s_axi.awready <= s_axi.awvalid;
		s_axi.arready <= s_axi.arvalid;
		s_axi.bvalid <= s_axi.bready;
		s_axi.rvalid <= s_axi.rready;
		s_axi.wready <= s_axi.wvalid;

		gpiowe <= 1'b0;

		// xxxxxxx0 -> data i/o
		// xxxxxxx4 -> pin read mask
		// xxxxxxx8 -> pin write mask

		if (s_axi.rready) begin
			unique case (s_axi.araddr[3:0])
				4'h4: s_axi.rdata[31:0] <= {17'd0, gpioinputmask};
				4'h8: s_axi.rdata[31:0] <= {17'd0, gpiooutputmask};
				default: s_axi.rdata[31:0] <= {17'd0, gpioinstate};
			endcase
			s_axi.rlast <= 1'b1;
		end

		if (s_axi.wvalid) begin
			unique case (s_axi.awaddr[3:0])
				4'h4: begin
					gpioinputmask <= s_axi.wdata[14:0];
				end
				4'h8: begin
					gpiooutputmask <= s_axi.wdata[14:0];
				end
				default: begin
					gpiowe <= 1'b1;
					gpiodout <= s_axi.wdata[14:0];
				end
			endcase
		end
	end
end

endmodule
