ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

TARGET = agnes.elf

default: $(TARGET)

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_GCC_OPTS ?= -mcmodel=medany --param "l1-cache-line-size=64" --param "l1-cache-size=16" -finline-limit=8192 -Wall -Warray-bounds=0 -Wstringop-overflow=0 -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_GCC_OPTS ?= -mcmodel=medany --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -finline-limit=8192 -Wall -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
endif

incs  += -I$(src_dir) -I$(corelib_dir) $(addprefix -I$(src_dir)/, $(folders))
libs += $(wildcard $(corelib_dir)/*.S) $(wildcard $(corelib_dir)/*.c)
objs  := 

$(TARGET):
	$(RISCV_GCC) $(incs) -o $(TARGET) $(wildcard $(src_dir)/*.c) $(libs) $(RISCV_GCC_OPTS)

clean:
ifeq ($(UNAME), Windows)
	del $(TARGET)
else
	rm -rf $(TARGET)
endif
