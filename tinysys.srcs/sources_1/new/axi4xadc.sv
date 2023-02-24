`timescale 1ns / 1ps

module axi4xadc(
	input wire aclk,
	input wire aresetn,
	axi4if.slave s_axi,
	output wire [11:0] device_temp,
	adcwires.def adcconn);

logic [1:0] waddrstate = 2'b00;
logic [1:0] writestate = 2'b00;
logic [1:0] raddrstate = 2'b00;

// AUX and TEMP are MSB aligned 12 bit values
(* async_reg = "true" *) logic [11:0] acq0 = 12'd0;
(* async_reg = "true" *) logic [11:0] acq1 = 12'd0;
(* async_reg = "true" *) logic [11:0] tmp0 = 12'd0;
(* async_reg = "true" *) logic [11:0] devicetemperature = 12'd0;

// Accodring to Xilinx documentation the 12 MSB correspond to temperature
// voltage = 10 * (kT/q) * ln(10);
// where k is Boltzmann's constant (1.38e-23 J/K)
// T is temperature in kelvin (or C + 273.15)
// q is charge on an electron (1.6e-19 C)
// Therefore temperature readout is
// temp_centigrates = (ADCcode*503.975)/4096 - 273.15;
assign device_temp = devicetemperature;

logic [6:0] xadc_addr = 7'h10; // h00:TEMP h10:A0 h11:A1 ... h18:A8 h19:A9
logic [6:0] xadc_temp_addr = 7'h00; // temperature
logic evenodd = 1'b0;

wire enable, ready;
wire [15:0] xadc_data;
wire eos, adcalarm;

xadc_wiz_0 XADC (
	.daddr_in(evenodd ? xadc_addr : xadc_temp_addr),
	.dclk_in(aclk),
	.den_in(enable), 
	.di_in(16'd0), 
	.dwe_in(1'b0), 
	.busy_out(),
	.vp_in(1'b1),
	.vn_in(1'b0),
	.vauxp0(adcconn.xa_p[1]), // A0 - 7'h10
	.vauxn0(adcconn.xa_n[1]),
	.vauxp1(adcconn.xa_p[0]), // A1 - 7'h11
	.vauxn1(adcconn.xa_n[0]),
	.vauxp8(adcconn.xa_p[2]), // A8 - 7'h18
	.vauxn8(adcconn.xa_n[2]),
	.vauxp9(adcconn.xa_p[3]), // A9 - 7'h19
	.vauxn9(adcconn.xa_n[3]),                           
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
			// Swap addresses once current value is ready
			case (evenodd)
				default:	tmp0 <= xadc_data[15:4]; // 1'b0 - TEMPERATURE
				1'b1:		acq0 <= xadc_data[15:4]; // AUX0
			endcase
			// Flip for next time
			evenodd <= ~evenodd;
		end
		// DC
		acq1 <= acq0;
		devicetemperature <= tmp0;
	end
end

// XADC is read-only
assign s_axi.awready = 1'b1;
assign s_axi.wready = 1'b1;
assign s_axi.bvalid = 1'b1;
assign s_axi.bresp = 2'b00;

always @(posedge aclk) begin
	if (~aresetn) begin
		s_axi.rlast <= 1'b1;
		s_axi.arready <= 1'b0;
		s_axi.rvalid <= 1'b0;
		s_axi.rresp <= 2'b00;
	end else begin
		s_axi.rvalid <= 1'b0;
		s_axi.arready <= 1'b0;
		case (raddrstate)
			default: begin // 2'b00
				if (s_axi.arvalid) begin
					case (s_axi.araddr[3:0])
						default: raddrstate <= 2'b01; // 4'h0
						4'h4: raddrstate <= 2'b10;
					endcase
					s_axi.arready <= 1'b1;
				end
			end
			2'b01: begin
				if (s_axi.rready) begin
					s_axi.rdata <= {116'd0, acq1}; // AUX0
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
			2'b10: begin
				if (s_axi.rready) begin
					s_axi.rdata <= {116'd0, devicetemperature}; // TEMP
					s_axi.rvalid <= 1'b1;
					raddrstate <= 2'b00;
				end
			end
		endcase
	end
end

endmodule
