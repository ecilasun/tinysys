# Setup

XLEN ?= 32

default: all

# Directories

src_dir = .
corelib_dir = ../SDK
thirdparty_libs = ../3rdparty

# Executable folders

# TODO: Make sure FETCH always builds in non-compressed mode

folders = \
	BOOT \
	FETCH

# Rules

#override CFLAGS += -Xlinker --defsym=__stack_size=0x800

RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump
RISCVTOOL ?= ..\build\release\riscvtool.exe
# No filesystem when defined
# FILESYSTEM ?= -DDISABLE_FILESYSTEM
FILESYSTEM ?= -DENABLE_FILESYSTEM -DLFS_NO_MALLOC -DLFS_NO_DEBUG
# The COE file will be using 16 byte (128bit) output width to match the ROM layout
# (cache controller reads x4 16byte chunks for each line)
ROMWORDSIZE ?= 16
# 512 bit cache lines makes 64 bytes per line
# 256 cache lines makes total cache size 16 Kbytes
RISCV_GCC_OPTS ?= -mcmodel=medany -march=rv32imc_zicsr_zifencei -mabi=ilp32 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -Os -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug
#RISCV_GCC_OPTS ?= -mcmodel=medany -march=rv32imc -mabi=ilp32 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -Wno-format -Os -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug

incs  += -I$(src_dir) -I$(corelib_dir) -I$(thirdparty_libs)/ $(addprefix -I$(src_dir)/, $(folders))
objs  :=
libs  := $(wildcard $(thirdparty_libs)/**/*.c)

define compile_ROM
$(1).elf: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/*)
	$$(RISCV_GCC) -DBUILDING_ROM $$(FILESYSTEM) $$(incs) $$(RISCV_GCC_OPTS) -o $$@ $(wildcard $(src_dir)/$(1)/*.c) $(wildcard $(corelib_dir)/*.c) $$(libs) -Wl,-T$(1)/rom.lds

$(1).coe: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCVTOOL) $(src_dir)/$(1).elf -makerom $$(ROMWORDSIZE) 0 >> $(src_dir)/$(1).coe

$(1).txt: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCV_OBJDUMP) $(src_dir)/$(1).elf -x -D -S >>  $(src_dir)/$(1).txt
endef

$(foreach folder,$(folders),$(eval $(call compile_ROM,$(folder))))

# Build

folders_riscv_bin  = $(addsuffix .elf,  $(folders))
folders_riscv_coe = $(addsuffix .coe, $(folders))
folders_riscv_disasm = $(addsuffix .txt, $(folders))

executables: $(folders_riscv_bin)
romimages: $(folders_riscv_coe)
disassembly: $(folders_riscv_disasm)

junk += $(folders_riscv_bin) $(folders_riscv_coe) $(folders_riscv_disasm)

# Default

all: executables romimages disassembly

# Clean

clean:
	del $(objs) $(junk)

# rm -rf $(objs) $(junk)