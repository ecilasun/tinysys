#pragma once

#include <deque>
#include <map>
#include <vector>

#include "bitutil.h"

enum FetchState{
	EFetchInit,
	EFetchRead,
	EFetchWaitForBranch,
	EFetchWFI
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

#define OP_FLOAT_OP		0b1010011
#define OP_FLOAT_MADD	0b1000011
#define OP_FLOAT_MSUB	0b1000111
#define OP_FLOAT_NMSUB	0b1001011
#define OP_FLOAT_NMADD	0b1001111

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
	uint32_t m_f7;
	uint32_t m_f12;
	uint32_t m_rs1;
	uint32_t m_rs2;
	uint32_t m_rs3;
	uint32_t m_rd;
	uint32_t m_immed;
	uint32_t m_selimm;
	uint32_t m_csroffset;
	uint32_t m_rval1;
	uint32_t m_rval2;
	uint32_t m_rval3;
	uint32_t m_opindex;
};

class CBus;
class CCSRMem;

enum ERV32ExceptionMode
{
	EXC_NONE = 0x00000000,
	EXC_SWI = 0x00000001,
	EXC_HWI = 0x00000002,
	EXC_TMI = 0x00000003,
	EXC_EBREAK = 0x00000004,
	EXC_ECALL = 0x00000005,
	EXC_SWI_END = 0x80000001,
	EXC_HWI_END = 0x80000002,
	EXC_TMI_END = 0x80000003,
	EXC_EBREAK_END = 0x80000004,
	EXC_ECALL_END = 0x80000005
};

class InstructionCache
{
public:
	InstructionCache() {}
	~InstructionCache() {}

	void Reset();
	void Fetch(CBus *bus, uint32_t pc, uint32_t& instr);
	void Discard();

	// 256 entries, 16 words each
	uint32_t m_cache[256*16] = {};
	uint32_t m_cachelinetags[256] = {};

#if defined(CPU_STATS)
	uint32_t m_hits {0};
	uint32_t m_misses {0};
#endif
};

class DataCache
{
public:
	DataCache() {}
	~DataCache() {}

	void Reset();
	void WriteLine(CBus* bus, uint32_t line);
	void LoadLine(CBus* bus, uint32_t tag, uint32_t line);
	void Read(CBus* bus, uint32_t address, uint32_t& data);
	void Write(CBus* bus, uint32_t address, uint32_t data, uint32_t wstrobe);
	void Flush(CBus* bus);
	void Discard();

	// 512 entries, 16 words each
	uint32_t m_cache[512*16] = {};
	uint32_t m_cachelinetags[512] = {};
	uint32_t m_cachelinewb[512] = {};

#if defined(CPU_STATS)
	uint32_t m_readhits {0};
	uint32_t m_readmisses {0};
	uint32_t m_writehits {0};
	uint32_t m_writemisses {0};
#endif
};

struct SDecodedBlock
{
	uint32_t m_PC;
	std::vector<SDecodedInstruction> m_code;
};

class CRV32
{
public:
	explicit CRV32(uint32_t hartid, uint32_t resetvector) : m_hartid(hartid), m_resetvector(resetvector) {}
	~CRV32() {}

	// Right hand side
	uint32_t m_PC{ 0 };
	uint32_t m_branchtarget{ 0 };
	uint32_t m_fetchstate{ 0 };
	uint32_t m_branchresolved{ 0 };
	uint32_t m_wasmret{ 0 };
	uint32_t m_GPR[32] = {};

	// Internal counters
	uint64_t m_retired{ 0 };
	uint32_t m_wficount{ 0 };

	// HART0 by default
	uint32_t m_hartid{ 0 };

	ERV32ExceptionMode m_exceptionmode{ EXC_NONE };
	ERV32ExceptionMode m_lasttrap{ EXC_NONE };

	uint32_t m_resetvector{ 0x0 };

	std::vector<SDecodedInstruction> m_instructions;
	std::map<uint32_t, SDecodedBlock*> m_decodedBlocks;

	void Reset();
	bool Tick(CBus* bus);
	bool FetchDecode(CBus* bus);
	bool Execute(CBus* bus);

	InstructionCache m_icache;
	DataCache m_dcache;

#if defined(CPU_STATS)
	uint32_t m_btaken{ 0 };
	uint32_t m_bntaken{ 0 };
	uint32_t m_ucbtaken{ 0 };
#endif

private:
	void DecodeInstruction(const uint32_t pc, const uint32_t instr, SDecodedInstruction& dec);
	void InjectISRHeader(std::vector<SDecodedInstruction> *code);
	void InjectISRFooter(std::vector<SDecodedInstruction>* code);
	void GatherInstructions(CCSRMem* csr, CBus* bus);
	uint32_t ALU(SDecodedInstruction &instr);
	uint32_t BLU(SDecodedInstruction& instr);
};
