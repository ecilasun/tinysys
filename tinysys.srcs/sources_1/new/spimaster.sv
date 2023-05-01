`timescale 1ns / 1ps

// https://github.com/nandland/spi-master

// mit license
// 
// copyright (c) 2019 russell-merrick
// 
// permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "software"), to deal
// in the software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the software, and to permit persons to whom the software is
// furnished to do so, subject to the following conditions:
// 
// the above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the software.
// 
// the software is provided "as is", without warranty of any kind, express or
// implied, including but not limited to the warranties of merchantability,
// fitness for a particular purpose and noninfringement. in no event shall the
// authors or copyright holders be liable for any claim, damages or other
// liability, whether in an action of contract, tort or otherwise, arising from,
// out of or in connection with the software or the use or other dealings in the
// software.

///////////////////////////////////////////////////////////////////////////////
// description: spi (serial peripheral interface) master
//              creates master based on input configuration.
//              sends a byte one bit at a time on mosi
//              will also receive byte data one bit at a time on miso.
//              any data on input byte will be shipped out on mosi.
//
//              to kick-off transaction, user must pulse i_tx_dv.
//              this module supports multi-byte transmissions by pulsing
//              i_tx_dv and loading up i_tx_byte when o_tx_ready is high.
//
//              this module is only responsible for controlling clk, mosi, 
//              and miso.  if the spi peripheral requires a chip-select, 
//              this must be done at a higher level.
//
// note:        i_clk must be at least 2x faster than i_spi_clk
//
// parameters:  spi_mode, can be 0, 1, 2, or 3.  see above.
//              can be configured in one of 4 modes:
//              mode | clock polarity (cpol/ckp) | clock phase (cpha)
//               0   |             0             |        0
//               1   |             0             |        1
//               2   |             1             |        0
//               3   |             1             |        1
//              more: https://en.wikipedia.org/wiki/serial_peripheral_interface_bus#mode_numbers
//              clks_per_half_bit - sets frequency of o_spi_clk.  o_spi_clk is
//              derived from i_clk.  set to integer number of clocks for each
//              half-bit of spi data.  e.g. 100 mhz i_clk, clks_per_half_bit = 2
//              would create o_spi_clk of 25 mhz.  must be >= 2
//
///////////////////////////////////////////////////////////////////////////////

module spi_master
  #(parameter spi_mode = 0,
    parameter clks_per_half_bit = 2)
  (
   // control/data signals,
   input        i_rst_l,     // fpga reset
   input        i_clk,       // fpga clock
   
   // tx (mosi) signals
   input [7:0]  i_tx_byte,        // byte to transmit on mosi
   input        i_tx_dv,          // data valid pulse with i_tx_byte
   output reg   o_tx_ready,       // transmit ready for next byte
   
   // rx (miso) signals
   output reg       o_rx_dv,     // data valid pulse (1 clock cycle)
   output reg [7:0] o_rx_byte,   // byte received on miso

   // spi interface
   output reg o_spi_clk,
   input      i_spi_miso,
   output reg o_spi_mosi );

  // spi interface (all runs at spi clock domain)
  wire w_cpol;     // clock polarity
  wire w_cpha;     // clock phase

  reg [$clog2(clks_per_half_bit*2)-1:0] r_spi_clk_count;
  reg r_spi_clk;
  reg [4:0] r_spi_clk_edges;
  reg r_leading_edge;
  reg r_trailing_edge;
  reg       r_tx_dv;
  reg [7:0] r_tx_byte;

  reg [2:0] r_rx_bit_count;
  reg [2:0] r_tx_bit_count;

  // cpol: clock polarity
  // cpol=0 means clock idles at 0, leading edge is rising edge.
  // cpol=1 means clock idles at 1, leading edge is falling edge.
  assign w_cpol  = (spi_mode == 2) | (spi_mode == 3);

  // cpha: clock phase
  // cpha=0 means the "out" side changes the data on trailing edge of clock
  //              the "in" side captures data on leading edge of clock
  // cpha=1 means the "out" side changes the data on leading edge of clock
  //              the "in" side captures data on the trailing edge of clock
  assign w_cpha  = (spi_mode == 1) | (spi_mode == 3);



  // purpose: generate spi clock correct number of times when dv pulse comes
  always @(posedge i_clk or negedge i_rst_l)
  begin
    if (~i_rst_l)
    begin
      o_tx_ready      <= 1'b0;
      r_spi_clk_edges <= 0;
      r_leading_edge  <= 1'b0;
      r_trailing_edge <= 1'b0;
      r_spi_clk       <= w_cpol; // assign default state to idle state
      r_spi_clk_count <= 0;
    end
    else
    begin

      // default assignments
      r_leading_edge  <= 1'b0;
      r_trailing_edge <= 1'b0;
      
      if (i_tx_dv)
      begin
        o_tx_ready      <= 1'b0;
        r_spi_clk_edges <= 16;  // total # edges in one byte always 16
      end
      else if (r_spi_clk_edges > 0)
      begin
        o_tx_ready <= 1'b0;
        
        if (r_spi_clk_count == clks_per_half_bit*2-1)
        begin
          r_spi_clk_edges <= r_spi_clk_edges - 1;
          r_trailing_edge <= 1'b1;
          r_spi_clk_count <= 0;
          r_spi_clk       <= ~r_spi_clk;
        end
        else if (r_spi_clk_count == clks_per_half_bit-1)
        begin
          r_spi_clk_edges <= r_spi_clk_edges - 1;
          r_leading_edge  <= 1'b1;
          r_spi_clk_count <= r_spi_clk_count + 1;
          r_spi_clk       <= ~r_spi_clk;
        end
        else
        begin
          r_spi_clk_count <= r_spi_clk_count + 1;
        end
      end  
      else
      begin
        o_tx_ready <= 1'b1;
      end
      
      
    end // else: !if(~i_rst_l)
  end // always @ (posedge i_clk or negedge i_rst_l)


  // purpose: register i_tx_byte when data valid is pulsed.
  // keeps local storage of byte in case higher level module changes the data
  always @(posedge i_clk or negedge i_rst_l)
  begin
    if (~i_rst_l)
    begin
      r_tx_byte <= 8'h00;
      r_tx_dv   <= 1'b0;
    end
    else
      begin
        r_tx_dv <= i_tx_dv; // 1 clock cycle delay
        if (i_tx_dv)
        begin
          r_tx_byte <= i_tx_byte;
        end
      end // else: !if(~i_rst_l)
  end // always @ (posedge i_clk or negedge i_rst_l)


  // purpose: generate mosi data
  // works with both cpha=0 and cpha=1
  always @(posedge i_clk or negedge i_rst_l)
  begin
    if (~i_rst_l)
    begin
      o_spi_mosi     <= 1'b0;
      r_tx_bit_count <= 3'b111; // send msb first
    end
    else
    begin
      // if ready is high, reset bit counts to default
      if (o_tx_ready)
      begin
        r_tx_bit_count <= 3'b111;
      end
      // catch the case where we start transaction and cpha = 0
      else if (r_tx_dv & ~w_cpha)
      begin
        o_spi_mosi     <= r_tx_byte[3'b111];
        r_tx_bit_count <= 3'b110;
      end
      else if ((r_leading_edge & w_cpha) | (r_trailing_edge & ~w_cpha))
      begin
        r_tx_bit_count <= r_tx_bit_count - 1;
        o_spi_mosi     <= r_tx_byte[r_tx_bit_count];
      end
    end
  end


  // purpose: read in miso data.
  always @(posedge i_clk or negedge i_rst_l)
  begin
    if (~i_rst_l)
    begin
      o_rx_byte      <= 8'h00;
      o_rx_dv        <= 1'b0;
      r_rx_bit_count <= 3'b111;
    end
    else
    begin

      // default assignments
      o_rx_dv   <= 1'b0;

      if (o_tx_ready) // check if ready is high, if so reset bit count to default
      begin
        r_rx_bit_count <= 3'b111;
      end
      else if ((r_leading_edge & ~w_cpha) | (r_trailing_edge & w_cpha))
      begin
        o_rx_byte[r_rx_bit_count] <= i_spi_miso;  // sample data
        r_rx_bit_count            <= r_rx_bit_count - 1;
        if (r_rx_bit_count == 3'b000)
        begin
          o_rx_dv   <= 1'b1;   // byte done, pulse data valid
        end
      end
    end
  end
  
  
  // purpose: add clock delay to signals for alignment.
  always @(posedge i_clk or negedge i_rst_l)
  begin
    if (~i_rst_l)
    begin
      o_spi_clk  <= w_cpol;
    end
    else
      begin
        o_spi_clk <= r_spi_clk;
      end // else: !if(~i_rst_l)
  end // always @ (posedge i_clk or negedge i_rst_l)
  

endmodule // spi_master
