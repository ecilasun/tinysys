`timescale 1ns / 1ps

// 1x5 Device Address Router

module devicerouter(
	input wire aclk,
	input wire aresetn,
    axi4if.slave axi_s,
    wire [18:0] addressmask[10:0], // Bits [30:12] of the physical address
    axi4if.master axi_m[10:0]);

// ------------------------------------------------------------------------------------
// Write router
// ------------------------------------------------------------------------------------

logic [10:0] validwaddr = 11'd0;

always_comb begin
	unique case(axi_s.awaddr[30:12])
		addressmask[0]:  validwaddr = 11'b00000000001;
		addressmask[1]:  validwaddr = 11'b00000000010;
		addressmask[2]:  validwaddr = 11'b00000000100;
		addressmask[3]:  validwaddr = 11'b00000001000;
		addressmask[4]:  validwaddr = 11'b00000010000;
		addressmask[5]:  validwaddr = 11'b00000100000;
		addressmask[6]:  validwaddr = 11'b00001000000;
		addressmask[7]:  validwaddr = 11'b00010000000;
		addressmask[8]:  validwaddr = 11'b00100000000;
		addressmask[9]:  validwaddr = 11'b01000000000;
		addressmask[10]: validwaddr = 11'b10000000000;
		default:		 validwaddr = 11'b00000000000;
	endcase
end

genvar rgen;
generate
for (rgen=0; rgen<11; rgen++) begin
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
		validwaddr[10]: begin
			axi_s.awready = axi_m[10].awready;
			axi_s.bresp   = axi_m[10].bresp;
			axi_s.bvalid  = axi_m[10].bvalid;
			axi_s.wready  = axi_m[10].wready;
		end
		validwaddr[9]: begin
			axi_s.awready = axi_m[9].awready;
			axi_s.bresp   = axi_m[9].bresp;
			axi_s.bvalid  = axi_m[9].bvalid;
			axi_s.wready  = axi_m[9].wready;
		end
		validwaddr[8]: begin
			axi_s.awready = axi_m[8].awready;
			axi_s.bresp   = axi_m[8].bresp;
			axi_s.bvalid  = axi_m[8].bvalid;
			axi_s.wready  = axi_m[8].wready;
		end
		validwaddr[7]: begin
			axi_s.awready = axi_m[7].awready;
			axi_s.bresp   = axi_m[7].bresp;
			axi_s.bvalid  = axi_m[7].bvalid;
			axi_s.wready  = axi_m[7].wready;
		end
		validwaddr[6]: begin
			axi_s.awready = axi_m[6].awready;
			axi_s.bresp   = axi_m[6].bresp;
			axi_s.bvalid  = axi_m[6].bvalid;
			axi_s.wready  = axi_m[6].wready;
		end
		validwaddr[5]: begin
			axi_s.awready = axi_m[5].awready;
			axi_s.bresp   = axi_m[5].bresp;
			axi_s.bvalid  = axi_m[5].bvalid;
			axi_s.wready  = axi_m[5].wready;
		end
		validwaddr[4]: begin
			axi_s.awready = axi_m[4].awready;
			axi_s.bresp   = axi_m[4].bresp;
			axi_s.bvalid  = axi_m[4].bvalid;
			axi_s.wready  = axi_m[4].wready;
		end
		validwaddr[3]: begin
			axi_s.awready = axi_m[3].awready;
			axi_s.bresp   = axi_m[3].bresp;
			axi_s.bvalid  = axi_m[3].bvalid;
			axi_s.wready  = axi_m[3].wready;
		end
		validwaddr[2]: begin
			axi_s.awready = axi_m[2].awready;
			axi_s.bresp   = axi_m[2].bresp;
			axi_s.bvalid  = axi_m[2].bvalid;
			axi_s.wready  = axi_m[2].wready;
		end
		validwaddr[1]: begin
			axi_s.awready = axi_m[1].awready;
			axi_s.bresp   = axi_m[1].bresp;
			axi_s.bvalid  = axi_m[1].bvalid;
			axi_s.wready  = axi_m[1].wready;
		end
		validwaddr[0]: begin
			axi_s.awready = axi_m[0].awready;
			axi_s.bresp   = axi_m[0].bresp;
			axi_s.bvalid  = axi_m[0].bvalid;
			axi_s.wready  = axi_m[0].wready;
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

logic [10:0] validraddr = 11'd0;

always_comb begin
	unique case(axi_s.araddr[30:12])
		addressmask[0]:  validraddr = 11'b00000000001;
		addressmask[1]:  validraddr = 11'b00000000010;
		addressmask[2]:  validraddr = 11'b00000000100;
		addressmask[3]:  validraddr = 11'b00000001000;
		addressmask[4]:  validraddr = 11'b00000010000;
		addressmask[5]:  validraddr = 11'b00000100000;
		addressmask[6]:  validraddr = 11'b00001000000;
		addressmask[7]:  validraddr = 11'b00010000000;
		addressmask[8]:  validraddr = 11'b00100000000;
		addressmask[9]:  validraddr = 11'b01000000000;
		addressmask[10]: validraddr = 11'b10000000000;
		default:		 validraddr = 11'b00000000000;
	endcase
end

genvar wgen;
generate
for (wgen=0; wgen<11; wgen++) begin
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
		validraddr[10]: begin
			axi_s.arready = axi_m[10].arready;
			axi_s.rdata   = axi_m[10].rdata;
			axi_s.rresp   = axi_m[10].rresp;
			axi_s.rvalid  = axi_m[10].rvalid;
			axi_s.rlast   = axi_m[10].rlast;
		end
		validraddr[9]: begin
			axi_s.arready = axi_m[9].arready;
			axi_s.rdata   = axi_m[9].rdata;
			axi_s.rresp   = axi_m[9].rresp;
			axi_s.rvalid  = axi_m[9].rvalid;
			axi_s.rlast   = axi_m[9].rlast;
		end
		validraddr[8]: begin
			axi_s.arready = axi_m[8].arready;
			axi_s.rdata   = axi_m[8].rdata;
			axi_s.rresp   = axi_m[8].rresp;
			axi_s.rvalid  = axi_m[8].rvalid;
			axi_s.rlast   = axi_m[8].rlast;
		end
		validraddr[7]: begin
			axi_s.arready = axi_m[7].arready;
			axi_s.rdata   = axi_m[7].rdata;
			axi_s.rresp   = axi_m[7].rresp;
			axi_s.rvalid  = axi_m[7].rvalid;
			axi_s.rlast   = axi_m[7].rlast;
		end
		validraddr[6]: begin
			axi_s.arready = axi_m[6].arready;
			axi_s.rdata   = axi_m[6].rdata;
			axi_s.rresp   = axi_m[6].rresp;
			axi_s.rvalid  = axi_m[6].rvalid;
			axi_s.rlast   = axi_m[6].rlast;
		end
		validraddr[5]: begin
			axi_s.arready = axi_m[5].arready;
			axi_s.rdata   = axi_m[5].rdata;
			axi_s.rresp   = axi_m[5].rresp;
			axi_s.rvalid  = axi_m[5].rvalid;
			axi_s.rlast   = axi_m[5].rlast;
		end
		validraddr[4]: begin
			axi_s.arready = axi_m[4].arready;
			axi_s.rdata   = axi_m[4].rdata;
			axi_s.rresp   = axi_m[4].rresp;
			axi_s.rvalid  = axi_m[4].rvalid;
			axi_s.rlast   = axi_m[4].rlast;
		end
		validraddr[3]: begin
			axi_s.arready = axi_m[3].arready;
			axi_s.rdata   = axi_m[3].rdata;
			axi_s.rresp   = axi_m[3].rresp;
			axi_s.rvalid  = axi_m[3].rvalid;
			axi_s.rlast   = axi_m[3].rlast;
		end
		validraddr[2]: begin
			axi_s.arready = axi_m[2].arready;
			axi_s.rdata   = axi_m[2].rdata;
			axi_s.rresp   = axi_m[2].rresp;
			axi_s.rvalid  = axi_m[2].rvalid;
			axi_s.rlast   = axi_m[2].rlast;
		end
		validraddr[1]: begin
			axi_s.arready = axi_m[1].arready;
			axi_s.rdata   = axi_m[1].rdata;
			axi_s.rresp   = axi_m[1].rresp;
			axi_s.rvalid  = axi_m[1].rvalid;
			axi_s.rlast   = axi_m[1].rlast;
		end
		validraddr[0]: begin
			axi_s.arready = axi_m[0].arready;
			axi_s.rdata   = axi_m[0].rdata;
			axi_s.rresp   = axi_m[0].rresp;
			axi_s.rvalid  = axi_m[0].rvalid;
			axi_s.rlast   = axi_m[0].rlast;
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
