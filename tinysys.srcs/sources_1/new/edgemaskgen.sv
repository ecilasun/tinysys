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
    STARTRASTER,
    GENMASK, GENTWO,
    ENDRASTER } rasterstatetype;
rasterstatetype rstate = RINIT;

logic signed [15:0] tilex;
logic signed [15:0] tiley;
logic signed [15:0] x0;
logic signed [15:0] y0;
logic signed [15:0] x1;
logic signed [15:0] y1;
logic signed [15:0] dx;
logic signed [15:0] dy;
logic signed [15:0] A[0:15];
logic signed [15:0] B[0:15];
logic signed [15:0] partial[0:15];
logic signed [15:0] result[0:15];
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
            x0 <= v0x;
            y0 <= v0y;
            x1 <= v1x;
            y1 <= v1y;
            tilex <= tx;
            tiley <= ty;
            rstate <= ena ? STARTRASTER : RWCMD;
        end
 
        STARTRASTER: begin
            dx <= x0-x1;
            dy <= y1-y0;

            A[0] <= tiley+0-x0;
            B[0] <= tilex+0-x0;
            A[1] <= tiley+0-x0;
            B[1] <= tilex+1-x0;
            A[2] <= tiley+0-x0;
            B[2] <= tilex+2-x0;
            A[3] <= tiley+0-x0;
            B[3] <= tilex+3-x0;

            A[4] <= tiley+1-x0;
            B[4] <= tilex+0-x0;
            A[5] <= tiley+1-x0;
            B[5] <= tilex+1-x0;
            A[6] <= tiley+1-x0;
            B[6] <= tilex+2-x0;
            A[7] <= tiley+1-x0;
            B[7] <= tilex+3-x0;

            A[8] <= tiley+2-x0;
            B[8] <= tilex+0-x0;
            A[9] <= tiley+2-x0;
            B[9] <= tilex+1-x0;
            A[10] <= tiley+2-x0;
            B[10] <= tilex+2-x0;
            A[11] <= tiley+2-x0;
            B[11] <= tilex+3-x0;

            A[12] <= tiley+3-x0;
            B[12] <= tilex+0-x0;
            A[13] <= tiley+3-x0;
            B[13] <= tilex+1-x0;
            A[14] <= tiley+3-x0;
            B[14] <= tilex+2-x0;
            A[15] <= tiley+3-x0;
            B[15] <= tilex+3-x0;

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
                result[15]<0?1'b1:1'b0, result[14]<0?1'b1:1'b0, result[13]<0?1'b1:1'b0, result[12]<0?1'b1:1'b0,
                result[11]<0?1'b1:1'b0, result[10]<0?1'b1:1'b0, result[9]<0?1'b1:1'b0,  result[8]<0?1'b1:1'b0,
                result[7]<0?1'b1:1'b0,  result[6]<0?1'b1:1'b0,  result[5]<0?1'b1:1'b0,  result[4]<0?1'b1:1'b0,
                result[3]<0?1'b1:1'b0,  result[2]<0?1'b1:1'b0,  result[1]<0?1'b1:1'b0,  result[0]<0?1'b1:1'b0 };
            bready <= 1'b1;
            rstate <= RWCMD;
        end
    endcase

    if (~rstn) begin
        rmask <= 16'd0;
    end
end

endmodule
