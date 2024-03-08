#pragma once

#include "clock.h"
#include "memman.h"

enum CPUState{
	ECPUReset,
	ECPUFetch,
	ECPUDecode,
	ECPUExecute,
	ECPURetire
};

#define OP_OP		    0b0110011
#define OP_OP_IMM 	    0b0010011
#define OP_AUIPC	    0b0010111
#define OP_LUI		    0b0110111
#define OP_STORE	    0b0100011
#define OP_LOAD		    0b0000011
#define OP_JAL		    0b1101111
#define OP_JALR		    0b1100111
#define OP_BRANCH	    0b1100011

struct SDecodedInstruction
{
	uint32_t m_opcode;
	uint32_t m_f3;
	uint32_t m_rs1;
	uint32_t m_rs2;
	uint32_t m_rd;
	uint32_t m_immed;
};

class CRV32
{
public:
	CRV32();
	~CRV32();

	// Left hand side
	uint32_t m_PC_next;
	uint32_t m_GPR_next[32];
	uint32_t m_instruction_next;
	SDecodedInstruction m_decoded_next;

	// Right hand side
	uint32_t m_PC;
	uint32_t m_GPR[32];
	uint32_t m_instruction;
	SDecodedInstruction m_decoded;

	// Internal state
	CPUState m_state = ECPUReset;
	CPUState m_state_next;
	CMemMan *m_mem = nullptr;

	void SetMem(CMemMan *mem);
	void DecodeInstruction(uint32_t instr, SDecodedInstruction& dec);
	void Tick(CClock& cpuclock);
};
