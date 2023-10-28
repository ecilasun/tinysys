# Setup

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

OBJDUMP ?= riscv64-unknown-elf-
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_GCC_OPTS ?= -mcmodel=medany -Ofast -flto -Wall -march=rv32im -mstrict-align -mabi=ilp32 -mbranch-cost=8 --param l1-cache-line-size=64 --param l1-cache-size=16 -lgcc -lm

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) 
objs  := agnes.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

agnes.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.c) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: agnes.elf agnes.txt

%.txt: %.elf
	$(OBJDUMP)objdump -x -D -S $< >> $@

# Clean

clean:
	del $(objs) $(junk) *.txt
