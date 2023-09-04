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
	SETUPRASTER, STARTRASTER,
	GENMASKB, GENMASKA,
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
logic signed [17:0] A[0:3];
logic signed [17:0] B[0:3];
logic signed [17:0] partialA[0:3];
logic signed [17:0] partialB[0:3];
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
			x0 <= {v0x[15],v0x[15],v0x};
			y0 <= {v0y[15],v0y[15],v0y};
			x1 <= {v1x[15],v1x[15],v1x};
			y1 <= {v1y[15],v1y[15],v1y};

			tilex <= {2'b00,tx};
			tiley <= {2'b00,ty};

			rstate <= ena ? SETUPRASTER : RWCMD;
		end
		
		SETUPRASTER: begin
			dx <= x0-x1;
			dy <= y1-y0;

			A[0] <= tiley-y0;
			B[0] <= tilex-x0;

			rstate <= STARTRASTER;
		end
 
		STARTRASTER: begin
			B[1] <= B[0]+1;
			B[2] <= B[0]+2;
			B[3] <= B[0]+3;

			A[1] <= A[0]+1;
			A[2] <= A[0]+2;
			A[3] <= A[0]+3;

			rstate <= GENMASKB;
		end
		
		GENMASKB: begin
			partialA[0] <= A[0]*dx;
			partialA[1] <= A[1]*dx;
			partialA[2] <= A[2]*dx;
			partialA[3] <= A[3]*dx;
			partialB[0] <= B[0]*dy;
			partialB[1] <= B[1]*dy;
			partialB[2] <= B[2]*dy;
			partialB[3] <= B[3]*dy;
			rstate <= GENMASKA;
		end

		GENMASKA: begin
			result[0] <= partialA[0] + partialB[0];
			result[1] <= partialA[0] + partialB[1];
			result[2] <= partialA[0] + partialB[2];
			result[3] <= partialA[0] + partialB[3];
			result[4] <= partialA[1] + partialB[0];
			result[5] <= partialA[1] + partialB[1];
			result[6] <= partialA[1] + partialB[2];
			result[7] <= partialA[1] + partialB[3];
			result[8] <= partialA[2] + partialB[0];
			result[9] <= partialA[2] + partialB[1];
			result[10] <= partialA[2] + partialB[2];
			result[11] <= partialA[2] + partialB[3];
			result[12] <= partialA[3] + partialB[0];
			result[13] <= partialA[3] + partialB[1];
			result[14] <= partialA[3] + partialB[2];
			result[15] <= partialA[3] + partialB[3];
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
