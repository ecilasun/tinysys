`timescale 1ns / 1ps

// 1x2 Device Address Router

module devicerouter(
	input wire aclk,
	input wire aresetn,
    axi4if.slave axi_s,
    wire [18:0] addressmask[3:0], // Address bits [30:12]
    axi4if.master axi_m[3:0]);

// ------------------------------------------------------------------------------------
// Write router
// ------------------------------------------------------------------------------------

logic [3:0] validwaddr = 4'b0000;

always_comb begin
	unique case(axi_s.awaddr[30:12])
		addressmask[0]: validwaddr = 4'b0001;
		addressmask[1]: validwaddr = 4'b0010;
		addressmask[2]: validwaddr = 4'b0100;
		addressmask[3]: validwaddr = 4'b1000;
		default:		validwaddr = 4'b0000;
	endcase
end

genvar rgen;
generate
for (rgen=0; rgen<4; rgen++) begin
	always_comb begin
		axi_m[rgen].awaddr  = validwaddr[rgen] ? axi_s.awaddr	: 32'd0;
		axi_m[rgen].awvalid = validwaddr[rgen] ? axi_s.awvalid	: 1'b0;
		axi_m[rgen].awlen   = validwaddr[rgen] ? axi_s.awlen	: 0;
		axi_m[rgen].awsize  = validwaddr[rgen] ? axi_s.awsize	: 0;
		axi_m[rgen].awburst = validwaddr[rgen] ? axi_s.awburst	: 0;
		axi_m[rgen].wdata   = validwaddr[rgen] ? axi_s.wdata	: 0;
		axi_m[rgen].wstrb   = validwaddr[rgen] ? axi_s.wstrb	: 'd0;
		axi_m[rgen].wvalid  = validwaddr[rgen] ? axi_s.wvalid	: 1'b0;
		axi_m[rgen].bready  = validwaddr[rgen] ? axi_s.bready	: 1'b0;
		axi_m[rgen].wlast   = validwaddr[rgen] ? axi_s.wlast	: 1'b0;
	end
end
endgenerate

always_comb begin
	unique case(1'b1)
		validwaddr[0]: begin
			axi_s.awready = axi_m[0].awready;
			axi_s.bresp   = axi_m[0].bresp;
			axi_s.bvalid  = axi_m[0].bvalid;
			axi_s.wready  = axi_m[0].wready;
		end
		validwaddr[1]: begin
			axi_s.awready = axi_m[1].awready;
			axi_s.bresp   = axi_m[1].bresp;
			axi_s.bvalid  = axi_m[1].bvalid;
			axi_s.wready  = axi_m[1].wready;
		end
		validwaddr[2]: begin
			axi_s.awready = axi_m[2].awready;
			axi_s.bresp   = axi_m[2].bresp;
			axi_s.bvalid  = axi_m[2].bvalid;
			axi_s.wready  = axi_m[2].wready;
		end
		validwaddr[3]: begin
			axi_s.awready = axi_m[3].awready;
			axi_s.bresp   = axi_m[3].bresp;
			axi_s.bvalid  = axi_m[3].bvalid;
			axi_s.wready  = axi_m[3].wready;
		end
		default: begin
			// Assume we could write to no-man's land to not stall the bus
			axi_s.awready = 1'b1;
			axi_s.bresp = 2'b11; // No slave at address
			axi_s.bvalid = 1'b1;
			axi_s.wready = 1'b1;
		end
	endcase
end

// ------------------------------------------------------------------------------------
// Read router
// ------------------------------------------------------------------------------------

logic [3:0] validraddr = 4'b0000;

always_comb begin
	unique case(axi_s.araddr[30:12])
		addressmask[0]: validraddr = 4'b0001;
		addressmask[1]: validraddr = 4'b0010;
		addressmask[2]: validraddr = 4'b0100;
		addressmask[3]: validraddr = 4'b1000;
		default:		validraddr = 4'b0000;
	endcase
end

genvar wgen;
generate
for (wgen=0; wgen<4; wgen++) begin
	always_comb begin
		axi_m[wgen].araddr   = validraddr[wgen] ? axi_s.araddr	: 32'd0;
		axi_m[wgen].arlen    = validraddr[wgen] ? axi_s.arlen	: 0;
		axi_m[wgen].arsize   = validraddr[wgen] ? axi_s.arsize	: 0;
		axi_m[wgen].arburst  = validraddr[wgen] ? axi_s.arburst	: 0;
		axi_m[wgen].arvalid  = validraddr[wgen] ? axi_s.arvalid	: 1'b0;
		axi_m[wgen].rready   = validraddr[wgen] ? axi_s.rready	: 1'b0;
	end
end
endgenerate

always_comb begin
	unique case(1'b1)
		validraddr[0]: begin
			axi_s.arready = axi_m[0].arready;
			axi_s.rdata   = axi_m[0].rdata;
			axi_s.rresp   = axi_m[0].rresp;
			axi_s.rvalid  = axi_m[0].rvalid;
			axi_s.rlast   = axi_m[0].rlast;
		end
		validraddr[1]: begin
			axi_s.arready = axi_m[1].arready;
			axi_s.rdata   = axi_m[1].rdata;
			axi_s.rresp   = axi_m[1].rresp;
			axi_s.rvalid  = axi_m[1].rvalid;
			axi_s.rlast   = axi_m[1].rlast;
		end
		validraddr[2]: begin
			axi_s.arready = axi_m[2].arready;
			axi_s.rdata   = axi_m[2].rdata;
			axi_s.rresp   = axi_m[2].rresp;
			axi_s.rvalid  = axi_m[2].rvalid;
			axi_s.rlast   = axi_m[2].rlast;
		end
		validraddr[3]: begin
			axi_s.arready = axi_m[3].arready;
			axi_s.rdata   = axi_m[3].rdata;
			axi_s.rresp   = axi_m[3].rresp;
			axi_s.rvalid  = axi_m[3].rvalid;
			axi_s.rlast   = axi_m[3].rlast;
		end
		default: begin
			// Assume we could read from no-man's land to not stall the bus
			axi_s.arready = 1'b1;
			axi_s.rdata   = 'd0;
			axi_s.rresp   = 2'b11; // No slave at address
			axi_s.rvalid  = 1'b1;
			axi_s.rlast   = 1'b1;
		end
	endcase
end

endmodule
