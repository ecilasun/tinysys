-makelib xcelium_lib/xpm -sv \
  "C:/Xilinx/Vivado/2022.1/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
-endlib
-makelib xcelium_lib/xpm \
  "C:/Xilinx/Vivado/2022.1/data/ip/xpm/xpm_VCOMP.vhd" \
-endlib
-makelib xcelium_lib/xil_defaultlib \
  "../../../../tinysys.gen/sources_1/ip/ddr3clkgen/ddr3clkgen_clk_wiz.v" \
  "../../../../tinysys.gen/sources_1/ip/ddr3clkgen/ddr3clkgen.v" \
-endlib
-makelib xcelium_lib/xil_defaultlib \
  glbl.v
-endlib

