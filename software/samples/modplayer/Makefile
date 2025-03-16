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
libxmploaders = ../../3rdparty/libxmp/loaders
libxmploadersextra = ../../3rdparty/libxmp/loaders/prowizard
libxmpdepackers = ../../3rdparty/libxmp/depackers
libxmp = ../../3rdparty/libxmp

# Rules

RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 -Wno-narrowing --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Warray-bounds=0 -Wstringop-overflow=0 -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 -Wno-narrowing --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -lgcc -lm
endif

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(libxmp)/ -I$(libxmploaders)/ -I$(libxmploadersextra)/ -I$(libxmpdepackers)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) $(wildcard $(libxmp)/*.c) $(wildcard $(libxmploaders)/*.c) $(wildcard $(libxmploadersextra)/*.c) $(wildcard $(libxmpdepackers)/*.c)
objs  := play.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

play.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.cpp) -D LIBXMP_NO_DEPACKERS $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: play.elf

dump: $(objs)
	$(RISCV_OBJDUMP) $(objs) -x -D -S >> $(objs).txt

# Clean
.PHONY: clean
clean:
ifeq ($(UNAME), Windows)
	del $(objs) $(objs).txt $(junk)
else
	rm -rf $(objs) $(objs).txt $(junk)
endif
