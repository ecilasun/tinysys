#include <stdio.h>
#include <math.h>
#include "rv32.h"
#include "bus.h"

const char *opnames[] = {
	"illegal",
	"op",
	"opimm",
	"auipc",
	"lui",
	"store",
	"load",
	"jal",
	"jalr",
	"branch",
	"fence",
	"system",
	"floatop",
	"floatmadd",
	"floatmsub",
	"floatnmsub",
	"floatnmadd" };

const char *alunames[] = {
	"", // none
	"add",
	"sub",
	"sll",
	"slt",
	"sltu",
	"xor",
	"srl",
	"sra",
	"or",
	"and",
	"mul",
	"div",
	"rem" };

const char *regnames[] = {
	"zero",
	"ra", "sp",
	"gp", "tp",
	"t0", "t1", "t2",
	"s0", "s1",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
	"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
	"t3", "t4", "t5", "t6" };

// Base CSR address per core
uint32_t csrBaseTable[] = { 0x80090000, 0x800A0000 };


// This is an exact copy of the ISR ROM contents of the real hardware
// Please see the fetch ROM source code for hardware implementation

uint32_t ISR_ROM[] = {
	0xFD079073,0x00000797,0x34179073,0x08000793,0x3047B073,0x3007A073,0x800007B7,0x00778793,0x34279073,0x00800793,0x3007B073,0xFD0027F3, // 0-11 TMI start
	0xFD079073,0x08000793,0x3007B073,0x3047A073,0x00800793,0x3007A073,0xFD0027F3, // 12-18 TMI end
	0xFD079073,0x00000797,0x34179073,0x000017B7,0x80078793,0x3047B073,0x0047D793,0x3007A073,0x800007B7,0x00B78793,0x34279073,0x00800793,0x3007B073,0xFD0027F3, // 19-32 HWI start
	0xFD079073,0x08000793,0x3007B073,0x00479793,0x3047A073,0x00800793,0x3007A073,0xFD0027F3, // 33-40 HWI end
	0xFD079073,0x00000797,0x34179073,0x00800793,0x3047B073,0x00479793,0x3007A073,0x00B00793,0x34279073,0x00800793,0x3007B073,0xFD0027F3, // 41-52 ecall start
	0xFD079073,0x08000793,0x3007B073,0x0047D793,0x3047A073,0x00800793,0x3007A073,0xFD0027F3, // 53-60 ecall end
	0xFD079073,0x00000797,0x34179073,0x00800793,0x3047B073,0x00479793,0x3007A073,0x00300793,0x34279073,0x00800793,0x3007B073,0xFD0027F3, // 61-72 ebreak start
	0xFD079073,0x08000793,0x3007B073,0x0047D793,0x3047A073,0x00800793,0x3007A073,0xFD0027F3, // 73-80 ebreak end
	0xFD079073,0x00000797,0x34179073,0x00800793,0x3047B073,0x00479793,0x3007A073,0x00200793,0x34279073,0x00800793,0x3007B073,0xFD0027F3, // 81-92 SWI start
	0xFD079073,0x08000793,0x3007B073,0x0047D793,0x3047A073,0x00800793,0x3007A073,0xFD0027F3  // 93-100 SWI end
};

static const uint32_t s_quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

// Floating point classification
#define RISCV_NEG_INF			0
#define RISCV_NEG_NORMAL		1
#define RISCV_NEG_SUBNORMAL		2
#define RISCV_NEG_ZERO			3
#define RISCV_POS_ZERO			4
#define RISCV_POS_SUBNORMAL		5
#define RISCV_POS_NORMAL		6
#define RISCV_POS_INF			7
#define RISCV_SNAN				8
#define RISCV_QNAN				9

void InstructionCache::Reset()
{
	for (uint32_t i=0; i<256; i++)
	{
		// TODO: AVX2 perhaps?
		uint32_t base = (i << 4);
		m_cache[base + 0x0] = 0x00000000;
		m_cache[base + 0x1] = 0x00000000;
		m_cache[base + 0x2] = 0x00000000;
		m_cache[base + 0x3] = 0x00000000;
		m_cache[base + 0x4] = 0x00000000;
		m_cache[base + 0x5] = 0x00000000;
		m_cache[base + 0x6] = 0x00000000;
		m_cache[base + 0x7] = 0x00000000;
		m_cache[base + 0x8] = 0x00000000;
		m_cache[base + 0x9] = 0x00000000;
		m_cache[base + 0xA] = 0x00000000;
		m_cache[base + 0xB] = 0x00000000;
		m_cache[base + 0xC] = 0x00000000;
		m_cache[base + 0xD] = 0x00000000;
		m_cache[base + 0xE] = 0x00000000;
		m_cache[base + 0xF] = 0x00000000;
	}

	for (uint32_t i = 0; i < 256; i++)
		m_cachelinetags[i] = 0x00000000;
}

void InstructionCache::Fetch(CBus *bus, uint32_t pc, uint32_t& instr)
{
	uint32_t tag = SelectBitRange(pc, 27,14);	// 14 bits
	uint32_t line = SelectBitRange(pc, 13, 6);	// 8 bits
	uint32_t offset = SelectBitRange(pc, 5, 2);	// 4 bits

	if ((tag | 0x4000) == m_cachelinetags[line])
	{
		instr = m_cache[(line << 4) + offset];
#if defined(CPU_STATS)
		m_hits++;
#endif
	}
	else
	{
		// Base cache address
		uint32_t addr = (tag << 14) | (line << 6);
		for (uint32_t i = 0; i < 16; i++)
			bus->Read(addr+(i<<2), m_cache[(line << 4) + i]);
		instr = m_cache[(line << 4) + offset];
		// Mark valid
		m_cachelinetags[line] = tag | 0x4000;
#if defined(CPU_STATS)
		m_misses++;
#endif
	}
}

void InstructionCache::Discard()
{
	for (uint32_t i = 0; i < 256; i++)
		m_cachelinetags[i] = 0x00000000;
}

