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

/*
// Hardware based on the following function, replicated 16 times for a 4x4 tile
int32_t edgeFunction(const sVec2 &v0, const sVec2 &v1, const sVec2 &p)
{
	// Same as what our hardware does
	int32_t A = (p.y - v0.y); // 16 copies, 1 for each pixel
	int32_t B = (p.x - v0.x); // 16 copies, 1 for each pixel
	int32_t dx = (v0.x - v1.x); // One-time
	int32_t dy = (v1.y - v0.y); // One-time
	return A*dx + B*dy;
}*/

typedef enum logic [2:0] {
    RINIT, RWCMD,
    SETUPRASTER, STARTRASTER,
    GENMASK, GENTWO,
    ENDRASTER } rasterstatetype;
rasterstatetype rstate = RINIT;

// All internal math is 18 bits, sign extended from incoming 16 bits
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
            rstate <= RWCMD;
        end
 
        RWCMD: begin
            // Sign extend and store the values
            x0 <= {v0x[15], v0x[15], v0x};
            y0 <= {v0y[15], v0y[15], v0y};
            x1 <= {v1x[15], v1x[15], v1x};
            y1 <= {v1y[15], v1y[15], v1y};
            tilex <= {tx[15], tx[15], tx};
            tiley <= {ty[15], ty[15], ty};
            rstate <= ena ? SETUPRASTER : RWCMD;
        end
        
        SETUPRASTER: begin
            // A = ty-v0.y
            // B = tx-v0.x
            // dx = v0.x-v1.x
            // dy = v1.y-v0.y
            // det = A*dx + B*dy
            dx <= x0-x1;
            dy <= y1-y0;

            // All other A/Bs are +1..+3 of the same value
            A[0] <= tiley-y0;
            B[0] <= tilex-x0;

            rstate <= STARTRASTER;
        end
 
        STARTRASTER: begin
            // Shifted versions of A[0] and B[0] for each tile pixel
            A[1] <= A[0];
            A[2] <= A[0];
            A[3] <= A[0];
            B[1] <= B[0]+18'd1;
            B[2] <= B[0]+18'd2;
            B[3] <= B[0]+18'd3;

            A[4] <= A[0]+18'd1;
            A[5] <= A[0]+18'd1;
            A[6] <= A[0]+18'd1;
            A[7] <= A[0]+18'd1;
            B[4] <= B[0];
            B[5] <= B[0]+18'd1;
            B[6] <= B[0]+18'd2;
            B[7] <= B[0]+18'd3;

            A[8]  <= A[0]+18'd2;
            A[9]  <= A[0]+18'd2;
            A[10] <= A[0]+18'd2;
            A[11] <= A[0]+18'd2;
            B[8]  <= B[0];
            B[9]  <= B[0]+18'd1;
            B[10] <= B[0]+18'd2;
            B[11] <= B[0]+18'd3;

            A[12] <= A[0]+18'd3;
            A[13] <= A[0]+18'd3;
            A[14] <= A[0]+18'd3;
            A[15] <= A[0]+18'd3;
            B[12] <= B[0];
            B[13] <= B[0]+18'd1;
            B[14] <= B[0]+18'd2;
            B[15] <= B[0]+18'd3;

            rstate <= GENMASK;
        end
        
        GENMASK: begin
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
            rstate <= GENTWO;
        end

        GENTWO: begin
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
        rmask <= 16'd0;
    end
end

endmodule
