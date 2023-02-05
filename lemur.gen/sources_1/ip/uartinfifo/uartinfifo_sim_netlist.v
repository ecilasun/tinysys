// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2022.1.2 (win64) Build 3605665 Fri Aug  5 22:53:37 MDT 2022
// Date        : Sun Feb  5 14:19:16 2023
// Host        : enci-pc running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode funcsim g:/tinysys/lemur.gen/sources_1/ip/uartinfifo/uartinfifo_sim_netlist.v
// Design      : uartinfifo
// Purpose     : This verilog netlist is a functional simulation representation of the design and should not be modified
//               or synthesized. This netlist cannot be used for SDF annotated simulation.
// Device      : xc7a200tfbg484-1
// --------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CHECK_LICENSE_TYPE = "uartinfifo,fifo_generator_v13_2_7,{}" *) (* downgradeipidentifiedwarnings = "yes" *) (* x_core_info = "fifo_generator_v13_2_7,Vivado 2022.1.2" *) 
(* NotValidForBitStream *)
module uartinfifo
   (rst,
    wr_clk,
    rd_clk,
    din,
    wr_en,
    rd_en,
    dout,
    full,
    empty,
    valid);
  input rst;
  (* x_interface_info = "xilinx.com:signal:clock:1.0 write_clk CLK" *) (* x_interface_parameter = "XIL_INTERFACENAME write_clk, FREQ_HZ 10000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, INSERT_VIP 0" *) input wr_clk;
  (* x_interface_info = "xilinx.com:signal:clock:1.0 read_clk CLK" *) (* x_interface_parameter = "XIL_INTERFACENAME read_clk, FREQ_HZ 100000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, INSERT_VIP 0" *) input rd_clk;
  (* x_interface_info = "xilinx.com:interface:fifo_write:1.0 FIFO_WRITE WR_DATA" *) input [7:0]din;
  (* x_interface_info = "xilinx.com:interface:fifo_write:1.0 FIFO_WRITE WR_EN" *) input wr_en;
  (* x_interface_info = "xilinx.com:interface:fifo_read:1.0 FIFO_READ RD_EN" *) input rd_en;
  (* x_interface_info = "xilinx.com:interface:fifo_read:1.0 FIFO_READ RD_DATA" *) output [7:0]dout;
  (* x_interface_info = "xilinx.com:interface:fifo_write:1.0 FIFO_WRITE FULL" *) output full;
  (* x_interface_info = "xilinx.com:interface:fifo_read:1.0 FIFO_READ EMPTY" *) output empty;
  output valid;

  wire [7:0]din;
  wire [7:0]dout;
  wire empty;
  wire full;
  wire rd_clk;
  wire rd_en;
  wire rst;
  wire valid;
  wire wr_clk;
  wire wr_en;
  wire NLW_U0_almost_empty_UNCONNECTED;
  wire NLW_U0_almost_full_UNCONNECTED;
  wire NLW_U0_axi_ar_dbiterr_UNCONNECTED;
  wire NLW_U0_axi_ar_overflow_UNCONNECTED;
  wire NLW_U0_axi_ar_prog_empty_UNCONNECTED;
  wire NLW_U0_axi_ar_prog_full_UNCONNECTED;
  wire NLW_U0_axi_ar_sbiterr_UNCONNECTED;
  wire NLW_U0_axi_ar_underflow_UNCONNECTED;
  wire NLW_U0_axi_aw_dbiterr_UNCONNECTED;
  wire NLW_U0_axi_aw_overflow_UNCONNECTED;
  wire NLW_U0_axi_aw_prog_empty_UNCONNECTED;
  wire NLW_U0_axi_aw_prog_full_UNCONNECTED;
  wire NLW_U0_axi_aw_sbiterr_UNCONNECTED;
  wire NLW_U0_axi_aw_underflow_UNCONNECTED;
  wire NLW_U0_axi_b_dbiterr_UNCONNECTED;
  wire NLW_U0_axi_b_overflow_UNCONNECTED;
  wire NLW_U0_axi_b_prog_empty_UNCONNECTED;
  wire NLW_U0_axi_b_prog_full_UNCONNECTED;
  wire NLW_U0_axi_b_sbiterr_UNCONNECTED;
  wire NLW_U0_axi_b_underflow_UNCONNECTED;
  wire NLW_U0_axi_r_dbiterr_UNCONNECTED;
  wire NLW_U0_axi_r_overflow_UNCONNECTED;
  wire NLW_U0_axi_r_prog_empty_UNCONNECTED;
  wire NLW_U0_axi_r_prog_full_UNCONNECTED;
  wire NLW_U0_axi_r_sbiterr_UNCONNECTED;
  wire NLW_U0_axi_r_underflow_UNCONNECTED;
  wire NLW_U0_axi_w_dbiterr_UNCONNECTED;
  wire NLW_U0_axi_w_overflow_UNCONNECTED;
  wire NLW_U0_axi_w_prog_empty_UNCONNECTED;
  wire NLW_U0_axi_w_prog_full_UNCONNECTED;
  wire NLW_U0_axi_w_sbiterr_UNCONNECTED;
  wire NLW_U0_axi_w_underflow_UNCONNECTED;
  wire NLW_U0_axis_dbiterr_UNCONNECTED;
  wire NLW_U0_axis_overflow_UNCONNECTED;
  wire NLW_U0_axis_prog_empty_UNCONNECTED;
  wire NLW_U0_axis_prog_full_UNCONNECTED;
  wire NLW_U0_axis_sbiterr_UNCONNECTED;
  wire NLW_U0_axis_underflow_UNCONNECTED;
  wire NLW_U0_dbiterr_UNCONNECTED;
  wire NLW_U0_m_axi_arvalid_UNCONNECTED;
  wire NLW_U0_m_axi_awvalid_UNCONNECTED;
  wire NLW_U0_m_axi_bready_UNCONNECTED;
  wire NLW_U0_m_axi_rready_UNCONNECTED;
  wire NLW_U0_m_axi_wlast_UNCONNECTED;
  wire NLW_U0_m_axi_wvalid_UNCONNECTED;
  wire NLW_U0_m_axis_tlast_UNCONNECTED;
  wire NLW_U0_m_axis_tvalid_UNCONNECTED;
  wire NLW_U0_overflow_UNCONNECTED;
  wire NLW_U0_prog_empty_UNCONNECTED;
  wire NLW_U0_prog_full_UNCONNECTED;
  wire NLW_U0_rd_rst_busy_UNCONNECTED;
  wire NLW_U0_s_axi_arready_UNCONNECTED;
  wire NLW_U0_s_axi_awready_UNCONNECTED;
  wire NLW_U0_s_axi_bvalid_UNCONNECTED;
  wire NLW_U0_s_axi_rlast_UNCONNECTED;
  wire NLW_U0_s_axi_rvalid_UNCONNECTED;
  wire NLW_U0_s_axi_wready_UNCONNECTED;
  wire NLW_U0_s_axis_tready_UNCONNECTED;
  wire NLW_U0_sbiterr_UNCONNECTED;
  wire NLW_U0_underflow_UNCONNECTED;
  wire NLW_U0_wr_ack_UNCONNECTED;
  wire NLW_U0_wr_rst_busy_UNCONNECTED;
  wire [4:0]NLW_U0_axi_ar_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_ar_rd_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_ar_wr_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_aw_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_aw_rd_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_aw_wr_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_b_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_b_rd_data_count_UNCONNECTED;
  wire [4:0]NLW_U0_axi_b_wr_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axi_r_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axi_r_rd_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axi_r_wr_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axi_w_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axi_w_rd_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axi_w_wr_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axis_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axis_rd_data_count_UNCONNECTED;
  wire [10:0]NLW_U0_axis_wr_data_count_UNCONNECTED;
  wire [9:0]NLW_U0_data_count_UNCONNECTED;
  wire [31:0]NLW_U0_m_axi_araddr_UNCONNECTED;
  wire [1:0]NLW_U0_m_axi_arburst_UNCONNECTED;
  wire [3:0]NLW_U0_m_axi_arcache_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_arid_UNCONNECTED;
  wire [7:0]NLW_U0_m_axi_arlen_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_arlock_UNCONNECTED;
  wire [2:0]NLW_U0_m_axi_arprot_UNCONNECTED;
  wire [3:0]NLW_U0_m_axi_arqos_UNCONNECTED;
  wire [3:0]NLW_U0_m_axi_arregion_UNCONNECTED;
  wire [2:0]NLW_U0_m_axi_arsize_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_aruser_UNCONNECTED;
  wire [31:0]NLW_U0_m_axi_awaddr_UNCONNECTED;
  wire [1:0]NLW_U0_m_axi_awburst_UNCONNECTED;
  wire [3:0]NLW_U0_m_axi_awcache_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_awid_UNCONNECTED;
  wire [7:0]NLW_U0_m_axi_awlen_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_awlock_UNCONNECTED;
  wire [2:0]NLW_U0_m_axi_awprot_UNCONNECTED;
  wire [3:0]NLW_U0_m_axi_awqos_UNCONNECTED;
  wire [3:0]NLW_U0_m_axi_awregion_UNCONNECTED;
  wire [2:0]NLW_U0_m_axi_awsize_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_awuser_UNCONNECTED;
  wire [63:0]NLW_U0_m_axi_wdata_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_wid_UNCONNECTED;
  wire [7:0]NLW_U0_m_axi_wstrb_UNCONNECTED;
  wire [0:0]NLW_U0_m_axi_wuser_UNCONNECTED;
  wire [7:0]NLW_U0_m_axis_tdata_UNCONNECTED;
  wire [0:0]NLW_U0_m_axis_tdest_UNCONNECTED;
  wire [0:0]NLW_U0_m_axis_tid_UNCONNECTED;
  wire [0:0]NLW_U0_m_axis_tkeep_UNCONNECTED;
  wire [0:0]NLW_U0_m_axis_tstrb_UNCONNECTED;
  wire [3:0]NLW_U0_m_axis_tuser_UNCONNECTED;
  wire [9:0]NLW_U0_rd_data_count_UNCONNECTED;
  wire [0:0]NLW_U0_s_axi_bid_UNCONNECTED;
  wire [1:0]NLW_U0_s_axi_bresp_UNCONNECTED;
  wire [0:0]NLW_U0_s_axi_buser_UNCONNECTED;
  wire [63:0]NLW_U0_s_axi_rdata_UNCONNECTED;
  wire [0:0]NLW_U0_s_axi_rid_UNCONNECTED;
  wire [1:0]NLW_U0_s_axi_rresp_UNCONNECTED;
  wire [0:0]NLW_U0_s_axi_ruser_UNCONNECTED;
  wire [9:0]NLW_U0_wr_data_count_UNCONNECTED;

  (* C_ADD_NGC_CONSTRAINT = "0" *) 
  (* C_APPLICATION_TYPE_AXIS = "0" *) 
  (* C_APPLICATION_TYPE_RACH = "0" *) 
  (* C_APPLICATION_TYPE_RDCH = "0" *) 
  (* C_APPLICATION_TYPE_WACH = "0" *) 
  (* C_APPLICATION_TYPE_WDCH = "0" *) 
  (* C_APPLICATION_TYPE_WRCH = "0" *) 
  (* C_AXIS_TDATA_WIDTH = "8" *) 
  (* C_AXIS_TDEST_WIDTH = "1" *) 
  (* C_AXIS_TID_WIDTH = "1" *) 
  (* C_AXIS_TKEEP_WIDTH = "1" *) 
  (* C_AXIS_TSTRB_WIDTH = "1" *) 
  (* C_AXIS_TUSER_WIDTH = "4" *) 
  (* C_AXIS_TYPE = "0" *) 
  (* C_AXI_ADDR_WIDTH = "32" *) 
  (* C_AXI_ARUSER_WIDTH = "1" *) 
  (* C_AXI_AWUSER_WIDTH = "1" *) 
  (* C_AXI_BUSER_WIDTH = "1" *) 
  (* C_AXI_DATA_WIDTH = "64" *) 
  (* C_AXI_ID_WIDTH = "1" *) 
  (* C_AXI_LEN_WIDTH = "8" *) 
  (* C_AXI_LOCK_WIDTH = "1" *) 
  (* C_AXI_RUSER_WIDTH = "1" *) 
  (* C_AXI_TYPE = "1" *) 
  (* C_AXI_WUSER_WIDTH = "1" *) 
  (* C_COMMON_CLOCK = "0" *) 
  (* C_COUNT_TYPE = "0" *) 
  (* C_DATA_COUNT_WIDTH = "10" *) 
  (* C_DEFAULT_VALUE = "BlankString" *) 
  (* C_DIN_WIDTH = "8" *) 
  (* C_DIN_WIDTH_AXIS = "1" *) 
  (* C_DIN_WIDTH_RACH = "32" *) 
  (* C_DIN_WIDTH_RDCH = "64" *) 
  (* C_DIN_WIDTH_WACH = "1" *) 
  (* C_DIN_WIDTH_WDCH = "64" *) 
  (* C_DIN_WIDTH_WRCH = "2" *) 
  (* C_DOUT_RST_VAL = "0" *) 
  (* C_DOUT_WIDTH = "8" *) 
  (* C_ENABLE_RLOCS = "0" *) 
  (* C_ENABLE_RST_SYNC = "1" *) 
  (* C_EN_SAFETY_CKT = "0" *) 
  (* C_ERROR_INJECTION_TYPE = "0" *) 
  (* C_ERROR_INJECTION_TYPE_AXIS = "0" *) 
  (* C_ERROR_INJECTION_TYPE_RACH = "0" *) 
  (* C_ERROR_INJECTION_TYPE_RDCH = "0" *) 
  (* C_ERROR_INJECTION_TYPE_WACH = "0" *) 
  (* C_ERROR_INJECTION_TYPE_WDCH = "0" *) 
  (* C_ERROR_INJECTION_TYPE_WRCH = "0" *) 
  (* C_FAMILY = "artix7" *) 
  (* C_FULL_FLAGS_RST_VAL = "0" *) 
  (* C_HAS_ALMOST_EMPTY = "0" *) 
  (* C_HAS_ALMOST_FULL = "0" *) 
  (* C_HAS_AXIS_TDATA = "1" *) 
  (* C_HAS_AXIS_TDEST = "0" *) 
  (* C_HAS_AXIS_TID = "0" *) 
  (* C_HAS_AXIS_TKEEP = "0" *) 
  (* C_HAS_AXIS_TLAST = "0" *) 
  (* C_HAS_AXIS_TREADY = "1" *) 
  (* C_HAS_AXIS_TSTRB = "0" *) 
  (* C_HAS_AXIS_TUSER = "1" *) 
  (* C_HAS_AXI_ARUSER = "0" *) 
  (* C_HAS_AXI_AWUSER = "0" *) 
  (* C_HAS_AXI_BUSER = "0" *) 
  (* C_HAS_AXI_ID = "0" *) 
  (* C_HAS_AXI_RD_CHANNEL = "1" *) 
  (* C_HAS_AXI_RUSER = "0" *) 
  (* C_HAS_AXI_WR_CHANNEL = "1" *) 
  (* C_HAS_AXI_WUSER = "0" *) 
  (* C_HAS_BACKUP = "0" *) 
  (* C_HAS_DATA_COUNT = "0" *) 
  (* C_HAS_DATA_COUNTS_AXIS = "0" *) 
  (* C_HAS_DATA_COUNTS_RACH = "0" *) 
  (* C_HAS_DATA_COUNTS_RDCH = "0" *) 
  (* C_HAS_DATA_COUNTS_WACH = "0" *) 
  (* C_HAS_DATA_COUNTS_WDCH = "0" *) 
  (* C_HAS_DATA_COUNTS_WRCH = "0" *) 
  (* C_HAS_INT_CLK = "0" *) 
  (* C_HAS_MASTER_CE = "0" *) 
  (* C_HAS_MEMINIT_FILE = "0" *) 
  (* C_HAS_OVERFLOW = "0" *) 
  (* C_HAS_PROG_FLAGS_AXIS = "0" *) 
  (* C_HAS_PROG_FLAGS_RACH = "0" *) 
  (* C_HAS_PROG_FLAGS_RDCH = "0" *) 
  (* C_HAS_PROG_FLAGS_WACH = "0" *) 
  (* C_HAS_PROG_FLAGS_WDCH = "0" *) 
  (* C_HAS_PROG_FLAGS_WRCH = "0" *) 
  (* C_HAS_RD_DATA_COUNT = "0" *) 
  (* C_HAS_RD_RST = "0" *) 
  (* C_HAS_RST = "1" *) 
  (* C_HAS_SLAVE_CE = "0" *) 
  (* C_HAS_SRST = "0" *) 
  (* C_HAS_UNDERFLOW = "0" *) 
  (* C_HAS_VALID = "1" *) 
  (* C_HAS_WR_ACK = "0" *) 
  (* C_HAS_WR_DATA_COUNT = "0" *) 
  (* C_HAS_WR_RST = "0" *) 
  (* C_IMPLEMENTATION_TYPE = "6" *) 
  (* C_IMPLEMENTATION_TYPE_AXIS = "1" *) 
  (* C_IMPLEMENTATION_TYPE_RACH = "1" *) 
  (* C_IMPLEMENTATION_TYPE_RDCH = "1" *) 
  (* C_IMPLEMENTATION_TYPE_WACH = "1" *) 
  (* C_IMPLEMENTATION_TYPE_WDCH = "1" *) 
  (* C_IMPLEMENTATION_TYPE_WRCH = "1" *) 
  (* C_INIT_WR_PNTR_VAL = "0" *) 
  (* C_INTERFACE_TYPE = "0" *) 
  (* C_MEMORY_TYPE = "4" *) 
  (* C_MIF_FILE_NAME = "BlankString" *) 
  (* C_MSGON_VAL = "1" *) 
  (* C_OPTIMIZATION_MODE = "0" *) 
  (* C_OVERFLOW_LOW = "0" *) 
  (* C_POWER_SAVING_MODE = "0" *) 
  (* C_PRELOAD_LATENCY = "0" *) 
  (* C_PRELOAD_REGS = "1" *) 
  (* C_PRIM_FIFO_TYPE = "1kx18" *) 
  (* C_PRIM_FIFO_TYPE_AXIS = "1kx18" *) 
  (* C_PRIM_FIFO_TYPE_RACH = "512x36" *) 
  (* C_PRIM_FIFO_TYPE_RDCH = "1kx36" *) 
  (* C_PRIM_FIFO_TYPE_WACH = "512x36" *) 
  (* C_PRIM_FIFO_TYPE_WDCH = "1kx36" *) 
  (* C_PRIM_FIFO_TYPE_WRCH = "512x36" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL = "6" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL_AXIS = "1022" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL_RACH = "1022" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL_RDCH = "1022" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL_WACH = "1022" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL_WDCH = "1022" *) 
  (* C_PROG_EMPTY_THRESH_ASSERT_VAL_WRCH = "1022" *) 
  (* C_PROG_EMPTY_THRESH_NEGATE_VAL = "7" *) 
  (* C_PROG_EMPTY_TYPE = "0" *) 
  (* C_PROG_EMPTY_TYPE_AXIS = "0" *) 
  (* C_PROG_EMPTY_TYPE_RACH = "0" *) 
  (* C_PROG_EMPTY_TYPE_RDCH = "0" *) 
  (* C_PROG_EMPTY_TYPE_WACH = "0" *) 
  (* C_PROG_EMPTY_TYPE_WDCH = "0" *) 
  (* C_PROG_EMPTY_TYPE_WRCH = "0" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL = "1017" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL_AXIS = "1023" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL_RACH = "1023" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL_RDCH = "1023" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL_WACH = "1023" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL_WDCH = "1023" *) 
  (* C_PROG_FULL_THRESH_ASSERT_VAL_WRCH = "1023" *) 
  (* C_PROG_FULL_THRESH_NEGATE_VAL = "1016" *) 
  (* C_PROG_FULL_TYPE = "0" *) 
  (* C_PROG_FULL_TYPE_AXIS = "0" *) 
  (* C_PROG_FULL_TYPE_RACH = "0" *) 
  (* C_PROG_FULL_TYPE_RDCH = "0" *) 
  (* C_PROG_FULL_TYPE_WACH = "0" *) 
  (* C_PROG_FULL_TYPE_WDCH = "0" *) 
  (* C_PROG_FULL_TYPE_WRCH = "0" *) 
  (* C_RACH_TYPE = "0" *) 
  (* C_RDCH_TYPE = "0" *) 
  (* C_RD_DATA_COUNT_WIDTH = "10" *) 
  (* C_RD_DEPTH = "1024" *) 
  (* C_RD_FREQ = "100" *) 
  (* C_RD_PNTR_WIDTH = "10" *) 
  (* C_REG_SLICE_MODE_AXIS = "0" *) 
  (* C_REG_SLICE_MODE_RACH = "0" *) 
  (* C_REG_SLICE_MODE_RDCH = "0" *) 
  (* C_REG_SLICE_MODE_WACH = "0" *) 
  (* C_REG_SLICE_MODE_WDCH = "0" *) 
  (* C_REG_SLICE_MODE_WRCH = "0" *) 
  (* C_SELECT_XPM = "0" *) 
  (* C_SYNCHRONIZER_STAGE = "2" *) 
  (* C_UNDERFLOW_LOW = "0" *) 
  (* C_USE_COMMON_OVERFLOW = "0" *) 
  (* C_USE_COMMON_UNDERFLOW = "0" *) 
  (* C_USE_DEFAULT_SETTINGS = "0" *) 
  (* C_USE_DOUT_RST = "0" *) 
  (* C_USE_ECC = "0" *) 
  (* C_USE_ECC_AXIS = "0" *) 
  (* C_USE_ECC_RACH = "0" *) 
  (* C_USE_ECC_RDCH = "0" *) 
  (* C_USE_ECC_WACH = "0" *) 
  (* C_USE_ECC_WDCH = "0" *) 
  (* C_USE_ECC_WRCH = "0" *) 
  (* C_USE_EMBEDDED_REG = "0" *) 
  (* C_USE_FIFO16_FLAGS = "0" *) 
  (* C_USE_FWFT_DATA_COUNT = "0" *) 
  (* C_USE_PIPELINE_REG = "0" *) 
  (* C_VALID_LOW = "0" *) 
  (* C_WACH_TYPE = "0" *) 
  (* C_WDCH_TYPE = "0" *) 
  (* C_WRCH_TYPE = "0" *) 
  (* C_WR_ACK_LOW = "0" *) 
  (* C_WR_DATA_COUNT_WIDTH = "10" *) 
  (* C_WR_DEPTH = "1024" *) 
  (* C_WR_DEPTH_AXIS = "1024" *) 
  (* C_WR_DEPTH_RACH = "16" *) 
  (* C_WR_DEPTH_RDCH = "1024" *) 
  (* C_WR_DEPTH_WACH = "16" *) 
  (* C_WR_DEPTH_WDCH = "1024" *) 
  (* C_WR_DEPTH_WRCH = "16" *) 
  (* C_WR_FREQ = "10" *) 
  (* C_WR_PNTR_WIDTH = "10" *) 
  (* C_WR_PNTR_WIDTH_AXIS = "10" *) 
  (* C_WR_PNTR_WIDTH_RACH = "4" *) 
  (* C_WR_PNTR_WIDTH_RDCH = "10" *) 
  (* C_WR_PNTR_WIDTH_WACH = "4" *) 
  (* C_WR_PNTR_WIDTH_WDCH = "10" *) 
  (* C_WR_PNTR_WIDTH_WRCH = "4" *) 
  (* C_WR_RESPONSE_LATENCY = "1" *) 
  (* is_du_within_envelope = "true" *) 
  uartinfifo_fifo_generator_v13_2_7 U0
       (.almost_empty(NLW_U0_almost_empty_UNCONNECTED),
        .almost_full(NLW_U0_almost_full_UNCONNECTED),
        .axi_ar_data_count(NLW_U0_axi_ar_data_count_UNCONNECTED[4:0]),
        .axi_ar_dbiterr(NLW_U0_axi_ar_dbiterr_UNCONNECTED),
        .axi_ar_injectdbiterr(1'b0),
        .axi_ar_injectsbiterr(1'b0),
        .axi_ar_overflow(NLW_U0_axi_ar_overflow_UNCONNECTED),
        .axi_ar_prog_empty(NLW_U0_axi_ar_prog_empty_UNCONNECTED),
        .axi_ar_prog_empty_thresh({1'b0,1'b0,1'b0,1'b0}),
        .axi_ar_prog_full(NLW_U0_axi_ar_prog_full_UNCONNECTED),
        .axi_ar_prog_full_thresh({1'b0,1'b0,1'b0,1'b0}),
        .axi_ar_rd_data_count(NLW_U0_axi_ar_rd_data_count_UNCONNECTED[4:0]),
        .axi_ar_sbiterr(NLW_U0_axi_ar_sbiterr_UNCONNECTED),
        .axi_ar_underflow(NLW_U0_axi_ar_underflow_UNCONNECTED),
        .axi_ar_wr_data_count(NLW_U0_axi_ar_wr_data_count_UNCONNECTED[4:0]),
        .axi_aw_data_count(NLW_U0_axi_aw_data_count_UNCONNECTED[4:0]),
        .axi_aw_dbiterr(NLW_U0_axi_aw_dbiterr_UNCONNECTED),
        .axi_aw_injectdbiterr(1'b0),
        .axi_aw_injectsbiterr(1'b0),
        .axi_aw_overflow(NLW_U0_axi_aw_overflow_UNCONNECTED),
        .axi_aw_prog_empty(NLW_U0_axi_aw_prog_empty_UNCONNECTED),
        .axi_aw_prog_empty_thresh({1'b0,1'b0,1'b0,1'b0}),
        .axi_aw_prog_full(NLW_U0_axi_aw_prog_full_UNCONNECTED),
        .axi_aw_prog_full_thresh({1'b0,1'b0,1'b0,1'b0}),
        .axi_aw_rd_data_count(NLW_U0_axi_aw_rd_data_count_UNCONNECTED[4:0]),
        .axi_aw_sbiterr(NLW_U0_axi_aw_sbiterr_UNCONNECTED),
        .axi_aw_underflow(NLW_U0_axi_aw_underflow_UNCONNECTED),
        .axi_aw_wr_data_count(NLW_U0_axi_aw_wr_data_count_UNCONNECTED[4:0]),
        .axi_b_data_count(NLW_U0_axi_b_data_count_UNCONNECTED[4:0]),
        .axi_b_dbiterr(NLW_U0_axi_b_dbiterr_UNCONNECTED),
        .axi_b_injectdbiterr(1'b0),
        .axi_b_injectsbiterr(1'b0),
        .axi_b_overflow(NLW_U0_axi_b_overflow_UNCONNECTED),
        .axi_b_prog_empty(NLW_U0_axi_b_prog_empty_UNCONNECTED),
        .axi_b_prog_empty_thresh({1'b0,1'b0,1'b0,1'b0}),
        .axi_b_prog_full(NLW_U0_axi_b_prog_full_UNCONNECTED),
        .axi_b_prog_full_thresh({1'b0,1'b0,1'b0,1'b0}),
        .axi_b_rd_data_count(NLW_U0_axi_b_rd_data_count_UNCONNECTED[4:0]),
        .axi_b_sbiterr(NLW_U0_axi_b_sbiterr_UNCONNECTED),
        .axi_b_underflow(NLW_U0_axi_b_underflow_UNCONNECTED),
        .axi_b_wr_data_count(NLW_U0_axi_b_wr_data_count_UNCONNECTED[4:0]),
        .axi_r_data_count(NLW_U0_axi_r_data_count_UNCONNECTED[10:0]),
        .axi_r_dbiterr(NLW_U0_axi_r_dbiterr_UNCONNECTED),
        .axi_r_injectdbiterr(1'b0),
        .axi_r_injectsbiterr(1'b0),
        .axi_r_overflow(NLW_U0_axi_r_overflow_UNCONNECTED),
        .axi_r_prog_empty(NLW_U0_axi_r_prog_empty_UNCONNECTED),
        .axi_r_prog_empty_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .axi_r_prog_full(NLW_U0_axi_r_prog_full_UNCONNECTED),
        .axi_r_prog_full_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .axi_r_rd_data_count(NLW_U0_axi_r_rd_data_count_UNCONNECTED[10:0]),
        .axi_r_sbiterr(NLW_U0_axi_r_sbiterr_UNCONNECTED),
        .axi_r_underflow(NLW_U0_axi_r_underflow_UNCONNECTED),
        .axi_r_wr_data_count(NLW_U0_axi_r_wr_data_count_UNCONNECTED[10:0]),
        .axi_w_data_count(NLW_U0_axi_w_data_count_UNCONNECTED[10:0]),
        .axi_w_dbiterr(NLW_U0_axi_w_dbiterr_UNCONNECTED),
        .axi_w_injectdbiterr(1'b0),
        .axi_w_injectsbiterr(1'b0),
        .axi_w_overflow(NLW_U0_axi_w_overflow_UNCONNECTED),
        .axi_w_prog_empty(NLW_U0_axi_w_prog_empty_UNCONNECTED),
        .axi_w_prog_empty_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .axi_w_prog_full(NLW_U0_axi_w_prog_full_UNCONNECTED),
        .axi_w_prog_full_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .axi_w_rd_data_count(NLW_U0_axi_w_rd_data_count_UNCONNECTED[10:0]),
        .axi_w_sbiterr(NLW_U0_axi_w_sbiterr_UNCONNECTED),
        .axi_w_underflow(NLW_U0_axi_w_underflow_UNCONNECTED),
        .axi_w_wr_data_count(NLW_U0_axi_w_wr_data_count_UNCONNECTED[10:0]),
        .axis_data_count(NLW_U0_axis_data_count_UNCONNECTED[10:0]),
        .axis_dbiterr(NLW_U0_axis_dbiterr_UNCONNECTED),
        .axis_injectdbiterr(1'b0),
        .axis_injectsbiterr(1'b0),
        .axis_overflow(NLW_U0_axis_overflow_UNCONNECTED),
        .axis_prog_empty(NLW_U0_axis_prog_empty_UNCONNECTED),
        .axis_prog_empty_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .axis_prog_full(NLW_U0_axis_prog_full_UNCONNECTED),
        .axis_prog_full_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .axis_rd_data_count(NLW_U0_axis_rd_data_count_UNCONNECTED[10:0]),
        .axis_sbiterr(NLW_U0_axis_sbiterr_UNCONNECTED),
        .axis_underflow(NLW_U0_axis_underflow_UNCONNECTED),
        .axis_wr_data_count(NLW_U0_axis_wr_data_count_UNCONNECTED[10:0]),
        .backup(1'b0),
        .backup_marker(1'b0),
        .clk(1'b0),
        .data_count(NLW_U0_data_count_UNCONNECTED[9:0]),
        .dbiterr(NLW_U0_dbiterr_UNCONNECTED),
        .din(din),
        .dout(dout),
        .empty(empty),
        .full(full),
        .injectdbiterr(1'b0),
        .injectsbiterr(1'b0),
        .int_clk(1'b0),
        .m_aclk(1'b0),
        .m_aclk_en(1'b0),
        .m_axi_araddr(NLW_U0_m_axi_araddr_UNCONNECTED[31:0]),
        .m_axi_arburst(NLW_U0_m_axi_arburst_UNCONNECTED[1:0]),
        .m_axi_arcache(NLW_U0_m_axi_arcache_UNCONNECTED[3:0]),
        .m_axi_arid(NLW_U0_m_axi_arid_UNCONNECTED[0]),
        .m_axi_arlen(NLW_U0_m_axi_arlen_UNCONNECTED[7:0]),
        .m_axi_arlock(NLW_U0_m_axi_arlock_UNCONNECTED[0]),
        .m_axi_arprot(NLW_U0_m_axi_arprot_UNCONNECTED[2:0]),
        .m_axi_arqos(NLW_U0_m_axi_arqos_UNCONNECTED[3:0]),
        .m_axi_arready(1'b0),
        .m_axi_arregion(NLW_U0_m_axi_arregion_UNCONNECTED[3:0]),
        .m_axi_arsize(NLW_U0_m_axi_arsize_UNCONNECTED[2:0]),
        .m_axi_aruser(NLW_U0_m_axi_aruser_UNCONNECTED[0]),
        .m_axi_arvalid(NLW_U0_m_axi_arvalid_UNCONNECTED),
        .m_axi_awaddr(NLW_U0_m_axi_awaddr_UNCONNECTED[31:0]),
        .m_axi_awburst(NLW_U0_m_axi_awburst_UNCONNECTED[1:0]),
        .m_axi_awcache(NLW_U0_m_axi_awcache_UNCONNECTED[3:0]),
        .m_axi_awid(NLW_U0_m_axi_awid_UNCONNECTED[0]),
        .m_axi_awlen(NLW_U0_m_axi_awlen_UNCONNECTED[7:0]),
        .m_axi_awlock(NLW_U0_m_axi_awlock_UNCONNECTED[0]),
        .m_axi_awprot(NLW_U0_m_axi_awprot_UNCONNECTED[2:0]),
        .m_axi_awqos(NLW_U0_m_axi_awqos_UNCONNECTED[3:0]),
        .m_axi_awready(1'b0),
        .m_axi_awregion(NLW_U0_m_axi_awregion_UNCONNECTED[3:0]),
        .m_axi_awsize(NLW_U0_m_axi_awsize_UNCONNECTED[2:0]),
        .m_axi_awuser(NLW_U0_m_axi_awuser_UNCONNECTED[0]),
        .m_axi_awvalid(NLW_U0_m_axi_awvalid_UNCONNECTED),
        .m_axi_bid(1'b0),
        .m_axi_bready(NLW_U0_m_axi_bready_UNCONNECTED),
        .m_axi_bresp({1'b0,1'b0}),
        .m_axi_buser(1'b0),
        .m_axi_bvalid(1'b0),
        .m_axi_rdata({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .m_axi_rid(1'b0),
        .m_axi_rlast(1'b0),
        .m_axi_rready(NLW_U0_m_axi_rready_UNCONNECTED),
        .m_axi_rresp({1'b0,1'b0}),
        .m_axi_ruser(1'b0),
        .m_axi_rvalid(1'b0),
        .m_axi_wdata(NLW_U0_m_axi_wdata_UNCONNECTED[63:0]),
        .m_axi_wid(NLW_U0_m_axi_wid_UNCONNECTED[0]),
        .m_axi_wlast(NLW_U0_m_axi_wlast_UNCONNECTED),
        .m_axi_wready(1'b0),
        .m_axi_wstrb(NLW_U0_m_axi_wstrb_UNCONNECTED[7:0]),
        .m_axi_wuser(NLW_U0_m_axi_wuser_UNCONNECTED[0]),
        .m_axi_wvalid(NLW_U0_m_axi_wvalid_UNCONNECTED),
        .m_axis_tdata(NLW_U0_m_axis_tdata_UNCONNECTED[7:0]),
        .m_axis_tdest(NLW_U0_m_axis_tdest_UNCONNECTED[0]),
        .m_axis_tid(NLW_U0_m_axis_tid_UNCONNECTED[0]),
        .m_axis_tkeep(NLW_U0_m_axis_tkeep_UNCONNECTED[0]),
        .m_axis_tlast(NLW_U0_m_axis_tlast_UNCONNECTED),
        .m_axis_tready(1'b0),
        .m_axis_tstrb(NLW_U0_m_axis_tstrb_UNCONNECTED[0]),
        .m_axis_tuser(NLW_U0_m_axis_tuser_UNCONNECTED[3:0]),
        .m_axis_tvalid(NLW_U0_m_axis_tvalid_UNCONNECTED),
        .overflow(NLW_U0_overflow_UNCONNECTED),
        .prog_empty(NLW_U0_prog_empty_UNCONNECTED),
        .prog_empty_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .prog_empty_thresh_assert({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .prog_empty_thresh_negate({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .prog_full(NLW_U0_prog_full_UNCONNECTED),
        .prog_full_thresh({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .prog_full_thresh_assert({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .prog_full_thresh_negate({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .rd_clk(rd_clk),
        .rd_data_count(NLW_U0_rd_data_count_UNCONNECTED[9:0]),
        .rd_en(rd_en),
        .rd_rst(1'b0),
        .rd_rst_busy(NLW_U0_rd_rst_busy_UNCONNECTED),
        .rst(rst),
        .s_aclk(1'b0),
        .s_aclk_en(1'b0),
        .s_aresetn(1'b0),
        .s_axi_araddr({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arburst({1'b0,1'b0}),
        .s_axi_arcache({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arid(1'b0),
        .s_axi_arlen({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arlock(1'b0),
        .s_axi_arprot({1'b0,1'b0,1'b0}),
        .s_axi_arqos({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arready(NLW_U0_s_axi_arready_UNCONNECTED),
        .s_axi_arregion({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arsize({1'b0,1'b0,1'b0}),
        .s_axi_aruser(1'b0),
        .s_axi_arvalid(1'b0),
        .s_axi_awaddr({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axi_awburst({1'b0,1'b0}),
        .s_axi_awcache({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_awid(1'b0),
        .s_axi_awlen({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axi_awlock(1'b0),
        .s_axi_awprot({1'b0,1'b0,1'b0}),
        .s_axi_awqos({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_awready(NLW_U0_s_axi_awready_UNCONNECTED),
        .s_axi_awregion({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_awsize({1'b0,1'b0,1'b0}),
        .s_axi_awuser(1'b0),
        .s_axi_awvalid(1'b0),
        .s_axi_bid(NLW_U0_s_axi_bid_UNCONNECTED[0]),
        .s_axi_bready(1'b0),
        .s_axi_bresp(NLW_U0_s_axi_bresp_UNCONNECTED[1:0]),
        .s_axi_buser(NLW_U0_s_axi_buser_UNCONNECTED[0]),
        .s_axi_bvalid(NLW_U0_s_axi_bvalid_UNCONNECTED),
        .s_axi_rdata(NLW_U0_s_axi_rdata_UNCONNECTED[63:0]),
        .s_axi_rid(NLW_U0_s_axi_rid_UNCONNECTED[0]),
        .s_axi_rlast(NLW_U0_s_axi_rlast_UNCONNECTED),
        .s_axi_rready(1'b0),
        .s_axi_rresp(NLW_U0_s_axi_rresp_UNCONNECTED[1:0]),
        .s_axi_ruser(NLW_U0_s_axi_ruser_UNCONNECTED[0]),
        .s_axi_rvalid(NLW_U0_s_axi_rvalid_UNCONNECTED),
        .s_axi_wdata({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axi_wid(1'b0),
        .s_axi_wlast(1'b0),
        .s_axi_wready(NLW_U0_s_axi_wready_UNCONNECTED),
        .s_axi_wstrb({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axi_wuser(1'b0),
        .s_axi_wvalid(1'b0),
        .s_axis_tdata({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .s_axis_tdest(1'b0),
        .s_axis_tid(1'b0),
        .s_axis_tkeep(1'b0),
        .s_axis_tlast(1'b0),
        .s_axis_tready(NLW_U0_s_axis_tready_UNCONNECTED),
        .s_axis_tstrb(1'b0),
        .s_axis_tuser({1'b0,1'b0,1'b0,1'b0}),
        .s_axis_tvalid(1'b0),
        .sbiterr(NLW_U0_sbiterr_UNCONNECTED),
        .sleep(1'b0),
        .srst(1'b0),
        .underflow(NLW_U0_underflow_UNCONNECTED),
        .valid(valid),
        .wr_ack(NLW_U0_wr_ack_UNCONNECTED),
        .wr_clk(wr_clk),
        .wr_data_count(NLW_U0_wr_data_count_UNCONNECTED[9:0]),
        .wr_en(wr_en),
        .wr_rst(1'b0),
        .wr_rst_busy(NLW_U0_wr_rst_busy_UNCONNECTED));
endmodule
`pragma protect begin_protected
`pragma protect version = 1
`pragma protect encrypt_agent = "XILINX"
`pragma protect encrypt_agent_info = "Xilinx Encryption Tool 2022.1.2"
`pragma protect key_keyowner="Synopsys", key_keyname="SNPS-VCS-RSA-2", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=128)
`pragma protect key_block
BPCTeWzWQR1Yr4NUhz9wjArBJcKSr225+dWtUl60ahf41vMv2w2wtnVldO7D/JfqKVM6SOr7vcE6
uskIj4JfXQ2fpmAMCOmxS8/6iwA/BP18jtjBpOoGMy3NhUAEFt+mVp6dA2eq4srdV4jMhb6/I/gb
kNoplPsr9nL4GSPzl5k=

`pragma protect key_keyowner="Aldec", key_keyname="ALDEC15_001", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
1vvkeKFTWNRaeDgIqh3MubasZ3Hr8zKOYsXwzpIzvSMDkYxMjRl2EoMt/mTRcmvnxBoY6E/qnnwb
+xT5TrGA1RL6wvd5UiOjHdSc8bs6xcX8TsyiUVJVvKrvXVoq8Y7mPIr+uSuY9WdIJqyJ1ZsC8QrX
/hbbl0SBDpW1FvrHTdJN4mEiwKDr3gUH8u60RdV8g4ylawrEdpPQCpMMoH1LNp/PcZw9Z1nOzot8
4PbMoPsUxrbSg8s0G+BgBD4g72Z+H2mCWpeJlwH1NG+2F5tb3nVmVG3wPB7JvYcTQDdBKhh6Sg49
VC8X6ykkmUmj2YC1wKD9oKvRn+AU36PtP8Rt0Q==

`pragma protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-VELOCE-RSA", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=128)
`pragma protect key_block
eNq+I/lLvlmQ5jyI+7OqLNtmHUReTGI1lVXcWpVTvYdaQ+9wQIOA09QiTqm4rdJ/0Wq1r8BVWv1a
yrrXBaoilbU/SX2aJn569SAo60MVh8ofge3JVUJgyNkZZpA/ltm5UQcsuFQhjXfRG8nF1CryCNJb
fb33VREa0GHzqzCpo9U=

`pragma protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-VERIF-SIM-RSA-2", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
FHT8VOlK3qQyuZtp7l7GWPuxUNkawQhb7k8mhEzmHldkGfkqKpbC5K4RKv+plkE3ICbBfw/tDs6k
8RpzTDrASok7fHtKIiWMRgpWOGu6AVyA4unPb1Ed9iT+FXPs0NC1OH5x7Ec2MnUqykdmBXSmHYny
Km072qMbC3lL5Xgzby5nv/urRSn5Hmf1s5i0hjVyctgAa2k3Oh+OcdmSf9wrWm0zkiaxgPo7G3za
7hwarDPHEDWR4Dmk1D84jBK4xVybVG6dhyIEho+aaoCMuoGmIgGI/3IU/SGHPMH9YUIeh+JnDk/I
3OkCGvFCMqcnqDpQzO9eRSK+ONdg/Ko05VrkKQ==

`pragma protect key_keyowner="Real Intent", key_keyname="RI-RSA-KEY-1", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
L3wjmY4ypfd6pQZkS7scSe7K51MTYgEY+RQ+ENtAa2MKY3/mNcVvHvviesb7/DLXSKWQLTkTTsAn
CgEFVju74sJFaC1LpYYGIJoAhBJe/W1PtYOMxmS13338Ax3wau9i4fNsA8A5AEuCbFvHh1BjegjL
as06AIs9hg/8/m/fRr3ToAqe6A7Rc2fy2B7in2NLjoLKiC3hoDuzRdpcyfVpbPIQZBsCtEKd9uy8
cUy73jdKdLINRrEd3g/mv9fM1zrYOPyRlFjmA4z09YHxLxwysu+QTzgc9H7PzPPz7vgEyJo40owO
RlkL2X8aIdXHOs/NVlfOC2U/aOFPmMCVRBgFcg==

`pragma protect key_keyowner="Xilinx", key_keyname="xilinxt_2021_07", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
ubQtFXmfvuSwB0/azxcHg7bjyFosmkLr24hy+RKDl+ETGanMLXQcQWm6M4OIXASoAzTt31KZTbbv
5ntGisBwpzeZ3gpDX7aOZsZnbzZr7K3fMxAmmD6bdKEa3W+rK+w6JzEifUE+cTXUgI524KhsdXGz
tpRktrducCMEQlh44Ru5Uolqymo2hZsz8V0nQB5r/23IixOPxEL2O6lyp98DuzxKxgUFMWROc7GX
FMVZXTeOVw28WC4pwiThLlz6bxkx67OueSn/hQJ9CsZ6EY7bGzW2OEQ+8yGNLedd+XOYhPhCH9KQ
inzbpIDqGr4bN/b1ZYakGRdwlHhAs1eOZGzB8Q==

`pragma protect key_keyowner="Metrics Technologies Inc.", key_keyname="DSim", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
YE2sQgHdhU7yXlGfLiGy1DzLshx79w8q7oMG/w1kzqPXZbru7FpzWTgMW+Q1xOhg/L5jZCrox5k7
hLSiWFtZEB8PiQzrh/HFwFMcv64zrmzek1yMQuW7tnIFcBZAkMUx0x4uuz0LzpD+UH8zGq273bCx
c2Yj0e/mzV25rVYC/9UC6GeHSdFvQ4F62ZxLfO+V0VKdybe4Sgs+Q5Qui5g76Sw7Gc/UPTmkuGhp
8mll06oPeSGM5No9TuWp4/1PsuohXNlBKoi+KIph+4aDzSyL//AVCVVUWKpcv8FRvw+A1sotfXce
f478kSocmnSb9SFOrEh7ByTkiKWzIA9HdeAi0g==

`pragma protect key_keyowner="Atrenta", key_keyname="ATR-SG-RSA-1", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=384)
`pragma protect key_block
YNiQYso7jlz+Qyxh++HhByIu8hKOriBvh/MM4W4ONOEfRvK54HspgoD51g59CG9i/yS79CClWYIo
mKlb/R6nWrd/qKbo5P+3zVgqmsD4zhDNVMp1DgYA+Bj+Ci/V4FIcKZcYLxHe92Bk5h862TDchdSK
khMsheBfioMZoqnkj1bSeR8+9n87M8iLEdf/EgcfWjfW9kE9l7dlanDUJDEmQ5kX8N4UYCoBjtuh
ZBc0f/cOvyY7DGYXfqK0vyGVqja2m3bNPPAZ6dk8mphXItQb/m0lwurFdWJy9Ujpen7+0DIydVDn
GMPx7n8/NoZ6bvHq+42sYF3CjdDkensauNvMfKtc63LIm/ADatECzVHMrl0ueW9z+8tRZhoDIQHW
DE+NWhnsJPFXkQ+Pdv3JEtCOLXfqImWy6OdsTGYRHMw0uboviztV3u51Q48QpuNHJ8qm7yuY10hB
KNwtxCnMGzS8hNgg2DjQ+b3Gr4LAt9VG+8Ln1yEqelM0NlWaHfVQELJ0

`pragma protect key_keyowner="Cadence Design Systems.", key_keyname="CDS_RSA_KEY_VER_1", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
eNXj3NmnszmIvuSuLBM7MG9wIg/QOzOLwZxglHrqYN+tGvu3mOEFgPUBRA03tXXaGMoy2qU5SA8t
Upga9XmcC5QDXKluMOUYhOeXIVi8KvB3Pm6ZUQVWKN0wZy0HkDepAfXRbYBSoBaRrzYMRCu4qeUR
SjhIG4afbgo/VjGzkVoxJdj1xQpVYoYsYexmtAg2F50jh+gEyhzqtJQac6gGSvcVdJn8Rfbhphtz
463+6EeVOo5mQpczIXKGmvTb4TGaLQvgChf1WKsyQpnAFY71L9NJPG25DzQNuEpDvOLnSgH77S7h
RvePhmTPBhgTzdlFfWSnMjHeKeRCqI28MvbP1A==

`pragma protect key_keyowner="Synplicity", key_keyname="SYNP15_1", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
zaw+x9uVa4VRGjRrYX66D/C+Ot+IJHGmstAMVehc9jee5UcEGmhyaGS1jUySFlLTbzQcJ9FWMupc
fZubLAdoRN8YxNq3iOciKmpB1SCnx3V2J1w1DqS90DjzCEs3foabHSwiPL3ZmSKVBAaHVdPfqe5W
0xzHOdnHhG+y8IyCuZCfOFjoOm91LWRiPE2zSGB9UayTmoin+l5HFX+9159tGmlg7AxoW5h5XxxL
DHZqE1ZB+HXNdqv7FD4lTtRv+uCZm9GWM+BkBdrxeZQ5uFsnL8Sm5Gd9SEQZ3KLbwfhcXpoqilrB
cdvt5xt/lqzmPp8AHiQYi3Z6WoH/O6UYi+6PIA==

`pragma protect key_keyowner="Mentor Graphics Corporation", key_keyname="MGC-PREC-RSA", key_method="rsa"
`pragma protect encoding = (enctype="BASE64", line_length=76, bytes=256)
`pragma protect key_block
LfSq+TCZ017XZJ7TxY3o+n/leLAE95+dvwRL7Ew/B+XW2ecA6cA94XmC1d8r+0Co652ZA5fm/rQZ
M58ZC8o2ack2Wm2aBxcOSxYHu0QQF4MVqkQ3tHhzhsEbOAEOKmbnn4822Ow8sgxHMTm07duF9xrq
s/sFQcrH5isFE6kdlFN6RQwf6mTo6qyOo+r0NY/Cqf/F3TFR+BVTkv+GuyfhKi97sbTBQWJZLVs5
wBw1ZhRYjNT24rvl8JhVjgW9XxaEwRISzfTNF3XrGLJPvRn4yzn2b8MrFGgN5+K5V2LeSs05K1Wb
vaADCvJicGhxbaZPE1JuyZH2xhwpZ0KUasGQKg==

`pragma protect data_method = "AES128-CBC"
`pragma protect encoding = (enctype = "BASE64", line_length = 76, bytes = 53760)
`pragma protect data_block
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
kbh5quB+Y4ev
`pragma protect end_protected
`ifndef GLBL
`define GLBL
`timescale  1 ps / 1 ps

module glbl ();

    parameter ROC_WIDTH = 100000;
    parameter TOC_WIDTH = 0;
    parameter GRES_WIDTH = 10000;
    parameter GRES_START = 10000;

//--------   STARTUP Globals --------------
    wire GSR;
    wire GTS;
    wire GWE;
    wire PRLD;
    wire GRESTORE;
    tri1 p_up_tmp;
    tri (weak1, strong0) PLL_LOCKG = p_up_tmp;

    wire PROGB_GLBL;
    wire CCLKO_GLBL;
    wire FCSBO_GLBL;
    wire [3:0] DO_GLBL;
    wire [3:0] DI_GLBL;
   
    reg GSR_int;
    reg GTS_int;
    reg PRLD_int;
    reg GRESTORE_int;

//--------   JTAG Globals --------------
    wire JTAG_TDO_GLBL;
    wire JTAG_TCK_GLBL;
    wire JTAG_TDI_GLBL;
    wire JTAG_TMS_GLBL;
    wire JTAG_TRST_GLBL;

    reg JTAG_CAPTURE_GLBL;
    reg JTAG_RESET_GLBL;
    reg JTAG_SHIFT_GLBL;
    reg JTAG_UPDATE_GLBL;
    reg JTAG_RUNTEST_GLBL;

    reg JTAG_SEL1_GLBL = 0;
    reg JTAG_SEL2_GLBL = 0 ;
    reg JTAG_SEL3_GLBL = 0;
    reg JTAG_SEL4_GLBL = 0;

    reg JTAG_USER_TDO1_GLBL = 1'bz;
    reg JTAG_USER_TDO2_GLBL = 1'bz;
    reg JTAG_USER_TDO3_GLBL = 1'bz;
    reg JTAG_USER_TDO4_GLBL = 1'bz;

    assign (strong1, weak0) GSR = GSR_int;
    assign (strong1, weak0) GTS = GTS_int;
    assign (weak1, weak0) PRLD = PRLD_int;
    assign (strong1, weak0) GRESTORE = GRESTORE_int;

    initial begin
	GSR_int = 1'b1;
	PRLD_int = 1'b1;
	#(ROC_WIDTH)
	GSR_int = 1'b0;
	PRLD_int = 1'b0;
    end

    initial begin
	GTS_int = 1'b1;
	#(TOC_WIDTH)
	GTS_int = 1'b0;
    end

    initial begin 
	GRESTORE_int = 1'b0;
	#(GRES_START);
	GRESTORE_int = 1'b1;
	#(GRES_WIDTH);
	GRESTORE_int = 1'b0;
    end

endmodule
`endif
