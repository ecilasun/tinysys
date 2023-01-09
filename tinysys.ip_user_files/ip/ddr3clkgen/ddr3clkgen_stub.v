// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2022.1.2 (win64) Build 3605665 Fri Aug  5 22:53:37 MDT 2022
// Date        : Mon Nov  7 21:38:11 2022
// Host        : enci-pc running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub g:/tinysys/tinysys.gen/sources_1/ip/ddr3clkgen/ddr3clkgen_stub.v
// Design      : ddr3clkgen
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a200tfbg484-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
module ddr3clkgen(sys_clk_i, clk_ref_i, reset, locked, clk_in1)
/* synthesis syn_black_box black_box_pad_pin="sys_clk_i,clk_ref_i,reset,locked,clk_in1" */;
  output sys_clk_i;
  output clk_ref_i;
  input reset;
  output locked;
  input clk_in1;
endmodule
