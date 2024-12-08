`timescale 1ns / 1ps
`default_nettype none

module integerregisterfile(
	input wire clock,			// Writes are clocked, reads are not
	input wire [4:0] rs1,		// Source register 1
	input wire [4:0] rs2,		// Source register 2
	input wire [4:0] rs3,		// Source register 3
	input wire [4:0] rd,		// Destination register
	input wire wren,			// Write enable bit for writing to register rd 
	input wire [31:0] din,		// Data to write to register rd
	output wire [31:0] rval1,	// Register values for rs1 and rs2
	output wire [31:0] rval2,
	output wire [31:0] rval3 );

logic [31:0] registers[0:31];

// The whole register file is zero at boot
initial begin
	int ri;
	for (ri=0; ri<32; ri=ri+1) begin
		registers[ri]  = 32'h00000000;
	end
end

always @(posedge clock) begin
	if (wren && rd != 5'd0) // rd != 5'd0 check is integer regsiter file specific (to avoid writes to zero register)
		registers[rd] <= din;
end

assign rval1 = registers[rs1];
assign rval2 = registers[rs2];
assign rval3 = registers[rs3];

endmodule
