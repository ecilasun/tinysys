`timescale 1ns / 1ps

module edgemaskgen(
	input wire clk,
	input wire rstn,
	// Tile (16x16)
	input wire signed [15:0] tx,
	input wire signed [15:0] ty,
	// Edge
	input wire signed [15:0] v0x,
	input wire signed [15:0] v0y,
	input wire signed [15:0] v1x,
	input wire signed [15:0] v1y,
	// Triggers
	input wire ena,
	output wire ready,
	// Tile mask
	output logic [15:0] rmask);

typedef enum logic [2:0] {
	RINIT, RWCMD,
	SETUPRASTERA, SETUPRASTERB,
	STARTRASTER,
	GENMASKA, GENMASKB,
	ENDRASTER } rasterstatetype;
rasterstatetype rstate = RINIT;

logic signed [17:0] tilex;
logic signed [17:0] tiley;
logic signed [17:0] x0;
logic signed [17:0] y0;
logic signed [17:0] x1;
logic signed [17:0] y1;
logic signed [17:0] dx;
logic signed [17:0] dy;
logic signed [17:0] dx2;
logic signed [17:0] dy2;
logic signed [17:0] dx3;
logic signed [17:0] dy3;
logic signed [17:0] A[0:3];
logic signed [17:0] B[0:3];
logic signed [17:0] result[0:15];
logic bready;

assign ready = bready;

always @(posedge clk) begin

	bready <= 1'b0;

	unique case(rstate)
		RINIT: begin
			// Initial setup (detached from reset)
			rmask <= 16'd0;
			rstate <= RWCMD;
		end
 
		RWCMD: begin
			// Sign extend tile and edge vertices
			x0 <= {v0x[15],v0x[15],v0x};
			y0 <= {v0y[15],v0y[15],v0y};
			x1 <= {v1x[15],v1x[15],v1x};
			y1 <= {v1y[15],v1y[15],v1y};
			tilex <= {tx[15],tx[15],tx};
			tiley <= {ty[15],ty[15],ty};

			rstate <= ena ? SETUPRASTERA : RWCMD;
		end
		
		SETUPRASTERA: begin
			dx <= x0-x1;
			dy <= y1-y0;
			rstate <= SETUPRASTERB;
		end

		SETUPRASTERB: begin
			A[0] <= (tiley-y0)*dx;
			B[0] <= (tilex-x0)*dy;

			dx2 <= dx+dx;
			dx3 <= dx+dx+dx;

			dy2 <= dy+dy;
			dy3 <= dy+dy+dy;

			rstate <= STARTRASTER;
		end
 
		STARTRASTER: begin
			B[1] <= B[0] + dy;
			B[2] <= B[0] + dy2;
			B[3] <= B[0] + dy3;

			A[1] <= A[0] + dx;
			A[2] <= A[0] + dx2;
			A[3] <= A[0] + dx3;

			rstate <= GENMASKA;
		end

		GENMASKA: begin
			result[0]  <= A[0] + B[0];
			result[1]  <= A[0] + B[1];
			result[2]  <= A[0] + B[2];
			result[3]  <= A[0] + B[3];
			result[4]  <= A[1] + B[0];
			result[5]  <= A[1] + B[1];
			result[6]  <= A[1] + B[2];
			result[7]  <= A[1] + B[3];
			rstate <= GENMASKB;
		end

		GENMASKB: begin
			result[8]  <= A[2] + B[0];
			result[9]  <= A[2] + B[1];
			result[10] <= A[2] + B[2];
			result[11] <= A[2] + B[3];
			result[12] <= A[3] + B[0];
			result[13] <= A[3] + B[1];
			result[14] <= A[3] + B[2];
			result[15] <= A[3] + B[3];
			rstate <= ENDRASTER;
		end

		ENDRASTER: begin
			// Gather sign bits into edge mask
			rmask <= {
				result[15]<0?1'b0:1'b1, result[14]<0?1'b0:1'b1, result[13]<0?1'b0:1'b1, result[12]<0?1'b0:1'b1,
				result[11]<0?1'b0:1'b1, result[10]<0?1'b0:1'b1, result[9]<0?1'b0:1'b1,  result[8]<0?1'b0:1'b1,
				result[7]<0?1'b0:1'b1,  result[6]<0?1'b0:1'b1,  result[5]<0?1'b0:1'b1,  result[4]<0?1'b0:1'b1,
				result[3]<0?1'b0:1'b1,  result[2]<0?1'b0:1'b1,  result[1]<0?1'b0:1'b1,  result[0]<0?1'b0:1'b1 };
			bready <= 1'b1;
			rstate <= RWCMD;
		end
	endcase

	if (~rstn) begin
		rstate <= RINIT;
	end
end

endmodule
