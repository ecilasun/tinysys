`timescale 1ns / 1ps

module axi4xadc(
	input wire aclk,
	input wire clk10,
	input wire aresetn,
	axi4if.slave s_axi,
	adcwires.def adcconn, // Also ties to our off-chip ADC device
	output wire [11:0] device_temp);

// --------------------------------------------------
// On-chip ADC for temperature control and DDR3 SDRAM
// --------------------------------------------------

// TEMP is a MSB aligned 12 bit values
(* async_reg = "true" *) logic [11:0] tmp0 = 12'd0;
(* async_reg = "true" *) logic [11:0] devicetemperature = 12'd0;

// Accodring to Xilinx documentation the 12 MSB correspond to temperature
// voltage = 10 * (kT/q) * ln(10);
// where k is Boltzmann's constant (1.38e-23 J/K)
// T is temperature in kelvin (or C + 273.15)
// q is charge on an electron (1.6e-19 C)
// Therefore temperature readout is
// temp_centigrates = (ADCcode*503.975)/4096 - 273.15;

// Raw data for DDR3 SDRAM
assign device_temp = devicetemperature;

logic [6:0] xadc_temp_addr = 7'h00; // temperature register

wire enable, ready;
wire [15:0] xadc_data;
wire eos, adcalarm;

xadc_wiz_0 XADC (
	.daddr_in(xadc_temp_addr),	// NOTE: Ignoring p/n inputs A0(7'h10) A1(7'h11) A8(7'h18) A9(7'h19)
	.dclk_in(aclk),
	.den_in(enable), 
	.di_in(16'd0), 
	.dwe_in(1'b0), 
	.busy_out(),
	.vp_in(1'b1),
	.vn_in(1'b0),
	.do_out(xadc_data), 
	.eoc_out(enable),
	.channel_out(),
	.drdy_out(ready),
	.eos_out(eos),				// End of sequence temp/a0/a1/a8/a9
	.alarm_out(adcalarm) );

always @(posedge aclk) begin
	if (~aresetn) begin
		//
	end else begin
		if (ready) begin
			tmp0 <= xadc_data[15:4];			// 1'b0 - TEMPERATURE
		end
		devicetemperature <= tmp0;
	end
end

// --------------------------------------------------
// Off-chip ADC for on-board analog input
// --------------------------------------------------

// MCP3008T-I/SL
// Initiate comm by bringing cs high then low
// din high + first clock is start bit
// sgl/diff bit follows (single ended or differential mode select)
// next 3 bits select d0/d1/d2 input configuration (select CH0 to CH7 when sgl/diff is high)
// sample period ends on falling edge of 5th clock following start bit
// 
// CLK  xxx...............................................
// DIN  xxx11DDDxxxxxxxxxxxxxxxxxxxx11DDDxxxxxxxxxxxxxxxxx
// DOUT zzzzzzzzz?9876543210zzzzzzzzzzzzzz?9876543210zzzzz
// NOTE: min clock speed is about 1.2ms which is pretty slow

logic adccmdbit = 1'b0;
logic adccs = 1'b1;
logic adcclk = 1'b0;

assign adcconn.adclk = clk10;
assign adcconn.addin = adccmdbit;
assign adcconn.adcs = adccs;
assign adcbit = adcconn.addout;

logic [9:0] ch0dat = 10'd0;
logic [9:0] ch0stash = 10'd0;

logic [4:0] adcstate = 5'd0;
always @(negedge clk10) begin	// TODO: 125ns is 'high' and datasheet doesn't show a 'low', so... 10ns?
	if (~aresetn) begin
		//
	end else begin
		adccs <= 1'b0;
		adccmdbit <= 1'b0;
		case (adcstate)
			5'b00000: begin
				adccmdbit <= 1'b1; // start
				adcstate <= 5'b00001;
			end
			5'b00001: begin
				adccmdbit <= 1'b1; // single
				adcstate <= 5'b00010;
			end
			5'b00010: begin
				// D2
				adccmdbit <= 1'b0;
				adcstate <= 5'b00011;
			end
			5'b00011: begin
				// D1
				adccmdbit <= 1'b0;
				adcstate <= 5'b00100;
			end
			5'b00100: begin
				// D0 : CH0 selected
				adccmdbit <= 1'b0;
				adcstate <= 5'b00101;
			end
			5'b00101: begin
				// gap
				adcstate <= 5'b00110;
			end
			5'b00110: begin
				// null bit arrives
				adcstate <= 5'b00111;
			end
			5'b00111: begin
				// TODO: On each pass, sample the next channel and stash
				ch0dat[9] <= adcbit;
				adcstate <= 5'b01000;
			end
			5'b01000: begin
				ch0dat[8] <= adcbit;
				adcstate <= 5'b01001;
			end
			5'b01001: begin
				ch0dat[7] <= adcbit;
				adcstate <= 5'b01010;
			end
			5'b01010: begin
				ch0dat[6] <= adcbit;
				adcstate <= 5'b01011;
			end
			5'b01011: begin
				ch0dat[5] <= adcbit;
				adcstate <= 5'b01100;
			end
			5'b01100: begin
				ch0dat[4] <= adcbit;
				adcstate <= 5'b01101;
			end
			5'b01101: begin
				ch0dat[3] <= adcbit;
				adcstate <= 5'b01110;
			end
			5'b01110: begin
				ch0dat[2] <= adcbit;
				adcstate <= 5'b01111;
			end
			5'b01111: begin
				ch0dat[1] <= adcbit;
				adcstate <= 5'b10000;
			end
			5'b10000: begin
				ch0dat[0] <= adcbit;
				adcstate <= 5'b10001;
			end
			5'b10001: begin
				// gap
				adccs <= 1'b1;	// end
				adcstate <= 5'b00000;
			end
		endcase 
	end
end

always @(posedge aclk) begin
	ch0stash <= ch0dat;
end

// --------------------------------------------------
// AXI interface for ADC
// --------------------------------------------------

// ADC is read-only
assign s_axi.awready = 1'b1;
assign s_axi.wready = 1'b1;
assign s_axi.bvalid = 1'b1;
assign s_axi.bresp = 2'b00;

logic [1:0] raddrstate = 2'b00;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.rlast <= 1'b1;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		unique case (raddrstate)
			2'b00: begin
				if (s_axi.arvalid) begin
					s_axi.arready <= 1'b1;
					unique case(s_axi.araddr[5:0])
						6'b000000: raddrstate <= 2'b01;	// 0x00: Temperature
						default: raddrstate <= 2'b10;	// 0x04..0x20: CH0..CH7
					endcase
				end
			end
			2'b01: begin
				if (s_axi.rready) begin
					s_axi.rdata <= {116'd0, devicetemperature};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
			2'b10: begin
				if (s_axi.rready) begin
					// TODO: Map each address to an ADC channel
					s_axi.rdata <= {118'd0, ch0stash};
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
