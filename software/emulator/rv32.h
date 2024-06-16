#pragma once

#include "bitutil.h"
#include <queue>

enum FetchState{
	EFetchRead,
	EFetchWaitForBranch,
};

#define OP_OP			0b0110011
#define OP_OP_IMM		0b0010011
#define OP_AUIPC		0b0010111
#define OP_LUI			0b0110111
#define OP_STORE		0b0100011
#define OP_LOAD			0b0000011
#define OP_JAL			0b1101111
#define OP_JALR			0b1100111
#define OP_BRANCH		0b1100011
#define OP_FENCE		0b0001111
#define OP_SYSTEM		0b1110011

#define OPCODE_FLOAT_OP		0b1010011
#define OPCODE_FLOAT_MADD	0b1000011
#define OPCODE_FLOAT_MSUB	0b1000111
#define OPCODE_FLOAT_NMSUB	0b1001011
#define OPCODE_FLOAT_NMADD	0b1001111

// Integer base
#define ALU_NONE		0
#define ALU_ADD 		1
#define ALU_SUB			2
#define ALU_SLL			3
#define ALU_SLT			4
#define ALU_SLTU		5
#define ALU_XOR			6
#define ALU_SRL			7
#define ALU_SRA			8
#define ALU_OR			9
#define ALU_AND			10
// Mul/Div extension
#define ALU_MUL			11
#define ALU_DIV			12
#define ALU_REM			13

#define BLU_NONE		0
#define BLU_EQ			1
#define BLU_NE			2
#define BLU_L			3
#define BLU_GE			4
#define BLU_LU			5
#define BLU_GEU			6

#define F12_CDISCARD   0xFC2
#define F12_CFLUSH     0xFC0
#define F12_MRET       0x302
#define F12_WFI        0x105
#define F12_EBREAK     0x001
#define F12_ECALL      0x000

#define CSR0BASE 0x800A0000
#define CSR1BASE 0x800B0000

struct SDecodedInstruction
{
	uint32_t m_pc;
	uint32_t m_opcode;
	uint32_t m_aluop;
	uint32_t m_bluop;
	uint32_t m_f3;
	uint32_t m_f12;
	uint32_t m_rs1;
	uint32_t m_rs2;
	uint32_t m_rd;
	uint32_t m_immed;
	uint32_t m_selimm;
	uint32_t m_csroffset;
	uint32_t m_rval1;
	uint32_t m_rval2;
//#if defined(DEBUG)
	// internal / debugging related
	uint32_t m_opindex;
//#endif
};

class CBus;

class CRV32
{
public:
	CRV32();
	~CRV32();

	// Right hand side
	uint32_t m_PC;
	uint32_t m_branchtarget;
	uint32_t m_fetchstate;
	uint32_t m_branchresolved;
	uint32_t m_GPR[32];

	// Internal counters
	uint64_t m_cyclecounter = 0;
	uint64_t m_wallclock = 0;
	uint64_t m_retired = 0;

	// HART0 by default
	uint32_t m_hartid = 0;
	uint32_t m_sie = 0;

	uint32_t m_exceptionmode = 0;
	uint32_t m_lasttrap = 0;

	//uint32_t m_postmret = 0;
	//uint32_t m_posteoi = 0;

	uint32_t m_resetvector = 0x0FFE0000;
	bool m_pendingCPUReset{ false };

	std::queue<SDecodedInstruction> m_instructionfifo;

	void Reset();
	bool Tick(CBus& bus, uint32_t irq);
	bool FetchDecode(CBus& bus, uint32_t irq);
	bool Execute(CBus& bus);

private:
	void DecodeInstruction(const uint32_t pc, const uint32_t instr, SDecodedInstruction& dec);
	void InjectISRHeaderFooter();
	uint32_t ALU(SDecodedInstruction &instr);
	uint32_t BLU(SDecodedInstruction& instr);
};
