-- Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2022.1.2 (win64) Build 3605665 Fri Aug  5 22:53:37 MDT 2022
-- Date        : Mon Nov  7 21:38:11 2022
-- Host        : enci-pc running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub g:/tinysys/tinysys.gen/sources_1/ip/ddr3clkgen/ddr3clkgen_stub.vhdl
-- Design      : ddr3clkgen
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a200tfbg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity ddr3clkgen is
  Port ( 
    sys_clk_i : out STD_LOGIC;
    clk_ref_i : out STD_LOGIC;
    reset : in STD_LOGIC;
    locked : out STD_LOGIC;
    clk_in1 : in STD_LOGIC
  );

end ddr3clkgen;

architecture stub of ddr3clkgen is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "sys_clk_i,clk_ref_i,reset,locked,clk_in1";
begin
end;
