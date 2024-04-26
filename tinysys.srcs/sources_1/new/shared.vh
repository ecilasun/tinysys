// ------------------------------------------
// Data
// ------------------------------------------

`define DATACMD_PASSTHROUGH	3'b000
`define DATACMD_STORE		3'b001
`define DATACMD_LOAD		3'b010
`define DATACMD_CACHEOP		3'b011
`define DATACMD_MATHOP		3'b100
`define DATACMD_UNUSED0		3'b101
`define DATACMD_UNUSED1		3'b110
`define DATACMD_UNUSED2		3'b111

// ------------------------------------------
// Uncompressed instruction groups
// ------------------------------------------

`define OPCODE_OP		    7'b0110011
`define OPCODE_OP_IMM 	    7'b0010011
`define OPCODE_AUIPC	    7'b0010111
`define OPCODE_LUI		    7'b0110111
`define OPCODE_STORE	    7'b0100011
`define OPCODE_LOAD		    7'b0000011
`define OPCODE_JAL		    7'b1101111
`define OPCODE_JALR		    7'b1100111
`define OPCODE_BRANCH	    7'b1100011
`define OPCODE_FENCE	    7'b0001111
`define OPCODE_SYSTEM	    7'b1110011

`define OPCODE_FLOAT_OP     7'b1010011
`define OPCODE_FLOAT_MADD   7'b1000011
`define OPCODE_FLOAT_MSUB   7'b1000111
`define OPCODE_FLOAT_NMSUB  7'b1001011
`define OPCODE_FLOAT_NMADD  7'b1001111

// ------------------------------------------
// Uncompressed instruction subgroups
// ------------------------------------------

`define INSTR_LUI			7'b0110111
`define INSTR_AUIPC			7'b0010111
`define INSTR_JAL			7'b1101111
`define INSTR_JALR			7'b1100111
`define INSTR_BEQ			7'b1100011
`define INSTR_BNE			7'b1100011
`define INSTR_BLT			7'b1100011
`define INSTR_BGE			7'b1100011
`define INSTR_BLTU			7'b1100011
`define INSTR_BGEU			7'b1100011
`define INSTR_LB			7'b0000011
`define INSTR_LH			7'b0000011
`define INSTR_LW			7'b0000011
`define INSTR_LBU			7'b0000011
`define INSTR_LHU			7'b0000011
`define INSTR_SB			7'b0100011
`define INSTR_SH			7'b0100011
`define INSTR_SW			7'b0100011
`define INSTR_ADDI			7'b0010011
`define INSTR_SLTI			7'b0010011
`define INSTR_SLTIU			7'b0010011
`define INSTR_XORI			7'b0010011
`define INSTR_ORI			7'b0010011
`define INSTR_ANDI			7'b0010011
`define INSTR_SLLI			7'b0010011
`define INSTR_SRLI			7'b0010011
`define INSTR_SRAI			7'b0010011
`define INSTR_ADD			7'b0110011
`define INSTR_SUB			7'b0110011
`define INSTR_SLL			7'b0110011
`define INSTR_SLT			7'b0110011
`define INSTR_SLTU			7'b0110011
`define INSTR_XOR			7'b0110011
`define INSTR_SRL			7'b0110011
`define INSTR_SRA			7'b0110011
`define INSTR_OR			7'b0110011
`define INSTR_AND			7'b0110011
`define INSTR_FENCE			7'b0001111
`define INSTR_ECALL			7'b1110011
`define INSTR_EBREAK		7'b1110011

// ------------------------------------------
// Compressed instruction subgroups
// ------------------------------------------

// Quadrant 0
`define INSTR_CADDI4SPN		5'b00000 // RES, nzuimm=0 +
//`define INSTR_CFLD		5'b00100 // 32/64
//`define INSTR_CLQ			5'b00100 // 128
`define INSTR_CLW			5'b01000 // 32? +
//`define INSTR_CFLW		5'b01100 // 32
//`define INSTR_CLD			5'b01100 // 64/128 
//`define INSTR_CFSD		5'b10100 // 32/64
//`define INSTR_CSQ			5'b10100 // 128
`define INSTR_CSW			5'b11000 // 32? +
//`define INSTR_CFSW		5'b11100 // 32 
//`define INSTR_CSD			5'b11100 // 64/128

