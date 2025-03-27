ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

TARGET = hellodebugger.elf

.DEFAULT: $(TARGET)

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -g -O0 -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -Wl,-gc-sections -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -g -O0 -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -Wl,-gc-sections -lgcc -lm
endif

RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump

incs  += -I$(src_dir) -I$(corelib_dir) $(addprefix -I$(src_dir)/, $(folders))
libs += $(wildcard $(corelib_dir)/*.S) $(wildcard $(corelib_dir)/*.c)
objs  := 

$(TARGET):
	$(RISCV_GCC) $(incs) -o $(TARGET) $(wildcard $(src_dir)/*.cpp) $(libs) $(RISCV_GCC_OPTS)

dump: $(TARGET)
	$(RISCV_OBJDUMP) $(TARGET) -x -D -S >> $(TARGET).txt

.PHONY: clean
clean:
ifeq ($(UNAME), Windows)
	del *.elf *.txt
else
	rm -rf *.elf *.txt
endif
