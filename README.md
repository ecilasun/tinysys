# tinysys

# General execution flow

## Fetch/Decode/IRQ:
This unit reads an instruction at PC, decodes it and outputs it (together with its PC) to instruction output FIFO (IFIFO). If it's an interrupt or some other special instruction (for instance I$ flush) then it is handled entirely within the fetch unit. This unit is also responsible for inserting pre/post interrupt code from an internal ROM at interrupt or exception time. If a branch instruction is encountered, fetch unit will drop into an idle state and wait for the execute unit to resolve the target branch address.

## Execute/Load/Store:
The execute unit will fetch an instruction from the FIFO if available, read register values, execute (ALU/BLU/CSR/SYS) and decide on new branch target if there's a branch involved. On a branch, decision the fetch unit is notified so it can resume instruction fetches. Any load or store operations will be overlapped with fetch and execution as much as possible.

