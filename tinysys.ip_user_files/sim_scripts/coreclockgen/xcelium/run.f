-makelib xcelium_lib/xpm -sv \
  "C:/Xilinx/Vivado/2022.1/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
-endlib
-makelib xcelium_lib/xpm \
  "C:/Xilinx/Vivado/2022.1/data/ip/xpm/xpm_VCOMP.vhd" \
-endlib
-makelib xcelium_lib/xil_defaultlib \
  "../../../../tinysys.gen/sources_1/ip/coreclockgen/coreclockgen_clk_wiz.v" \
  "../../../../tinysys.gen/sources_1/ip/coreclockgen/coreclockgen.v" \
-endlib
-makelib xcelium_lib/xil_defaultlib \
  glbl.v
-endlib