void DataCache::Reset()
{
	for (uint32_t i=0; i<512; i++)
	{
		// TODO: AVX2 perhaps?
		uint32_t base = (i << 4);
		m_cache[base + 0x0] = 0x00000000;
		m_cache[base + 0x1] = 0x00000000;
		m_cache[base + 0x2] = 0x00000000;
		m_cache[base + 0x3] = 0x00000000;
		m_cache[base + 0x4] = 0x00000000;
		m_cache[base + 0x5] = 0x00000000;
		m_cache[base + 0x6] = 0x00000000;
		m_cache[base + 0x7] = 0x00000000;
		m_cache[base + 0x8] = 0x00000000;
		m_cache[base + 0x9] = 0x00000000;
		m_cache[base + 0xA] = 0x00000000;
		m_cache[base + 0xB] = 0x00000000;
		m_cache[base + 0xC] = 0x00000000;
		m_cache[base + 0xD] = 0x00000000;
		m_cache[base + 0xE] = 0x00000000;
		m_cache[base + 0xF] = 0x00000000;
	}

	for (uint32_t i = 0; i < 512; i++)
	{
		m_cachelinewb[i] = 0;
		m_cachelinetags[i] = 0x00000000;
	}
}

void DataCache::WriteLine(CBus* bus, uint32_t line)
{
	if (m_cachelinewb[line])
	{
		m_cachelinewb[line] = 0;
		uint32_t tag = m_cachelinetags[line] & 0x3FFF;
		uint32_t wbaddr = (tag << 15) | (line << 6);
		bus->m_mem->Write512bits(wbaddr, &m_cache[line << 4]);
	}
}

void DataCache::LoadLine(CBus* bus, uint32_t tag, uint32_t line)
{
	uint32_t addr = (tag << 15) | (line << 6);
	bus->m_mem->Read512bits(addr, &m_cache[line << 4]);
	m_cachelinetags[line] = tag | 0x4000;
}

uint32_t DataCache::Read(CBus* bus, uint32_t address, uint32_t& data)
{
	uint32_t tag = SelectBitRange(address, 27, 15);		// 13 bits
	uint32_t line = SelectBitRange(address, 14, 6);		// 9 bits
	uint32_t offset = SelectBitRange(address, 5, 2);	// 4 bits

	uint32_t retVal = 0;
	if ((tag | 0x4000) != m_cachelinetags[line])
	{
		WriteLine(bus, line);
		LoadLine(bus, tag, line);
#if defined(CPU_STATS)
		m_readmisses++;
#endif
		retVal = 80;
	}
	else
	{
#if defined(CPU_STATS)
		m_readhits++;
#endif
		retVal = 4;
	}

	// Read from cache
	data = m_cache[(line << 4) + offset];
	return retVal;
}

uint32_t DataCache::Write(CBus* bus, uint32_t address, uint32_t data, uint32_t wstrobe)
{
	uint32_t tag = SelectBitRange(address, 27, 15);		// 13 bits
	uint32_t line = SelectBitRange(address, 14, 6);		// 9 bits
	uint32_t offset = SelectBitRange(address, 5, 2);	// 4 bits

	uint32_t retVal = 0;
	if ((tag | 0x4000) != m_cachelinetags[line])
	{
		WriteLine(bus, line);
		LoadLine(bus, tag, line);
#if defined(CPU_STATS)
		m_writemisses++;
#endif
		retVal = 80;
	}
	else
	{
#if defined(CPU_STATS)
		m_writehits++;
#endif
		retVal = 4;
	}

	// Write to cache
	uint32_t fullmask = s_quadexpand[wstrobe];
	uint32_t invfullmask = ~fullmask;
	uint32_t olddata = m_cache[(line << 4) + offset];
	m_cache[(line << 4) + offset] = (olddata&invfullmask) | (data&fullmask);

	// This line is now dirty and requres write back to memory
	m_cachelinewb[line] = 1;

	return retVal;
}

uint32_t DataCache::Flush(CBus* bus)
{
	for (uint32_t line = 0; line < 512; line++)
		WriteLine(bus, line);
	
	return 128;
}

void DataCache::Discard()
{
	for (uint32_t i = 0; i < 512; i++)
	{
		m_cachelinetags[i] = 0x00000000;
		m_cachelinewb[i] = 0;
	}
}

void CRV32::Reset()
{
	m_fetchstate = EFetchInit;
	SecondaryReset();
}

void CRV32::SecondaryReset()
{
	m_icache.Reset();
	m_dcache.Reset();

	m_wficount = 0;

	m_PC = m_resetvector;
	m_branchresolved = 0;
	m_wasmret = 0;
	m_branchtarget = 0;
	m_retired = 0;

	m_exceptionmode = EXC_NONE;
	m_lasttrap = EXC_NONE;

	for (uint32_t i=0; i<32; i++)
		m_GPR[i] = 0x00000000;

	m_instructions.clear();
	m_decodedBlocks.clear();
	m_fetchstate = EFetchRead;
}

