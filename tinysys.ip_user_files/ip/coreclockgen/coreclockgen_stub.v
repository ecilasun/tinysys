// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2022.1.2 (win64) Build 3605665 Fri Aug  5 22:53:37 MDT 2022
// Date        : Sun Jan  8 23:13:21 2023
// Host        : enci-pc running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub g:/tinysys/tinysys.gen/sources_1/ip/coreclockgen/coreclockgen_stub.v
// Design      : coreclockgen
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a200tfbg484-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
module coreclockgen(core, aluclk, reset, locked, clk_in1)
/* synthesis syn_black_box black_box_pad_pin="core,aluclk,reset,locked,clk_in1" */;
  output core;
  output aluclk;
  input reset;
  output locked;
  input clk_in1;
endmodule
