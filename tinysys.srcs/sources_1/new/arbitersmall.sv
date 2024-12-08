`timescale 1ns / 1ps
`default_nettype none

module arbitersmall(
	input wire aclk,
	input wire aresetn,
	axi4if.slave axi_s[1:0],	// To slave in ports of master devices
	axi4if.master axi_m );		// To master in port of slave device

// --------------------------------------------------
// Common
// --------------------------------------------------

// Round-robin arbiter with independent read and write arbitration

// NOTE: Expand the enum and state machine for more devices
typedef enum logic [1:0] {INIT, ARBITRATE0, ARBITRATE1, GRANTED} arbiterstatetype;

// --------------------------------------------------
// Read arbiter
// --------------------------------------------------

arbiterstatetype readstate = INIT;
arbiterstatetype rarbstate = ARBITRATE0;

logic [1:0] rreq;
logic [1:0] rgrant;
logic rreqcomplete = 0;

// A request is considered only when an incoming read or write address is valid
genvar rreqgen;
generate
for (rreqgen=0; rreqgen<2; rreqgen++) begin
	always_comb begin
		rreq[rreqgen] = axi_s[rreqgen].arvalid;
	end
end
endgenerate

// A read grant is complete once we get a notification for read completion
always_comb begin
	rreqcomplete = axi_m.rvalid && axi_m.rlast;
end

// Read grants
genvar rgnt;
generate
for (rgnt=0; rgnt<2; rgnt++) begin
	always_comb begin
		axi_s[rgnt].arready = rgrant[rgnt] ? axi_m.arready : 0;
		axi_s[rgnt].rdata   = rgrant[rgnt] ? axi_m.rdata : 'dz;
		axi_s[rgnt].rresp   = rgrant[rgnt] ? axi_m.rresp : 0;
		axi_s[rgnt].rvalid  = rgrant[rgnt] ? axi_m.rvalid : 0;
		axi_s[rgnt].rlast   = rgrant[rgnt] ? axi_m.rlast : 0;
	end
end
endgenerate

