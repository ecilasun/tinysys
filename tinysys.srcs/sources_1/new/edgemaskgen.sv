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

// https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/

/*
// Tile dimension
static const int stepXSize = 4;
static const int stepYSize = 4;

// 256 bits per vector (logic [255:0] somevector;)
struct Vec16i{
	int16_t val[16];
};

// Once per primitive
Vec16i Edge::init(const Point2D& v0, const Point2D& v1, const Point2D& origin)
{
	// Edge setup
	int16_t A = v0.y - v1.y;
	int16_t B = v1.x - v0.x;
	int16_t C = v0.x*v1.y - v0.y*v1.x;

	// Step deltas
	oneStepX = Vec16i(A * stepXSize);
	oneStepY = Vec16i(B * stepYSize);

	// x/y values for initial pixel block (origin == min corner of tile)
	Vec16i x = Vec16i(origin.x) + Vec16i(0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3);
	Vec16i y = Vec16i(origin.y) + Vec16i(0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3);

	// Edge function values for a grid of 4x4 pixel positions
	return Vec16i(A)*x + Vec16i(B)*y + Vec16i(C);
}

// Once per edge: (1-2) (2-0) (0-1)
Vec16i w0_row = e12.init(v1, v2, p);
Vec16i w1_row = e20.init(v2, v0, p);
Vec16i w2_row = e01.init(v0, v1, p);

// Once per tile
for (p.y = minY; p.y <= maxY; p.y += Edge::stepYSize)
{
	// Barycentric coordinates at start of tile row
	Vec16i w0 = w0_row;
	Vec16i w1 = w1_row;
	Vec16i w2 = w2_row;

	for (p.x = minX; p.x <= maxX; p.x += Edge::stepXSize)
	{
		// If p is on or inside all edges for any pixels,
		// render those pixels.
		Vec16i mask = w0 | w1 | w2; // i.e. (w0[255] | w1[255] | w2[255]) | (w0[223] | w1[223] | w2[223]) | and so on for all 16 values
		if (any(mask >= 0))
			renderPixels(p, w0, w1, w2, mask);

		// One step to the right
		w0 += e12.oneStepX;
		w1 += e20.oneStepX;
		w2 += e01.oneStepX;
	}

	// One row step
	w0_row += e12.oneStepY;
	w1_row += e20.oneStepY;
	w2_row += e01.oneStepY;
}
*/

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
logic signed [17:0] A[0:15];
logic signed [17:0] B[0:15];
logic signed [17:0] partial[0:15];
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
			A[1] <= A[0];
			B[1] <= B[0]+1;
			A[2] <= A[0];
			B[2] <= B[0]+2;
			A[3] <= A[0];
			B[3] <= B[0]+3;

			A[4] <= A[0]+1;
			B[4] <= B[0];
			A[5] <= A[0]+1;
			B[5] <= B[0]+1;
			A[6] <= A[0]+1;
			B[6] <= B[0]+2;
			A[7] <= A[0]+1;
			B[7] <= B[0]+3;

			A[8] <= A[0]+2;
			B[8] <= B[0];
			A[9] <= A[0]+2;
			B[9] <= B[0]+1;
			A[10] <= A[0]+2;
			B[10] <= B[0]+2;
			A[11] <= A[0]+2;
			B[11] <= B[0]+3;

			A[12] <= A[0]+3;
			B[12] <= B[0];
			A[13] <= A[0]+3;
			B[13] <= B[0]+1;
			A[14] <= A[0]+3;
			B[14] <= B[0]+2;
			A[15] <= A[0]+3;
			B[15] <= B[0]+3;

			rstate <= GENMASKB;
		end
		
		GENMASKB: begin
			partial[0] <= B[0]*dy;
			partial[1] <= B[1]*dy;
			partial[2] <= B[2]*dy;
			partial[3] <= B[3]*dy;
			partial[4] <= B[4]*dy;
			partial[5] <= B[5]*dy;
			partial[6] <= B[6]*dy;
			partial[7] <= B[7]*dy;
			partial[8] <= B[8]*dy;
			partial[9] <= B[9]*dy;
			partial[10] <= B[10]*dy;
			partial[11] <= B[11]*dy;
			partial[12] <= B[12]*dy;
			partial[13] <= B[13]*dy;
			partial[14] <= B[14]*dy;
			partial[15] <= B[15]*dy;
			rstate <= GENMASKA;
		end

		GENMASKA: begin
			result[0] <= A[0]*dx + partial[0];
			result[1] <= A[1]*dx + partial[1];
			result[2] <= A[2]*dx + partial[2];
			result[3] <= A[3]*dx + partial[3];
			result[4] <= A[4]*dx + partial[4];
			result[5] <= A[5]*dx + partial[5];
			result[6] <= A[6]*dx + partial[6];
			result[7] <= A[7]*dx + partial[7];
			result[8] <= A[8]*dx + partial[8];
			result[9] <= A[9]*dx + partial[9];
			result[10] <= A[10]*dx + partial[10];
			result[11] <= A[11]*dx + partial[11];
			result[12] <= A[12]*dx + partial[12];
			result[13] <= A[13]*dx + partial[13];
			result[14] <= A[14]*dx + partial[14];
			result[15] <= A[15]*dx + partial[15];
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