uint32_t CRV32::ALU(SDecodedInstruction& instr)
{
	uint32_t aluout = 0;

	uint32_t selected = instr.m_selimm ? instr.m_immed : instr.m_rval2;

	switch(instr.m_aluop)
	{
		case ALU_OR:
			aluout = instr.m_rval1 | selected;
		break;
		case ALU_SUB:
			aluout = instr.m_rval1 + (~selected + 1); // val1-val2
		break;
		case ALU_SLL:
			aluout = instr.m_rval1 << (selected&0x1F);
		break;
		case ALU_SLT:
			aluout = ((int32_t)instr.m_rval1 < (int32_t)selected) ? 1 : 0;
		break;
		case ALU_SLTU:
			aluout = (instr.m_rval1 < selected) ? 1 : 0;
		break;
		case ALU_XOR:
			aluout = instr.m_rval1 ^ selected;
		break;
		case ALU_SRL:
			aluout = instr.m_rval1 >> (selected&0x1F);
		break;
		case ALU_SRA:
			aluout = (int32_t)instr.m_rval1 >> (selected&0x1F);
		break;
		case ALU_ADD:
			aluout = instr.m_rval1 + selected;
		break;
		case ALU_AND:
			aluout = instr.m_rval1 & selected;
		break;
		case ALU_MUL:
		{
			uint64_t a, b;
			int64_t sign1ext = (int32_t)instr.m_rval1;
			int64_t sign2ext = (int32_t)instr.m_rval2;
			switch (instr.m_f3)
			{
				case 0b000:
				case 0b001: {
					a = sign1ext;
					b = sign2ext;
				} break; // mul/mulh
				case 0b010: {
					a = sign1ext;
					b = instr.m_rval2;
				} break; // mulhsu
				case 0b011: {
					a = instr.m_rval1;
					b = instr.m_rval2;
				} break; // mulhu
				default: {
					a = 0;
					b = 0;
				} break;
			}
			uint64_t result = a * b;
			aluout = instr.m_f3 == 0b000 ? (uint32_t)(result&0xFFFFFFFF) : (uint32_t)((result&0xFFFFFFFF00000000)>>32);
		}
		break;
		case ALU_DIV:
		case ALU_REM:
		{
			int64_t sign1ext = (int32_t)instr.m_rval1;
			int64_t sign2ext = (int32_t)instr.m_rval2;
			switch (instr.m_f3)
			{
				case 0b100: {
					aluout = instr.m_rval2 == 0 ? 0xFFFFFFFF : uint32_t(sign1ext / sign2ext);
				} break; //  div
				case 0b101: {
					aluout = instr.m_rval2 == 0 ? 0xFFFFFFFF : instr.m_rval1 / instr.m_rval2;
				} break; // divu
				case 0b110: {
					aluout = instr.m_rval2 == 0 ? instr.m_rval1 : uint32_t(sign1ext % sign2ext);
				} break; // rem
				case 0b111: {
					aluout = instr.m_rval2 == 0 ? instr.m_rval1 : instr.m_rval1 % instr.m_rval2;
				} break; // remu
				default: {
					aluout = 0;
				} break;
			}
		}
		break;
	}

	return aluout;
}

uint32_t CRV32::BLU(SDecodedInstruction& instr)
{
	uint32_t bluout = 0;

	uint32_t eq = instr.m_rval1 == instr.m_rval2 ? 1 : 0;
	uint32_t sless = (int32_t)instr.m_rval1 < (int32_t)instr.m_rval2 ? 1 : 0;
	uint32_t less = instr.m_rval1 < instr.m_rval2 ? 1 : 0;
	switch (instr.m_bluop)
	{
		case BLU_EQ:
			bluout = eq;
		break;
		case BLU_NE:
			bluout = !eq;
		break;
		case BLU_L:
			bluout = sless;
		break;
		case BLU_GE:
			bluout = !sless;
		break;
		case BLU_LU:
			bluout = less;
		break;
		case BLU_GEU:
			bluout = !less;
		break;
	}

	return bluout;
}

void CRV32::DecodeInstruction(uint32_t pc, uint32_t instr, SDecodedInstruction& dec)
{
	dec.m_pc = pc;
	dec.m_rawInstruction = instr;
	dec.m_opcode = SelectBitRange(instr, 6, 0);
	dec.m_f3 = SelectBitRange(instr, 14, 12);
	dec.m_f7 = SelectBitRange(instr, 31, 25);
	dec.m_rs1 = SelectBitRange(instr, 19, 15);
	dec.m_rs2 = SelectBitRange(instr, 24, 20);
	dec.m_rs3 = SelectBitRange(instr, 31, 27);
	dec.m_rd = SelectBitRange(instr, 11, 7);
	dec.m_f12 = SelectBitRange(instr, 31, 20);
	dec.m_csroffset = (SelectBitRange(instr, 31, 25) << 5) | dec.m_rs2;

	switch (dec.m_opcode)
	{
		case OP_OP:				dec.m_opindex = 1; break;
		case OP_OP_IMM:			dec.m_opindex = 2; break;
		case OP_AUIPC:			dec.m_opindex = 3; break;
		case OP_LUI:			dec.m_opindex = 4; break;
		case OP_STORE:			dec.m_opindex = 5; break;
		case OP_LOAD:			dec.m_opindex = 6; break;
		case OP_JAL:			dec.m_opindex = 7; break;
		case OP_JALR:			dec.m_opindex = 8; break;
		case OP_BRANCH:			dec.m_opindex = 9; break;
		case OP_FENCE:			dec.m_opindex = 10; break;
		case OP_SYSTEM:			dec.m_opindex = 11; break;
		case OP_FLOAT_OP:		dec.m_opindex = 12; break;
		case OP_FLOAT_MADD:		dec.m_opindex = 13; break;
		case OP_FLOAT_MSUB:		dec.m_opindex = 14; break;
		case OP_FLOAT_NMSUB:	dec.m_opindex = 15; break;
		case OP_FLOAT_NMADD:	dec.m_opindex = 16; break;
		default:				dec.m_opindex = 0; break;		// Invalid opcode
	}

	switch (dec.m_opcode)
	{
		case OP_LUI:
		case OP_AUIPC:
		{
			// U-imm
			dec.m_immed = SelectBitRange(instr, 31, 12) << 12;
		}
		break;

		case OP_STORE:
		{
			// S-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 20;	// 32-11 == 21
			uint32_t upper = SelectBitRange(instr, 30, 25);		// 6
			uint32_t lower = SelectBitRange(instr, 11, 7);		// +5 == 11
			dec.m_immed = sign | (upper<<5) | lower;
		}
		break;

		case OP_JAL:
		{
			// J-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 11;	// 32-20 == 12
			uint32_t upper = SelectBitRange(instr, 19, 12);		// 8
			uint32_t middle = SelectBitRange(instr, 20, 20);	// +1
			uint32_t lower = SelectBitRange(instr, 30, 21);		// +10
			uint32_t zero = 0;									// +1 == 20
			dec.m_immed = sign | (upper<<12) | (middle<<11) | (lower<<1) | zero;
		}
		break;

		case OP_BRANCH:
		{
			// B-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 19;	// 32-12 == 20
			uint32_t upper = SelectBitRange(instr, 7, 7);		// 1
			uint32_t middle = SelectBitRange(instr, 30, 25);	// +6
			uint32_t lower = SelectBitRange(instr, 11, 8);		// +4
			uint32_t zero = 0x0;								// +1 == 12
			dec.m_immed = sign | (upper<<11) | (middle<<5) | (lower<<1) | zero;
		}
		break;

		case OP_SYSTEM:
		{
			uint32_t lower = SelectBitRange(instr, 19, 15);	// 5
			dec.m_immed = lower;
		}
		break;

		default:
		/*case OP_OP_IMM:
		case OP_LOAD:
		case OP_JALR:*/
		{
			// Includes OP_FENCE, OP_FLOAT_OP, OP_FLOAT_MADD, OP_FLOAT_MSUB, OP_FLOAT_NMSUB, OP_FLOAT_NMADD which don't need this

			// I-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 20;	// 32-11 == 21
			uint32_t lower = SelectBitRange(instr, 30, 20);		// 11
			dec.m_immed = sign | lower;
		}
		break;
	}

	dec.m_aluop = ALU_NONE;
	dec.m_bluop = BLU_NONE;

	uint32_t mathopsel = SelectBitRange(instr, 30, 30);
	uint32_t muldiv = SelectBitRange(instr, 25, 25);
	if (dec.m_opcode == OP_OP)
	{
		switch (dec.m_f3)
		{
			case 0b000:
				dec.m_aluop = muldiv ? ALU_MUL : (mathopsel ? ALU_SUB : ALU_ADD);
			break;
			case 0b001:
				dec.m_aluop = muldiv ? ALU_MUL : ALU_SLL;
			break;
			case 0b011:
				dec.m_aluop = muldiv ? ALU_MUL : ALU_SLTU;
			break;
			case 0b010:
				dec.m_aluop = muldiv ? ALU_MUL : ALU_SLT;
			break;
			case 0b110:
				dec.m_aluop = muldiv ? ALU_REM : ALU_OR;
			break;
			case 0b111:
				dec.m_aluop = muldiv ? ALU_REM : ALU_AND;
			break;
			case 0b101:
				dec.m_aluop = muldiv ? ALU_DIV : (mathopsel ? ALU_SRA : ALU_SRL);
			break;
			case 0b100:
				dec.m_aluop = muldiv ? ALU_DIV : ALU_XOR;
			break;
			default:
				dec.m_aluop = ALU_NONE;
			break;
		}
	}

	if (dec.m_opcode == OP_OP_IMM)
	{
		switch (dec.m_f3)
		{
			case 0b000:	dec.m_aluop = ALU_ADD; break;
			case 0b001:	dec.m_aluop = ALU_SLL; break;
			case 0b011:	dec.m_aluop = ALU_SLTU; break;
			case 0b010:	dec.m_aluop = ALU_SLT; break;
			case 0b110:	dec.m_aluop = ALU_OR; break;
			case 0b111:	dec.m_aluop = ALU_AND; break;
			case 0b101:	dec.m_aluop = mathopsel ? ALU_SRA : ALU_SRL; break;
			case 0b100:	dec.m_aluop = ALU_XOR; break;
			default:	dec.m_aluop = ALU_NONE; break;
		}
	}

	if (dec.m_opcode == OP_BRANCH)
	{
		switch (dec.m_f3)
		{
			case 0b000:	dec.m_bluop = BLU_EQ; break;
			case 0b001:	dec.m_bluop = BLU_NE; break;
			case 0b011:	dec.m_bluop = BLU_NONE; break;
			case 0b010:	dec.m_bluop = BLU_NONE; break;
			case 0b110:	dec.m_bluop = BLU_LU; break;
			case 0b111:	dec.m_bluop = BLU_GEU; break;
			case 0b101:	dec.m_bluop = BLU_GE; break;
			case 0b100:	dec.m_bluop = BLU_L; break;
			default:	dec.m_bluop = BLU_NONE; break;
		}
	}

	dec.m_selimm = (dec.m_opcode==OP_JALR) || (dec.m_opcode==OP_OP_IMM) || (dec.m_opcode==OP_LOAD) || (dec.m_opcode==OP_STORE);
}

