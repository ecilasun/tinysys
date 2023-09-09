# Setup

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
#RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++14 -Wall -Ofast -march=rv32imc_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++17 -Wall -Ofast -march=rv32imc -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) 
objs  := olcNES.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

olcNES.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.cpp) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: olcNES.elf

# Clean

clean:
	del $(objs) $(junk)
