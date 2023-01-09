onbreak {quit -force}
onerror {quit -force}

asim +access +r +m+ddr3clkgen -L xpm -L xil_defaultlib -L unisims_ver -L unimacro_ver -L secureip -O5 xil_defaultlib.ddr3clkgen xil_defaultlib.glbl

set NumericStdNoWarnings 1
set StdArithNoWarnings 1

do {wave.do}

view wave
view structure

do {ddr3clkgen.udo}

run -all

endsim

quit -force
