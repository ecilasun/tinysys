# tinysys

# General flow

Fetch:
This unit reads an instruction at PC, decodes it and outputs it (together with its PC) to instruction output FIFO (IFIFO). If it's an interrupt or some other special instruction (for instance I$ flush) then it is handled entirely within the fetch unit. This unit is also responsible for inserting pre/post interrupt code from an internal ROM at interrupt or exception time. If a branch instruction is encountered, fetch unit will drop into an idle state and wait for the execute unit to resolve the target branch address.

The execute unit will fetch an instruction from the FIFO if available, read register values, execute (ALU/BLU/CSR/SYS) and decide on new branch target if there's a branch involved. On a branch, decision the fetch unit is notified so it can resume instruction fetches.

# Addendum: APU

TBD

# Instruction encoding space

|inst[6:5] | 000    | 001      | 010      | 011      | 100    | 101      | 110             | 111(> 32b)| inst[6:5]|
|----------|--------|----------|----------|----------|--------|----------|-----------------|-----------|----------|
|00        | LOAD   | LOAD-FP  | custom-0 | MISC-MEM | OP-IMM | AUIPC    | OP-IMM-32       | 48b       |          |
|01        | STORE  | STORE-FP | custom-1 | AMO      | OP     | LUI      | OP-32           | 64b       |          |
|10        | MADD   | MSUB     | NMSUB    | NMADD    | OP-FP  | reserved | custom-2/rv128  | 48b       |          |
|11        | BRANCH | JALR     | reserved | JAL      | SYSTEM | reserved | custom-3/rv128  | ≥ 80      |          |

TBD: encode custom instructions into custom-0 space (i.e. 7'b??_010_11 for a total of 4 new core instructions)
