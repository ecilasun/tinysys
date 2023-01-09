-- Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2022.1.2 (win64) Build 3605665 Fri Aug  5 22:53:37 MDT 2022
-- Date        : Sun Jan  8 23:13:21 2023
-- Host        : enci-pc running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub g:/tinysys/tinysys.gen/sources_1/ip/coreclockgen/coreclockgen_stub.vhdl
-- Design      : coreclockgen
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a200tfbg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity coreclockgen is
  Port ( 
    core : out STD_LOGIC;
    aluclk : out STD_LOGIC;
    reset : in STD_LOGIC;
    locked : out STD_LOGIC;
    clk_in1 : in STD_LOGIC
  );

end coreclockgen;

architecture stub of coreclockgen is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "core,aluclk,reset,locked,clk_in1";
begin
end;
