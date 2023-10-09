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
	// Output
	output logic signed [17:0] A,
	output logic signed [17:0] B,
	output logic signed [17:0] W);

typedef enum logic [1:0] {
	RINIT,
	RWCMD,
	SETUPRASTER, EDGEEQUATION} rasterstatetype;
rasterstatetype rstate = RINIT;

logic signed [17:0] tilex;
logic signed [17:0] tiley;
logic signed [17:0] x0;
logic signed [17:0] y0;
logic signed [17:0] x1;
logic signed [17:0] y1;
logic signed [17:0] C;
logic bready;

assign ready = bready;

always @(posedge clk) begin

	bready <= 1'b0;

	unique case(rstate)
		RINIT: begin
			// Initial setup (detached from reset)
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

			rstate <= ena ? SETUPRASTER : RWCMD;
		end
		
		SETUPRASTER: begin
			A <= v0y - v1y;
			B <= v1x - v0x;
			C <= v0x*v1y - v0y*v1x;

			rstate <= EDGEEQUATION;
		end

		EDGEEQUATION: begin
			W <= A*tilex + B*tiley + C;
			bready <= 1'b1;

			rstate <= RWCMD;
		end

	endcase

	if (~rstn) begin
		rstate <= RINIT;
	end
end

endmodule
