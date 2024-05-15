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
libxm = ../../3rdparty/libxm
micromod = ../../3rdparty/micromod

# Rules

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
LIBXM_OPTS ?= -D XM_DEBUG=0 -D XM_DEFENSIVE=0 -D XM_DEMO_MODE=0 -D XM_LIBXMIZE_DELTA_SAMPLES=0 -D XM_LINEAR_INTERPOLATION=0 -fpermissive 
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Warray-bounds=0 -Wstringop-overflow=0 -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
LIBXM_OPTS ?= -D XM_DEBUG=0 -D XM_DEFENSIVE=0 -D XM_DEMO_MODE=0 -D XM_LIBXMIZE_DELTA_SAMPLES=0 -D XM_LINEAR_INTERPOLATION=0 -fpermissive 
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
endif

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(libxm)/ -I$(micromod)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) $(wildcard $(libxm)/*.c) $(wildcard $(micromod)/*.c)
objs  := play.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

play.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.cpp) $(libs) $(LIBXM_OPTS) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: play.elf

# Clean

clean:
ifeq ($(UNAME), Windows)
	del $(objs) $(junk)
else
	rm -rf $(objs) $(junk)
endif