void CRV32::InjectISRHeader(std::vector<SDecodedInstruction> *code)
{
	switch (m_exceptionmode)
	{
		// TMI start
		case EXC_TMI: {
			for (uint32_t i = 0; i <= 11; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// HWI start
		case EXC_HWI: {
			for (uint32_t i = 19; i <= 32; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// ecall start
		case EXC_ECALL: {
			for (uint32_t i = 41; i <= 52; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// ebreak start
		case EXC_EBREAK: {
			for (uint32_t i = 61; i <= 72; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// SWI start
		case EXC_SWI: {
			for (uint32_t i = 81; i <= 92; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// Unknown
		default:
		{
			// Nothing to inject
			fprintf(stderr, "unknown header encountered\n");
		}
		break;
	}
}

void CRV32::InjectISRFooter(std::vector<SDecodedInstruction>* code)
{
	switch (m_exceptionmode)
	{
		// TMI end
		case EXC_TMI_END: {
			for (uint32_t i = 12; i <= 18; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// HWI end
		case EXC_HWI_END: {
			for (uint32_t i = 33; i <= 40; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// ecall end
		case EXC_ECALL_END: {
			for (uint32_t i = 53; i <= 60; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// ebreak end
		case EXC_EBREAK_END: {
			for (uint32_t i = 73; i <= 80; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// SWI end
		case EXC_SWI_END: {
			for (uint32_t i = 93; i <= 100; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				isrinstr.m_cantBreak = 1;
				code->push_back(isrinstr);
			}
		}
		break;

		// Unknown
		default:
		{
			// Nothing to inject
			fprintf(stderr, "unknown footer encountered\n");
		}
		break;
	}
}

void CRV32::GatherInstructions(CCSRMem* csr, CBus* bus)
{
	// IRQ handling sequence
	bool branchtomtvecforinterrupt = csr->m_irq && (m_exceptionmode == EXC_NONE);

	if (branchtomtvecforinterrupt)
	{
		if (csr->m_irq & 1)				// hardware
			m_exceptionmode = EXC_HWI;
		else if (csr->m_irq & 2)		// timer
			m_exceptionmode = EXC_TMI;

		uint32_t mtvec;
		csr->Read(CSR_MTVEC << 2, mtvec);

		// NOTE: ISR header/footer uses exception mode for address
		SDecodedBlock *blk;
		auto found = m_decodedBlocks.find(m_exceptionmode);
		if (found == m_decodedBlocks.end())
		{
			blk = new SDecodedBlock();
			blk->m_PC = mtvec;
			m_decodedBlocks[m_exceptionmode] = blk;
			InjectISRHeader(&blk->m_code);
			fprintf(stderr, "new ISR header block (HWI/TMI) %08X\n", mtvec);
		}
		else
			blk = found->second;

		// Grab pre-decoded code block
		for (auto instr : blk->m_code)
		{
			instr.m_pc = m_PC; // NOTE: ISR depends on this to be fixed to the interrupt PC
			m_instructions.push_back(instr);
		}

		m_lasttrap = m_exceptionmode;

		// Resume from ISR handler
		m_PC = mtvec;

		return;
	}

	// Regular instruction sequence
	bool doneFetching = false;
	do{
		uint32_t instruction;
		m_icache.Fetch(bus, m_PC, instruction);

		// Decode is part of fetch unit in hardware
		SDecodedInstruction decoded;
		DecodeInstruction(m_PC, instruction, decoded);
		decoded.m_cantBreak = 0; // non-ISR

		bool ismret = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_MRET;
		bool iswfi = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_WFI;
		bool isfence = decoded.m_opcode == OP_FENCE;
		bool isbranch = decoded.m_opcode == OP_BRANCH;
		bool isjalr = decoded.m_opcode == OP_JALR;
		bool isjal = decoded.m_opcode == OP_JAL;

		bool isebreak = csr->m_sie && decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_EBREAK;
		bool isecall = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_ECALL;
		bool isillegal = csr->m_sie && decoded.m_opindex == 0;
		bool branchtomtvecforinstr = (isebreak || isecall || isillegal) && (m_exceptionmode == EXC_NONE);

		// Exception handling is part of fetch unit in hardware
		if (branchtomtvecforinstr)
		{
			uint32_t mtvec;
			csr->Read(CSR_MTVEC << 2, mtvec);

			// Most of these prevent instruction execution so they have to come back to same PC
			if (isecall) // ecall
			{
				m_exceptionmode = EXC_ECALL;
				// ECALL assumes current instruction executed and will return to the next one
				m_PC += 4;
			}
			else if (isebreak) // ebreak
				m_exceptionmode = EXC_EBREAK; 
			else if (isillegal) // software - illegal instruction
				m_exceptionmode = EXC_SWI;

			// NOTE: ISR header/footer uses exception mode for address
			SDecodedBlock *blk;
			auto found = m_decodedBlocks.find(m_exceptionmode);
			if (found == m_decodedBlocks.end())
			{
				blk = new SDecodedBlock();
				blk->m_PC = mtvec;
				m_decodedBlocks[m_exceptionmode] = blk;
				InjectISRHeader(&blk->m_code);
				fprintf(stderr, "new ISR header block (ECALL/EBREAK/SWI) %08X\n", mtvec);
			}
			else
				blk = found->second;

			// Grab pre-decoded code block
			for (auto instr : blk->m_code)
			{
				instr.m_pc = m_PC; // NOTE: ISR depends on this to be fixed to the interrupt PC
				m_instructions.push_back(instr);
			}

			m_lasttrap = m_exceptionmode;

			// Resume from ISR handler
			m_PC = mtvec;
		}
		else
			m_instructions.push_back(decoded);

		// Determine next PC
		if (branchtomtvecforinstr) // Route execution to mtvec
		{
			doneFetching = true;
		}
		else if (iswfi)
		{
			m_fetchstate = EFetchWFI;
			m_PC = decoded.m_pc + 4;
			doneFetching = true;
		}
		else if (isjal) // For JAL instructions, we can calculate the target immediately without having to execute
		{
			m_PC = decoded.m_pc + decoded.m_immed;
			doneFetching = true;
		}
		else if (isebreak) // EBREAK stays at the same PC until it's replaced by another instruction or SWI is disabled
			m_PC = decoded.m_pc;
		else if (isfence || (!isbranch && !isjalr && !ismret)) // For anything that doesn't require a branch, just increment PC
			m_PC = decoded.m_pc + 4;
		else // For branches, jumps and mret, we need to wait for the branch target
		{
			m_fetchstate = EFetchWaitForBranch; // wait for branch target from exec
			doneFetching = true;
		}
	} while (!doneFetching);
}

bool CRV32::FetchDecode(CBus* bus)
{
	if (m_fetchstate == EFetchInit)
	{
		CCSRMem* csr = bus->GetCSR(m_hartid);
		SecondaryReset();
		csr->m_pendingCPUReset = false;
		return true;
	}
	else if (m_fetchstate == EFetchWFI)
	{
		m_wficount = (m_wficount + 1) % 12;
		CCSRMem* csr = bus->GetCSR(m_hartid);
		if (m_wficount == 0 || csr->m_irq)
			m_fetchstate = EFetchRead;
		return true;
	}
	else if (m_fetchstate == EFetchRead)
	{
		// Grab a block of code until we hit a branch or exception
		CCSRMem* csr = bus->GetCSR(m_hartid);
		GatherInstructions(csr, bus);
		return true;
	}
	else if (m_fetchstate == EFetchWaitForBranch)
	{
		if (m_branchresolved)
		{
			if (m_lasttrap && m_wasmret)
			{
				m_wasmret = 0;

				// Add footer (append _END to previous exception mode)
				m_exceptionmode = ERV32ExceptionMode(0x80000000 | m_lasttrap);

				// NOTE: ISR header/footer uses exception mode for address
				SDecodedBlock *blk;
				auto found = m_decodedBlocks.find(m_exceptionmode);
				if (found == m_decodedBlocks.end())
				{
					CCSRMem* csr = bus->GetCSR(m_hartid);
					uint32_t mtvec;
					csr->Read(CSR_MTVEC << 2, mtvec);
					blk = new SDecodedBlock();
					blk->m_PC = mtvec;
					m_decodedBlocks[m_exceptionmode] = blk;
					InjectISRFooter(&blk->m_code);
					fprintf(stderr, "new ISR footer block %08X\n", mtvec);
				}
				else
					blk = found->second;

				// Grab pre-decoded code block
				for (auto instr : blk->m_code)
				{
					instr.m_pc = m_PC; // NOTE: ISR depends on this to be based on current PC
					m_instructions.push_back(instr);
				}

				// We're done, can accept new interrupts now
				m_exceptionmode = EXC_NONE;
				m_lasttrap = EXC_NONE;
			}

			m_branchresolved = 0;
			m_PC = m_branchtarget;
			m_fetchstate = EFetchRead;
		}
		return true;
	}
	else
	{
		fprintf(stderr, "unknown fetch state %d\n", m_fetchstate);
		return false;
	}
}

bool CRV32::Execute(CBus* bus)
{
	CCSRMem* csr = bus->GetCSR(m_hartid);
	for (auto &instr : m_instructions)
	{
		// Is this PC in the m_breakpoints?
		auto found = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [&](const SBreakpoint& b) { return b.address == instr.m_pc && !instr.m_cantBreak; });
		if (found != m_breakpoints.end())
		{
#if defined(GDB_COMM_DEBUG)
			fprintf(stderr, "Break at 0x%08X (0x%08X)\n", instr.m_pc, instr.m_rawInstruction);
#endif
		
			// Remove instructions we have already executed up to and excluding this one

			while(m_instructions.size() > 1 && m_instructions[1].m_pc != instr.m_pc)
				m_instructions.erase(m_instructions.begin() + 1);

			found->isHit = 1;
			found->isCommunicated = 0;
			return true;
		}

		csr->SetPC(instr.m_pc);

		// Get register contents
		instr.m_rval1 = m_GPR[instr.m_rs1 & 0x1F];
		instr.m_rval2 = m_GPR[instr.m_rs2 & 0x1F];
		instr.m_rval3 = m_GPR[instr.m_rs3 & 0x1F];

		// Calculate future PC and other offsets
		uint32_t adjacentpc = instr.m_pc + 4;
		uint32_t rwaddress = instr.m_rval1 + instr.m_immed;
		uint32_t offsetpc = instr.m_pc + instr.m_immed;
		uint32_t rdin = 0;
		uint32_t rwen = 0;
		uint32_t wdata = 0;
		uint32_t wstrobe = 0;

		m_cycles += 4; // cache read + read registers + dispatch

		// Execute
		switch (instr.m_opcode)
		{
			case OP_OP:
			case OP_OP_IMM:
				rdin = ALU(instr);
				rwen = 1;
			break;

			case OP_AUIPC:
				rdin = offsetpc;
				rwen = 1;
			break;

			case OP_LUI:
				rdin = instr.m_immed;
				rwen = 1;
			break;

			case OP_JAL:
				// fetch handles this
				rdin = adjacentpc;
				rwen = 1;
#if defined(CPU_STATS)
				m_ucbtaken++;
#endif
			break;

			case OP_JALR:
				m_branchresolved = 1;
				m_branchtarget = rwaddress;
				rdin = adjacentpc;
				rwen = 1;
#if defined(CPU_STATS)
				m_ucbtaken++;
#endif
			break;

			case OP_BRANCH:
			{
				m_branchresolved = 1;
				bool branchout = BLU(instr);
#if defined(CPU_STATS)
				m_btaken += branchout ? 1 : 0;
				m_bntaken += branchout ? 0 : 1;
#endif
				m_branchtarget = branchout ? offsetpc : adjacentpc;
			}
			break;

			case OP_FENCE:
				m_icache.Discard();
				m_instructions.clear();
				m_decodedBlocks.clear();
				m_branchresolved = 1;
				m_branchtarget = instr.m_pc + 4;
			break;

			case OP_SYSTEM:
			{
				m_cycles += 6; // SYSOP + WAIT

				if (instr.m_f12 == F12_CDISCARD)
				{
					// cacheop=0b01
					// NOOP for now, D$ not implemented yet
					//fprintf(stderr, "- cdiscard\n");
					m_cycles += 50; // CACHE OP
					m_dcache.Discard();
				}
				else if (instr.m_f12 == F12_CFLUSH)
				{
					// cacheop=0b11
					m_cycles += m_dcache.Flush(bus);
				}
				else if (instr.m_f12 == F12_MRET)
				{
					m_cycles += 2; // MRET
					m_wasmret = 1;
					m_branchresolved = 1;
					csr->Read((CSR_MEPC << 2), m_branchtarget);
				}
				else if (instr.m_f12 == F12_WFI)
				{
					// This is handled by fetch unit, same as hardware
				}
				else if (instr.m_f12 == F12_EBREAK)
				{
					// This is handled by fetch unit, same as hardware
				}
				else if (instr.m_f12 == F12_ECALL)
				{
					// This is handled by fetch unit, same as hardware
				}
				else // CSROP
				{
					m_cycles += 4; // READ + MODIFY + WRITE + WAIT

					// Read previous value
					uint32_t csrprevval;
					uint32_t csraddress = (instr.m_csroffset << 2);
					csr->Read(csraddress, csrprevval);

					// Keep it in a register
					rwen = 1;
					rdin = csrprevval;
					const uint32_t csrbase = csrBaseTable[m_hartid];
					rwaddress = csrbase + csraddress;

					// Apply operation
					wstrobe = 0b1111;
					switch (instr.m_f3)
					{
						case 0b001: // csrrw
							wdata = instr.m_rval1;
							break;
						case 0b101: // csrrwi
							wdata = instr.m_immed;
							break;
						case 0b010: // csrrs / csrr (set bits using rval1 as mask)
							wdata = csrprevval | instr.m_rval1;
							break;
						case 0b110: // csrrsi
							wdata = csrprevval | instr.m_immed;
							break;
						case 0b011: // csrrc
							wdata = csrprevval & (~instr.m_rval1);
							break;
						case 0b111: // csrrci
							wdata = csrprevval & (~instr.m_immed);
							break;
						default: // unknown - keep previous value
							wdata = csrprevval;
							break;
					}
				}
			}
			break;

			case OP_STORE:
			{
				m_cycles += 1; // + bus busy wait

				uint32_t byte = SelectBitRange(instr.m_rval2, 7, 0);
				uint32_t half = SelectBitRange(instr.m_rval2, 15, 0);
				switch (instr.m_f3)
				{
					case 0b000:	wdata = (byte << 24) | (byte << 16) | (byte << 8) | byte; break;
					case 0b001:	wdata = (half << 16) | half; break;
					default:	wdata = instr.m_rval2; break;
				}
				uint32_t ah = SelectBitRange(rwaddress, 1, 1);
				uint32_t ab = SelectBitRange(rwaddress, 0, 0);
				uint32_t himask = (ah << 3) | (ah << 2) | ((1 - ah) << 1) | (1 - ah);
				uint32_t lomask = ((ab << 3) | ((1 - ab) << 2) | (ab << 1) | (1 - ab));
				switch (instr.m_f3)
				{
					case 0b000:	wstrobe = himask & lomask; break;
					case 0b001:	wstrobe = himask; break;
					default:	wstrobe = 0b1111; break;
				}
			}
			break;

			case OP_LOAD:
			{
				m_cycles += 1; // + bus busy wait

				uint32_t dataword;
				if (rwaddress & 0x80000000)
				{
					m_cycles += 2; // uncached access
					bus->Read(rwaddress, dataword);
				}
				else
				{
					// Read from cache or miss cache
					m_cycles += m_dcache.Read(bus, rwaddress, dataword);
				}

				uint32_t range1 = SelectBitRange(rwaddress, 1, 1);
				uint32_t range2 = SelectBitRange(rwaddress, 1, 0);

				uint32_t b[4];
				b[3] = SelectBitRange(dataword, 31, 24);
				b[2] = SelectBitRange(dataword, 23, 16);
				b[1] = SelectBitRange(dataword, 15, 8);
				b[0] = SelectBitRange(dataword, 7, 0);

				uint32_t h[2];
				h[1] = SelectBitRange(dataword, 31, 16);
				h[0] = SelectBitRange(dataword, 15, 0);

				int32_t sign[4];
				sign[3] = int32_t(dataword & 0x80000000);
				sign[2] = int32_t((dataword << 8) & 0x80000000);
				sign[1] = int32_t((dataword << 16) & 0x80000000);
				sign[0] = int32_t((dataword << 24) & 0x80000000);

				switch (instr.m_f3)
				{
					case 0b000: // BYTE with sign extension
						rdin = (sign[range2] >> 24) | b[range2];
					break;
					case 0b001: // HALF with sign extension
						rdin = (sign[range1*2+1] >> 16) | h[range1];
					break;
					case 0b100: // BYTE with zero extension
						rdin = b[range2];
					break;
					case 0b101: // HALF with zero extension
						rdin = h[range1];
					break;
					default: // WORD - 0b010
						rdin = dataword;
					break;
				}
				rwen = 1;
			}
			break;

			case OP_FLOAT_MADD:
			case OP_FLOAT_MSUB:
			case OP_FLOAT_NMSUB:
			case OP_FLOAT_NMADD:
			{
				m_cycles += 19; // all of them take 19 cycles

				// We use zfinx extension (floating point registers in integer registers) so we need to alias them
				float A = *(float*)&instr.m_rval1;
				float B = *(float*)&instr.m_rval2;
				float C = *(float*)&instr.m_rval3;
				float* D = (float*)&rdin;
				rwen = 1;

				if (instr.m_opcode == OP_FLOAT_MADD)
					*D = A * B + C;
				else if (instr.m_opcode == OP_FLOAT_MSUB)
					*D = A * B - C;
				else if (instr.m_opcode == OP_FLOAT_NMSUB)
					*D = -(A * B - C);
				else if (instr.m_opcode == OP_FLOAT_NMADD)
					*D = -(A * B + C);
				else
				{
					fprintf(stderr, "- unknown floatop3\n");
				}
			}
			break;

			case OP_FLOAT_OP:
			{
				float A = *(float*)&instr.m_rval1;
				float B = *(float*)&instr.m_rval2;
				float* D = (float*)&rdin;
				rwen = 1;

				switch (instr.m_f7)
				{
					case 0b0010000: // fsgnj.s / fsgnjn.s / fsgnjx.s
					{
						m_cycles += 1;
						switch (instr.m_f3)
						{
							case 0b000: rdin = (instr.m_rval2 & 0x80000000) | (instr.m_rval1 & 0x7FFFFFFF); break;
							case 0b001: rdin = ((instr.m_rval2 & 0x80000000) ^ 0x80000000) | (instr.m_rval1 & 0x7FFFFFFF); break;
							default: rdin = ((instr.m_rval2 & 0x80000000) ^ (instr.m_rval1 & 0x80000000)) | (instr.m_rval1 & 0x7FFFFFFF); break;
						}
					}
					break;
					case 0b1110000: // fclass
					{
						m_cycles += 1; // Not implemented
						switch (instr.m_f3)
						{
							case 0b000: {
								rdin = instr.m_rval1; // fmv.x.w
							}
							break;
							case 0b001: { // fclass.s
								rdin = RISCV_POS_NORMAL; // Not implementing this for now
								/*if (instr.m_rval1 == 0x00000000) rdin = POS_ZERO;
								else if (instr.m_rval1 == 0x80000000) rdin = NEG_ZERO;
								else if (instr.m_rval1 == 0x7F800000) rdin = POS_INF;
								else if (instr.m_rval1 == 0xFF800000) rdin = NEG_INF;
								else if (instr.m_rval1 >= 0x7F800001 && instr.m_rval1 <= 0x7FBFFFFF) rdin = SNAN;
								else if (instr.m_rval1 >= 0xFF800001 && instr.m_rval1 <= 0xFFBFFFFF) rdin = SNAN;
								else if (instr.m_rval1 >= 0x7FC00000 && instr.m_rval1 <= 0x7FFFFFFF) rdin = QNAN;
								else if (instr.m_rval1 >= 0xFFC00000 && instr.m_rval1 <= 0xFFFFFFFF) rdin = QNAN;
								else if (instr.m_rval1 & 0x80000000) rdin = NEG_NORMAL;
								else rdin = POS_NORMAL;*/
							}
							break;
						}
					}
					break;
					case 0b0010100: // fmin.s / fmax.s
					{
						m_cycles += 1;
						switch (instr.m_f3)
						{
							case 0b000: *D = A < B ? A : B; break;
							case 0b001: *D = A > B ? A : B; break;
						}
					}
					break;
					case 0b1010000: // feq.s / flt.s / fle.s
					{
						m_cycles += 2;
						switch (instr.m_f3)
						{
							case 0b010: rdin = A == B ? 1 : 0; break;
							case 0b001: rdin = A < B ? 1 : 0; break;
							case 0b000: rdin = A <= B ? 1 : 0; break;
						}
					}
					break;
					case 0b1100000: // fcvtws / fcvtwus
					{
						m_cycles += 8; // fcvtws
						//m_cycles += 5; // fcvtwus
						if (instr.m_rs2 == 0b00000) // Signed
							rdin = (int32_t)A;
						else // Unsigned - 5'b00000
							rdin = (uint32_t)A;
					}
					break;
					case 0b1101000: // fcvtsw / fcvtwus
					{
						m_cycles += 6;
						if (instr.m_rs2 == 0b00000) // signed
							*D = (float)(int32_t)instr.m_rval1;
						else // unsigned - NOTE: doing the abs() trick the hardware does here
							*D = (float)(instr.m_rval1&0x7FFFFFFF);
					}
					break;
					case 0b0000000: // fadd.s
					{
						m_cycles += 11;
						*D = A + B;
					}
					break;
					case 0b0000100: // fsub.s
					{
						m_cycles += 11;
						*D = A - B;
					}
					break;
					case 0b0001000: // fmul.s
					{
						m_cycles += 8;
						*D = A * B;
					}
					break;
					case 0b0001100: // fdiv.s
					{
						m_cycles += 28;
						if (instr.m_rval2 == 0)
							rdin = 0x7fc00000;
						else
							*D = A / B;
					}
					break;
					case 0b0101100: // fsqrt.s
					{
						m_cycles += 28;
						*D = sqrtf(abs(A)); // NOTE: hardware drops sign bit i.e. abs()
					}
					break;
					case 0b1100001: // fcvtswu4sat.s
					{
						m_cycles += 2;
						int sat = (int)(16.0f * A);
						sat = sat > 15 ? 15 : sat;
						sat = sat < 0 ? 0 : sat;
						rdin = sat;
					}
					break;
					default:
					{
						m_cycles += 1;
						rwen = 0;
						fprintf(stderr, "- unknown floatop2\n");
					}
					break;
				}
			}
			break;

			default:
			{
				// Illegal instruction exception should catch this
			}
			break;
		}

		if (wstrobe)
		{
			m_cycles += 1;
			//fprintf(stderr, "- W @%.8X val=%.8x mask=%.8x\n", rwaddress, wdata, wstrobe);
			if (rwaddress & 0x80000000)
				bus->Write(rwaddress, wdata, wstrobe);
			else
			{
				m_cycles += m_dcache.Write(bus, rwaddress, wdata, wstrobe);
			}
		}

		if (rwen && instr.m_rd != 0)
		{
			//fprintf(stderr, "- regw @%.8X val=%.8x\n", instr.m_rd, rdin);
			m_GPR[instr.m_rd] = rdin;
		}

		++m_retired;
	}

	m_instructions.clear();

	csr->SetRetiredInstructions(m_retired);
	return false;
}

void CRV32::AddBreakpoint(uint32_t isVolatile, uint32_t address, CBus* bus)
{
	SBreakpoint brkpt;
	brkpt.address = address;
	brkpt.isVolatile = isVolatile;
	brkpt.isHit = 0;
	brkpt.isCommunicated = 0;
	bus->Read(address, brkpt.originalInstruction);
	m_breakpoints.push_back(brkpt);

#if defined(GDB_COMM_DEBUG)
	fprintf(stderr, "Added %sbreakpoint at 0x%08X (insn:0x%08X)\n", isVolatile ? "volatile " : "", address, brkpt.originalInstruction);
#endif
}

void CRV32::RemoveBreakpoint(uint32_t address, CBus* bus)
{
	// Erase the breakpoint
	auto found = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [&](const SBreakpoint& b) { return b.address == address; });
	if (found != m_breakpoints.end())
	{
#if defined(GDB_COMM_DEBUG)
		fprintf(stderr, "Removing %sbreakpoint at %08X\n", found->isVolatile ? "volatile " : "", address);
#endif
		bus->Write(address, found->originalInstruction, 0b1111);
		m_breakpoints.erase(found);
	}
#if defined(GDB_COMM_DEBUG)
	else
		fprintf(stderr, "No breakpoint found at %08X\n", address);
#endif
}

void CRV32::RemoveAllBreakpoints(CBus* bus)
{
	for (auto& brkpt : m_breakpoints)
	{
		bus->Write(brkpt.address, brkpt.originalInstruction, 0b1111);
	}

	m_breakpoints.clear();

#if defined(GDB_COMM_DEBUG)
	fprintf(stderr, "Removed all breakpoints\n");
#endif
}

void CRV32::Continue(CBus* bus)
{
	// Erase the breakpoint
	/*auto found = std::find_if(m_breakpoints.begin(), m_breakpoints.end(), [&](const SBreakpoint& b) { return b.address == m_currentBreakAddress; });
	if (found != m_breakpoints.end())
	{
		bus->Write(m_currentBreakAddress, found->originalInstruction, 0b1111);
		m_breakpoints.erase(found);
		fprintf(stderr, "Removed breakpoint at %08X\n", m_currentBreakAddress);
	}
	else
		fprintf(stderr, "No breakpoint found at %08X\n", m_currentBreakAddress);*/

	// Reset hit breakpoint states
	for (auto& breakpoint : m_breakpoints)
	{
		if (breakpoint.isHit && breakpoint.isCommunicated)
		{
			breakpoint.isHit = 0;
			breakpoint.isCommunicated = 0;
		}
	}

	m_breakLatch = 0;
}

void CRV32::Tick(uint64_t wallclock, CBus* bus)
{
	int hitBreakpointCount = 0;
	for (auto& breakpoint : m_breakpoints)
		hitBreakpointCount += breakpoint.isHit;
	if (hitBreakpointCount)
		m_breakLatch = 1;

	if (!m_breakLatch)
	{
		CCSRMem* csr = bus->GetCSR(m_hartid);

		// Gather a block of code (or grab precompiled version)
		bool fetchok = FetchDecode(bus);
		csr->UpdateTime(wallclock, m_cycles);

		// Execute the whole block up to a possible breakpoint
		Execute(bus);

		if (csr->m_pendingCPUReset)
			m_fetchstate = EFetchInit;
	}
}