// Quadrant 1									        [12] [11:10] [6:5]
`define INSTR_CNOP			5'b00001 // HINT, nzimm!=0 +
`define INSTR_CADDI			5'b00001 // HINT, nzimm=0 +
`define INSTR_CJAL			5'b00101 // 32 +
//`define INSTR_CADDIW		5'b00101 // 64/128
`define INSTR_CLI			5'b01001
`define INSTR_CADDI16SP		5'b01101
`define INSTR_CLUI			5'b01101
`define INSTR_CSRLI			5'b10001 //                      00      
`define INSTR_CSRAI			5'b10001 //                      01      
`define INSTR_CANDI			5'b10001 //                      10      
`define INSTR_CSUB			5'b10001 //                  0   11      00
`define INSTR_CXOR			5'b10001 //                  0   11      01
`define INSTR_COR			5'b10001 //                  0   11      10
`define INSTR_CAND			5'b10001 //                  0   11      11
//`define INSTR_CSUBW		5'b10001 //                  1   11      00
//`define INSTR_CADDW		5'b10001 //                  1   11      01
`define INSTR_CJ			5'b10101 //
`define INSTR_CBEQZ			5'b11001 //
`define INSTR_CBNEZ			5'b11101 //

// Quadrant 2
`define INSTR_CSLLI			5'b00010
//`define INSTR_CFLDSP		5'b00110
//`define INSTR_CLQSP		5'b00110
`define INSTR_CLWSP			5'b01010
//`define INSTR_CFLWSP		5'b01110
//`define INSTR_CLDSP		5'b01110
`define INSTR_CJR			5'b10010
`define INSTR_CMV			5'b10010
`define INSTR_CEBREAK		5'b10010
`define INSTR_CJALR			5'b10010
`define INSTR_CADD			5'b10010
//`define INSTR_CFSDSP		5'b10110
//`define INSTR_CSQSP		5'b10110
`define INSTR_CSWSP			5'b11010
//`define INSTR_CFSWSP		5'b11110
//`define INSTR_CSDSP		5'b11110

// ------------------------------------------
// Sub-instructions
// ------------------------------------------

// Flow control
`define F3_BEQ		3'b000
`define F3_BNE		3'b001
`define F3_BLT		3'b100
`define F3_BGE		3'b101
`define F3_BLTU		3'b110
`define F3_BGEU		3'b111

// Logic ops
`define F3_ADD		3'b000
`define F3_SLL		3'b001
`define F3_SLT		3'b010
`define F3_SLTU		3'b011
`define F3_XOR		3'b100
`define F3_SR		3'b101
`define F3_OR		3'b110
`define F3_AND		3'b111

// Integer math
`define F3_MUL		3'b000
`define F3_MULH		3'b001
`define F3_MULHSU	3'b010
`define F3_MULHU	3'b011
`define F3_DIV		3'b100
`define F3_DIVU		3'b101
`define F3_REM		3'b110
`define F3_REMU		3'b111

// Load type
`define F3_LB		3'b000
`define F3_LH		3'b001
`define F3_LW		3'b010
`define F3_LBU		3'b100
`define F3_LHU		3'b101

// Store type
`define F3_SB		3'b000
`define F3_SH		3'b001
`define F3_SW		3'b010

// Float compare type
`define F3_FEQ		3'b010
`define F3_FLT		3'b001
`define F3_FLE		3'b000

// Floating point math
`define F7_FADD        7'b0000000
`define F7_FSUB        7'b0000100
`define F7_FMUL        7'b0001000
`define F7_FDIV        7'b0001100
`define F7_FSQRT       7'b0101100

// Sign injection
`define F7_FSGNJ       7'b0010000
`define F7_FSGNJN      7'b0010000
`define F7_FSGNJX      7'b0010000

// Comparison / classification
`define F7_FMIN        7'b0010100
`define F7_FMAX        7'b0010100
`define F7_FEQ         7'b1010000
`define F7_FLT         7'b1010000
`define F7_FLE         7'b1010000
`define F7_FCLASS      7'b1110000

// Conversion from/to integer
`define F7_FCVTWS      7'b1100000
`define F7_FCVTWUS     7'b1100000
`define F7_FCVTSW      7'b1101000
`define F7_FCVTSWU     7'b1101000
`define F7_FCVTSWU4SAT 7'b1100001 // NON-STANDARD EXTENSION

