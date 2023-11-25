/*
 The MIT License (MIT)

 Copyright (c) 2019 Yuya Kudo.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

/*
Taken from
https://github.com/medalotte/SystemVerilog-UART/tree/master
With some bug fixes applied
*/

module uart
  #(parameter
    /*
     You can specify the following three parameters.
     1. DATA_WIDTH : width of data that is transmited by this module
     2. BAUD_RATE  : baud rate of output uart signal
     3. CLK_FREQ   : freqency of input clock signal
    */
    DATA_WIDTH = 8,
    BAUD_RATE  = 115200,
    CLK_FREQ   = 100_000_000)
   (uart_if.rx   rxif,
    uart_if.tx   txif,
    input logic  clk,
    input logic  rstn);

   uart_tx #(DATA_WIDTH, BAUD_RATE, CLK_FREQ)
   uart_tx_inst(.txif(txif),
                .clk(clk),
                .rstn(rstn));

   uart_rx #(DATA_WIDTH, BAUD_RATE, CLK_FREQ)
   uart_rx_inst(.rxif(rxif),
                .clk(clk),
                .rstn(rstn));
endmodule