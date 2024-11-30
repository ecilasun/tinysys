`timescale 1ns / 1ps

module xadcmodule(
	input wire aclk,
	input wire aresetn,
	output wire [11:0] device_temp);

// --------------------------------------------------
// Reset delay line
// --------------------------------------------------

wire delayedresetn;
delayreset delayresetinst(
	.aclk(aclk),
	.inputresetn(aresetn),
	.delayedresetn(delayedresetn) );

// --------------------------------------------------
// On-chip ADC for temperature control and DDR3 SDRAM
// --------------------------------------------------

// TEMP is a MSB aligned 12 bit values
(* async_reg = "true" *) logic [11:0] tmp0;
(* async_reg = "true" *) logic [11:0] devicetemperature;

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
	if (~delayedresetn) begin
		tmp0 <= 12'd0;
		devicetemperature <= 12'd0;
	end else begin
		if (ready) begin
			tmp0 <= xadc_data[15:4];			// 1'b0 - TEMPERATURE
			devicetemperature <= tmp0;
		end
	end
end

endmodule
