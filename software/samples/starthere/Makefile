# Setup

ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK
# my_3rdparty_libs = ../../3rdparty/libfoldernamehere

# Rules

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Warray-bounds=0 -Wstringop-overflow=0 -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
endif

# add your third party include folder here before src_dir as -I$(my_3rdparty_libs)/
incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(src_dir)/.
# add your third party libs / sources here at the end as $(wildcard $(my_3rdparty_libs)/*.c) or *.cpp
libs += $(wildcard $(corelib_dir)/*.c)
objs  := starthere.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

starthere.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.cpp) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: starthere.elf

# Clean

clean:
ifeq ($(UNAME), Windows)
	del $(objs) $(junk)
else
	rm -rf $(objs) $(junk)
endif
