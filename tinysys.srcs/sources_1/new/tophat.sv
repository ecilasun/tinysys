`timescale 1ns / 1ps

module tophat(
    input sys_clk,
    input sys_rst_n,

    output wire ddr3_reset_n,
    output wire [0:0] ddr3_cke,
    output wire [0:0] ddr3_ck_p,
    output wire [0:0] ddr3_ck_n,
    output wire ddr3_ras_n,
    output wire ddr3_cas_n,
    output wire ddr3_we_n,
    output wire [2:0] ddr3_ba,
    output wire [13:0] ddr3_addr,
    output wire [0:0] ddr3_odt,
    output wire [1:0] ddr3_dm,
    inout wire [1:0] ddr3_dqs_p,
    inout wire [1:0] ddr3_dqs_n,
    inout wire [15:0] ddr3_dq,

    output wire [1:0] leds);

parameter DLY_CNT = 32'd25000000;
parameter HALF_DLY_CNT = 32'd12500000;

logic ledreg = 1'b0;
logic [31:0]count = 32'd0;

wire core_clk_locked;
wire core_clk, aluclk;
coreclockgen clkinst(
	.clk_in1(sys_clk),
	.core(core_clk),
	.aluclk(aluclk),
	.locked(core_clk_locked),
	.reset(~sys_rst_n));

wire ddr3_clk_locked;
wire sys_clk_i, clk_ref_i;
ddr3clkgen ddr3clkinst(
	.clk_in1(sys_clk),
	.sys_clk_i(sys_clk_i),
	.clk_ref_i(clk_ref_i),
	.locked(ddr3_clk_locked),
	.reset(~sys_rst_n));

always@(posedge core_clk or negedge sys_rst_n)
begin
	if(!sys_rst_n) begin
		count <= 32'd0;
	end else if(count == DLY_CNT) begin
		count <= 32'd0;
	end else begin
		count <= count + 32'd1;
	end
end

//led output register control
always@(posedge core_clk or negedge sys_rst_n)
begin
	if(!sys_rst_n) begin
		ledreg <= 1'b0;
	end	else begin
		ledreg <= count < HALF_DLY_CNT ? 1'b1 : 1'b0;
	end
end

// DDR3 wiring

wire ui_clk, ui_clk_sync_rst, init_calib_complete;
mig_7series_0 memtest (
   // Memory interface ports
    .ddr3_addr                      (ddr3_addr),
    .ddr3_ba                        (ddr3_ba),
    .ddr3_cas_n                     (ddr3_cas_n),
    .ddr3_ck_n                      (ddr3_ck_n),
    .ddr3_ck_p                      (ddr3_ck_p),
    .ddr3_cke                       (ddr3_cke),
    .ddr3_ras_n                     (ddr3_ras_n),
    .ddr3_reset_n                   (ddr3_reset_n),
    .ddr3_we_n                      (ddr3_we_n),
    .ddr3_dq                        (ddr3_dq),
    .ddr3_dqs_n                     (ddr3_dqs_n),
    .ddr3_dqs_p                     (ddr3_dqs_p),
    .init_calib_complete            (init_calib_complete),
    .ddr3_dm                        (ddr3_dm),
    .ddr3_odt                       (ddr3_odt),
    // Application interface ports
    .ui_clk                         (ui_clk),
    .ui_clk_sync_rst                (ui_clk_sync_rst),
    .mmcm_locked                    (),
    .aresetn                        (sys_rst_n),
    .app_sr_req                     (1'b0),
    .app_ref_req                    (1'b0),
    .app_zq_req                     (1'b0),
    .app_sr_active                  (),
    .app_ref_ack                    (),
    .app_zq_ack                     (),
    // Slave Interface Write Address Ports
    .s_axi_awid                     (),
    .s_axi_awaddr                   (),
    .s_axi_awlen                    (),
    .s_axi_awsize                   (),
    .s_axi_awburst                  (),
    .s_axi_awlock                   (),
    .s_axi_awcache                  (),
    .s_axi_awprot                   (),
    .s_axi_awqos                    (),
    .s_axi_awvalid                  (),
    .s_axi_awready                  (),
    // Slave Interface Write Data Ports
    .s_axi_wdata                    (),
    .s_axi_wstrb                    (),
    .s_axi_wlast                    (),
    .s_axi_wvalid                   (),
    .s_axi_wready                   (),
    // Slave Interface Write Response Ports
    .s_axi_bid                      (),
    .s_axi_bresp                    (),
    .s_axi_bvalid                   (),
    .s_axi_bready                   (),
    // Slave Interface Read Address Ports
    .s_axi_arid                     (),
    .s_axi_araddr                   (),
    .s_axi_arlen                    (),
    .s_axi_arsize                   (),
    .s_axi_arburst                  (),
    .s_axi_arlock                   (),
    .s_axi_arcache                  (),
    .s_axi_arprot                   (),
    .s_axi_arqos                    (),
    .s_axi_arvalid                  (),
    .s_axi_arready                  (),
    // Slave Interface Read Data Ports
    .s_axi_rid                      (),
    .s_axi_rdata                    (),
    .s_axi_rresp                    (),
    .s_axi_rlast                    (),
    .s_axi_rvalid                   (),
    .s_axi_rready                   (),
    // System Clock Ports
    .sys_clk_i                      (sys_clk_i),
    // Reference Clock Ports
    .clk_ref_i                      (clk_ref_i),
    .sys_rst                        (sys_rst_n) );

assign leds[1] = ledreg;
assign leds[0] = init_calib_complete;

// Divider test - runs at a separate clock (and faster) than the CPU

logic divstart = 1'b0;
//logic [31:0] dividend = 'd231;	// Test: 231/12 -> Q=19 R=3 -> 19*12+3 == 231
//logic [31:0] divisor = 'd12;
logic [31:0] dividend = 'd2331;	// Test: 2331/1223 -> Q=1 R=1108 -> 1*1223+1108 == 2331
logic [31:0] divisor = 'd1223;
wire divdone;
wire [31:0] quotient;
wire [31:0] remainder;
nonrestoringdiv intdiv(
	.aclk(aluclk),
	.aresetn(sys_rst_n),
	.start(divstart),
	.dividend(dividend),
	.divisor(divisor),
	.done(divdone),
	.quotient(quotient),
	.remainder(remainder) );

logic [3:0] teststate = 'd0;
always @(posedge aluclk) begin
	if (~sys_rst_n) begin
		// 
	end else begin
		divstart <= 1'b0;
		case (teststate)
			'd0: begin
				divstart <= 1'b1;
				teststate <= 'd1;
			end
			'd1: begin
				if (divdone) begin
					// done dividing
					teststate <= 'd2;
				end else begin
					// wait until done
					teststate <= 'd1;
				end
			end
			'd2: begin
				// Stay here
				teststate <= 'd2;
			end
		endcase
	end
end

endmodule
