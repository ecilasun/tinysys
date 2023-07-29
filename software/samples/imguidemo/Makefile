# Setup

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK
thirdparty_libs = ../../3rdparty

# Rules

RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
#RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++14 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -Ofast -march=rv32imc_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++14 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -Ofast -march=rv32imc -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(thirdparty_libs)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) $(wildcard $(thirdparty_libs)/**/*.c) $(wildcard $(thirdparty_libs)/**/*.cpp)
objs  := imguidemo.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

imguidemo.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.cpp) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: imguidemo.elf

# Clean

clean:
	del $(objs) $(junk)
# rm -rf $(objs) $(junk)