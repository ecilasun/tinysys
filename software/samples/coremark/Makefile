# Setup

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

OBJDUMP ?= riscv64-unknown-elf-
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_GCC_OPTS ?= -mcmodel=medany -Ofast -fno-exceptions -Wall -march=rv32im -mabi=ilp32 --param l1-cache-line-size=64 --param l1-cache-size=16 -lgcc -lm

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) 
objs  := coremark.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

coremark.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -DPERFORMANCE_RUN=1 -DITERATIONS=25000 -o $@ $(wildcard $(src_dir)/*.c) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: coremark.elf coremark.txt

%.txt: %.elf
	$(OBJDUMP)objdump -x -D -S $< >> $@

# Clean

clean:
	del $(objs) $(junk) *.txt
