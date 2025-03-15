# Setup

ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else ifeq ($(MSYSTEM), DARWIN)
		UNAME := DARWIN
	else
		UNAME := WINDOWS
	endif
else
	UNAME := $(shell uname)
endif

default: all

XLEN ?= 32

# Directories

src_dir = .
corelib_dir = ../../SDK
filesystemlib_dir = ../../3rdparty/fat32

# Executable folders

# TODO: Make sure FETCH always builds in non-compressed mode

folders = \
	rom

# Rules

#override CFLAGS += -Xlinker --defsym=__stack_size=0x800

RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump
ifeq ($(UNAME), WINDOWS)
RISCVTOOL ?= ..\..\riscvtool.exe
RISCV_PREFIX ?= riscv32-unknown-elf-
else
RISCVTOOL ?= ../../riscvtool
RISCV_PREFIX ?= riscv64-unknown-elf-
endif
# No filesystem when defined
# FILESYSTEM ?= -DDISABLE_FILESYSTEM
FILESYSTEM ?= -DENABLE_FILESYSTEM -DLFS_NO_MALLOC -DLFS_NO_DEBUG
# The MEM file will be using 16 byte (128bit) output width to match the ROM layout
# (cache controller reads x4 16byte chunks for each line)
ROMWORDSIZE ?= 16
# 512 bit cache lines makes 64 bytes per line
# 256 cache lines makes total cache size 16 Kbytes
ifeq ($(UNAME), WINDOWS)
RISCV_GCC_OPTS ?= -mcmodel=medany -std=gnu99 -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Warray-bounds=0 -Wstringop-overflow=0 -Wno-format -Os -static -ffreestanding -nostartfiles -flto -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug -Wl,--strip-all 
else
RISCV_GCC_OPTS ?= -mcmodel=medany -std=gnu99 -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Wno-format -Os -static -ffreestanding -nostartfiles -flto -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug -Wl,--strip-all 
endif

incs  += -I$(src_dir) -I$(corelib_dir) -I$(filesystemlib_dir)/ $(addprefix -I$(src_dir)/, $(folders))
objs  :=
libs  := $(wildcard $(filesystemlib_dir)/*.c)

define compile_ROM
$(1).elf: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/*)
	$$(RISCV_GCC) -DBUILDING_ROM $$(FILESYSTEM) $$(incs) $$(RISCV_GCC_OPTS) -o $$@ $(wildcard $(src_dir)/$(1)/*.S) $(wildcard $(src_dir)/$(1)/*.c) $(wildcard $(corelib_dir)/*.c) $$(libs) -Wl,-T$(1)/rom.lds
	$$(RISCV_OBJDUMP) $(src_dir)/$(1).elf -x -D -S >> $(src_dir)/$(1).txt
	$$(RISCVTOOL) -makemem $(src_dir)/$(1).elf $$(ROMWORDSIZE) $(src_dir)/$(1).mem
	$$(RISCVTOOL) -makebin $(src_dir)/$(1).elf 4 $(src_dir)/$(1).bin
endef

$(foreach folder,$(folders),$(eval $(call compile_ROM,$(folder))))

# Build

folders_riscv_elf  = $(addsuffix .elf,  $(folders))
folders_riscv_mem = $(addsuffix .mem, $(folders))
folders_riscv_bin = $(addsuffix .bin, $(folders))
folders_riscv_disasm = $(addsuffix .txt, $(folders))

executables: $(folders_riscv_elf)
romimages: $(folders_riscv_mem)
overlayimage: $(folders_riscv_bin)
disassembly: $(folders_riscv_disasm)

junk += $(folders_riscv_elf) $(folders_riscv_mem) $(folders_riscv_bin) $(folders_riscv_disasm)

# Default

all: executables romimages overlayimage disassembly

# Clean

clean:
ifeq ($(UNAME), WINDOWS)
	del $(objs) $(junk)
else
	rm -rf $(objs) $(junk)
endif
