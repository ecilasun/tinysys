# Setup

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
#RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++14 -Wall -Ofast -march=rv32imc_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
RISCV_GCC_OPTS ?= -mcmodel=medany -O3 -flto -Wall -march=rv32imc -mabi=ilp32 --param l1-cache-line-size=64 --param l1-cache-size=16 -ffunction-sections -fdata-sections -Wl,-gc-sections -lgcc -lm

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) 
objs  := agnes.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

agnes.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.c) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: agnes.elf

# Clean

clean:
	del $(objs) $(junk)