always_comb begin
	unique case(1'b1)
		rgrant[1]: begin
			axi_m.araddr	= axi_s[1].araddr;
			axi_m.arvalid	= axi_s[1].arvalid;
			axi_m.arlen		= axi_s[1].arlen;
			axi_m.arsize	= axi_s[1].arsize;
			axi_m.arburst	= axi_s[1].arburst;
			axi_m.rready	= axi_s[1].rready;
		end
		rgrant[0]: begin
			axi_m.araddr	= axi_s[0].araddr;
			axi_m.arvalid	= axi_s[0].arvalid;
			axi_m.arlen		= axi_s[0].arlen;
			axi_m.arsize	= axi_s[0].arsize;
			axi_m.arburst	= axi_s[0].arburst;
			axi_m.rready	= axi_s[0].rready;
		end
		default: begin
			axi_m.araddr	= 32'dz;
			axi_m.arvalid	= 0;
			axi_m.arlen		= 0;
			axi_m.arsize	= 0;
			axi_m.arburst	= 0;
			axi_m.rready	= 0;
		end
	endcase
end

always_ff @(posedge aclk) begin
	unique case(readstate)
		INIT: begin
			rgrant <= 0;
			readstate <= ARBITRATE0;
		end

		ARBITRATE0: begin
			readstate <= (|rreq) ? GRANTED : ARBITRATE0;
			priority case(1'b1)
				rreq[1]: begin rgrant <= 2'b10; rarbstate <= ARBITRATE1; end
				rreq[0]: begin rgrant <= 2'b01; rarbstate <= ARBITRATE0; end
				default: rgrant <= 2'b00;
			endcase
		end

		ARBITRATE1: begin
			readstate <= (|rreq) ? GRANTED : ARBITRATE1;
			priority case(1'b1)
				rreq[0]: begin rgrant <= 2'b01; rarbstate <= ARBITRATE0; end
				rreq[1]: begin rgrant <= 2'b10; rarbstate <= ARBITRATE1; end
				default: rgrant <= 2'b00;
			endcase
		end

		GRANTED: begin
			readstate <= rreqcomplete ? rarbstate : GRANTED;
		end
	endcase

	if (~aresetn) begin
		readstate <= INIT;
		rarbstate <= ARBITRATE0;
	end
end

// --------------------------------------------------
// Write arbiter
// --------------------------------------------------

arbiterstatetype writestate = INIT;
arbiterstatetype warbstate = ARBITRATE0;

logic [1:0] wreq;
logic [1:0] wgrant;
logic wreqcomplete = 0;

// A request is considered only when an incoming read or write address is valid
genvar wreqgen;
generate
for (wreqgen=0; wreqgen<2; wreqgen++) begin
	always_comb begin
		wreq[wreqgen] = axi_s[wreqgen].awvalid;
	end
end
endgenerate

// A write grant is complete once we get a notification for write completion
always_comb begin
	wreqcomplete = axi_m.bvalid;
end

genvar wgnt;
generate
for (wgnt=0; wgnt<2; wgnt++) begin
	always_comb begin
		axi_s[wgnt].awready = wgrant[wgnt] ? axi_m.awready : 0;
		axi_s[wgnt].wready  = wgrant[wgnt] ? axi_m.wready : 0;
		axi_s[wgnt].bresp   = wgrant[wgnt] ? axi_m.bresp : 0;
		axi_s[wgnt].bvalid  = wgrant[wgnt] ? axi_m.bvalid : 0;
	end
end
endgenerate

always_comb begin
	unique case(1'b1)
		wgrant[1]: begin
			axi_m.awaddr	= axi_s[1].awaddr;
			axi_m.awvalid	= axi_s[1].awvalid;
			axi_m.awlen		= axi_s[1].awlen;
			axi_m.awsize	= axi_s[1].awsize;
			axi_m.awburst	= axi_s[1].awburst;
			axi_m.wdata		= axi_s[1].wdata;
			axi_m.wstrb		= axi_s[1].wstrb;
			axi_m.wvalid	= axi_s[1].wvalid;
			axi_m.wlast		= axi_s[1].wlast;
			axi_m.bready	= axi_s[1].bready;
		end
		wgrant[0]: begin
			axi_m.awaddr	= axi_s[0].awaddr;
			axi_m.awvalid	= axi_s[0].awvalid;
			axi_m.awlen		= axi_s[0].awlen;
			axi_m.awsize	= axi_s[0].awsize;
			axi_m.awburst	= axi_s[0].awburst;
			axi_m.wdata		= axi_s[0].wdata;
			axi_m.wstrb		= axi_s[0].wstrb;
			axi_m.wvalid	= axi_s[0].wvalid;
			axi_m.wlast		= axi_s[0].wlast;
			axi_m.bready	= axi_s[0].bready;
		end
		default: begin
			axi_m.awaddr	= 32'dz;
			axi_m.awvalid	= 0;
			axi_m.awlen		= 0;
			axi_m.awsize	= 0;
			axi_m.awburst	= 0;
			axi_m.wdata		= 'dz;
			axi_m.wstrb		= 0;
			axi_m.wvalid	= 0;
			axi_m.wlast		= 0;
			axi_m.bready	= 0;
		end
	endcase
end

always_ff @(posedge aclk) begin
	unique case(writestate)
		INIT: begin
			wgrant <= 0;
			writestate <= ARBITRATE0;
		end

		ARBITRATE0: begin
			writestate <= (|wreq) ? GRANTED : ARBITRATE0;
			priority case(1'b1)
				wreq[1]: begin wgrant <= 2'b10; warbstate <= ARBITRATE1; end
				wreq[0]: begin wgrant <= 2'b01; warbstate <= ARBITRATE0; end
				default: wgrant <= 2'b00;
			endcase
		end

		ARBITRATE1: begin
			writestate <= (|wreq) ? GRANTED : ARBITRATE1;
			priority case(1'b1)
				wreq[0]: begin wgrant <= 2'b01; warbstate <= ARBITRATE0; end
				wreq[1]: begin wgrant <= 2'b10; warbstate <= ARBITRATE1; end
				default: wgrant <= 2'b00;
			endcase
		end

		GRANTED: begin
			writestate <= wreqcomplete ? warbstate : GRANTED;
		end
	endcase

	if (~aresetn) begin
		writestate <= INIT;
		warbstate <= ARBITRATE0;
	end
end

endmodule