// Move from/to integer registers
`define F7_FMVXW       7'b1110000
`define F7_FMVWX       7'b1111000

`define F12_CDISCARD   12'hFC2
`define F12_CFLUSH     12'hFC0
`define F12_MRET       12'h302
`define F12_WFI        12'h105
`define F12_EBREAK     12'h001
`define F12_ECALL      12'h000

// ------------------------------------------
// Instruction decoder one-hot states
// ------------------------------------------

`define O_H_OP				15
`define O_H_OP_IMM			14
`define O_H_AUIPC			13
`define O_H_LUI				12
`define O_H_STORE			11
`define O_H_LOAD			10
`define O_H_JAL				9
`define O_H_JALR			8
`define O_H_BRANCH			7

`define O_H_FENCE			6
`define O_H_SYSTEM			5

`define O_H_FLOAT_OP		4
`define O_H_FLOAT_MADD		3
`define O_H_FLOAT_MSUB		2
`define O_H_FLOAT_NMSUB		1
`define O_H_FLOAT_NMADD		0

// ------------------------------------------
// ALU ops
// ------------------------------------------

// Integer base
`define ALU_NONE		4'd0
`define ALU_ADD 		4'd1
`define ALU_SUB			4'd2
`define ALU_SLL			4'd3
`define ALU_SLT			4'd4
`define ALU_SLTU		4'd5
`define ALU_XOR			4'd6
`define ALU_SRL			4'd7
`define ALU_SRA			4'd8
`define ALU_OR			4'd9
`define ALU_AND			4'd10
// Mul/Div extension
`define ALU_MUL			4'd11
`define ALU_DIV			4'd12
`define ALU_REM			4'd13

// Branch
`define BLU_NONE		3'd0
`define BLU_EQ			3'd1
`define BLU_NE			3'd2
`define BLU_L			3'd3
`define BLU_GE			3'd4
`define BLU_LU			3'd5
`define BLU_GEU			3'd6

// ------------------------------------------
// CSR file
// ------------------------------------------

`define CSR_MSTATUS		12'h300
`define CSR_MISA		12'h301
`define CSR_MIE		    12'h304
`define CSR_MTVEC		12'h305
`define CSR_MSCRATCH    12'h340
`define CSR_MEPC		12'h341
`define CSR_MCAUSE		12'h342
`define CSR_MTVAL		12'h343
`define CSR_MIP			12'h344
`define CSR_TIMECMPLO	12'h800
`define CSR_TIMECMPHI	12'h801

`define CSR_REGISTERSHADOW	12'h8A0 // These are used to store shadow copies of registers from ISRs

`define CSR_CYCLELO		12'hC00	// 12'hB00
`define CSR_TIMELO		12'hC01
`define CSR_RETILO		12'hC02 // 12'hB02 
`define CSR_CYCLEHI		12'hC80	// 12'hB80
`define CSR_TIMEHI		12'hC81
`define CSR_RETIHI		12'hC82 // 12'hB82
`define CSR_MARCHID		12'hF12
`define CSR_MIMPID		12'hF13
`define CSR_MHARTID		12'hF14 // immutable

`define CSR_CPURESET	12'hFEE // Set lowest bit to 1'b1 to trigger reset, 1'b0 to stop reset
`define CSR_WATERMARK	12'hFF0 // Machine boot state (watermark) register, survives soft reboot intact
`define CSR_HWSTATE		12'hFFF // immutable, custom hardware state bits

// `define CSR_MCONFIGPTR  12'hF15
// `define CSR_DSCRATCH0   12'h7B2
// `define CSR_DSCRATCH1   12'h7B3
// `define CSR_ISAEXMISA   12'h301
// `define CSR_DCONTROL    12'h7B0
// `define CSR_DPC         12'h7B1
// `define CSR_DSCRATCH0   12'h7B2
// `define CSR_DSCRATCH1   12'h7B3
