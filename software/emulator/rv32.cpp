#include <stdio.h>
#include "rv32.h"
#include "bus.h"

const char *opnames[] = {
	"illegal",
	"",
	"",
	"auipc",
	"lui",
	"store",
	"load",
	"jal",
	"jalr",
	"branch",
	"fence",
	"system" };

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
	"and" };

const char *regnames[] = {
	"zero",
	"ra", "sp",
	"gp", "tp",
	"t0", "t1", "t2",
	"s0", "s1",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
	"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
	"t3", "t4", "t5", "t6" };

// This is an exact copy of the ISR ROM contents of the real hardware
uint32_t ISR_ROM[] = {
	0xfd079073,0x00000797,0x34179073,0x08000793,0x3047b073,0x3007a073,0x800007b7,0x00778793,0x34279073,0x00800793,0x3007b073,0xfd0027f3, // 0-11 TMI start
	0xfd079073,0x08000793,0x3007b073,0x3047a073,0x00800793,0x3007a073,0xfd0027f3, // 12-18 TMI end
	0xfd079073,0x00000797,0x34179073,0x000017b7,0x80078793,0x3047b073,0x0047d793,0x3007a073,0x800007b7,0x00b78793,0x34279073,0x00800793,0x3007b073,0xfd0027f3, // 19-32 HWI start
	0xfd079073,0x08000793,0x3007b073,0x00479793,0x3047a073,0x00800793,0x3007a073,0xfd0027f3, // 33-40 HWI end
	0xfd079073,0x00000797,0x34179073,0x00800793,0x3047b073,0x00479793,0x3007a073,0x00b00793,0x34279073,0x00800793,0x3007b073,0xfd0027f3, // 41-52 ecall start
	0xfd079073,0x08000793,0x3007b073,0x0047d793,0x3047a073,0x00800793,0x3007a073,0xfd0027f3, // 53-60 ecall end
	0xfd079073,0x00000797,0x34179073,0x00800793,0x3047b073,0x00479793,0x3007a073,0x00300793,0x34279073,0x00800793,0x3007b073,0xfd0027f3, // 61-72 ebreak start
	0xfd079073,0x08000793,0x3007b073,0x0047d793,0x3047a073,0x00800793,0x3007a073,0xfd0027f3, // 73-80 ebreak end
	0xfd079073,0x00000797,0x34179073,0x00800793,0x3047b073,0x00479793,0x3007a073,0x00200793,0x34279073,0x00800793,0x3007b073,0xfd0027f3, // 81-92 SWI start
	0xfd079073,0x08000793,0x3007b073,0x0047d793,0x3047a073,0x00800793,0x3007a073,0xfd0027f3 // 93-100 SWI end
};

void CRV32::Reset()
{
	m_fetchstate = EFetchInit;

	m_PC = m_resetvector;
	m_branchresolved = 0;
	m_wasmret = 0;
	m_branchtarget = 0;
	m_cyclecounter = 0;
	m_retired = 0;
	m_wallclock = 0;

	m_exceptionmode = 0;
	m_lasttrap = 0;

	for (uint32_t i=0; i<32; i++)
		m_GPR[i] = 0x00000000;

	m_instructionfifo = {};
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
				case 0b001: { a = sign1ext; b = sign2ext; } break; // mul/mulh
				case 0b010: { a = sign1ext; b = instr.m_rval2; } break; // mulhsu
				case 0b011: { a = instr.m_rval1; b = instr.m_rval2; } break; // mulhu
				default: { a = 0; b = 0; } break;
			}
			uint64_t result = a * b;
			aluout = instr.m_f3 == 0b000 ? (uint32_t)((a*b)&0xFFFFFFFF) : (uint32_t)(((a*b)&0xFFFFFFFF00000000)>>32);
		}
		break;
		case ALU_DIV:
		case ALU_REM:
		{
			uint64_t a, b;
			int64_t sign1ext = (int32_t)instr.m_rval1;
			int64_t sign2ext = (int32_t)instr.m_rval2;
			switch (instr.m_f3)
			{
				case 0b100: { a = sign1ext; b = sign2ext; aluout = (uint32_t)(a / b); } break; //  div
				case 0b101: { a = instr.m_rval1; b = instr.m_rval2; aluout = (uint32_t)(a / b); } break; // divu
				case 0b110: { a = sign1ext; b = sign2ext; aluout = (uint32_t)(a % b); } break; // rem
				case 0b111: { a = instr.m_rval1; b = instr.m_rval2; aluout = (uint32_t)(a % b); } break; // remu
				default: { a = 0; b = 1; } break;
			}
		}
		break;
	}

	return aluout;
}

uint32_t CRV32::BLU(SDecodedInstruction& instr)
{
	uint32_t bluout = 0;

	switch (instr.m_bluop)
	{
		case BLU_EQ:
			bluout = instr.m_rval1 == instr.m_rval2 ? 1 : 0;
		break;
		case BLU_NE:
			bluout = instr.m_rval1 != instr.m_rval2 ? 1 : 0;
		break;
		case BLU_L:
			bluout = (int32_t)instr.m_rval1 < (int32_t)instr.m_rval2 ? 1 : 0;
		break;
		case BLU_GE:
			bluout = (int32_t)instr.m_rval1 >= (int32_t)instr.m_rval2 ? 1 : 0;
		break;
		case BLU_LU:
			bluout = instr.m_rval1 < instr.m_rval2 ? 1 : 0;
		break;
		case BLU_GEU:
			bluout = instr.m_rval1 >= instr.m_rval2 ? 1 : 0;
		break;
	}

	return bluout;
}

void CRV32::DecodeInstruction(uint32_t pc, uint32_t instr, SDecodedInstruction& dec)
{
	dec.m_pc = pc;
	dec.m_opcode = SelectBitRange(instr, 6, 0);
	dec.m_f3 = SelectBitRange(instr, 14, 12);
	dec.m_rs1 = SelectBitRange(instr, 19, 15);
	dec.m_rs2 = SelectBitRange(instr, 24, 20);
	dec.m_rd = SelectBitRange(instr, 11, 7);
	dec.m_f12 = SelectBitRange(instr, 31, 20);
	dec.m_csroffset = (SelectBitRange(instr, 31, 25) << 5) | dec.m_rs2;

	switch (dec.m_opcode)
	{
		case OP_OP:		dec.m_opindex = 1; break;
		case OP_OP_IMM:	dec.m_opindex = 2; break;
		case OP_AUIPC:	dec.m_opindex = 3; break;
		case OP_LUI:	dec.m_opindex = 4; break;
		case OP_STORE:	dec.m_opindex = 5; break;
		case OP_LOAD:	dec.m_opindex = 6; break;
		case OP_JAL:	dec.m_opindex = 7; break;
		case OP_JALR:	dec.m_opindex = 8; break;
		case OP_BRANCH:	dec.m_opindex = 9; break;
		case OP_FENCE:	dec.m_opindex = 10; break;
		case OP_SYSTEM:	dec.m_opindex = 11; break;
		default:		dec.m_opindex = 0; break;
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
			dec.m_immed = sign | (upper<<12) | (middle<<11) | (lower<<1);
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
			dec.m_immed = sign | (upper<<11) | (middle<<5) | (lower<<1);
		}
		break;

		case OP_SYSTEM:
		{
			uint32_t lower = SelectBitRange(instr, 19, 15);	// 5
			dec.m_immed = lower;
		}
		break;

		case OP_OP_IMM:
		case OP_LOAD:
		case OP_JALR:
		{
			// I-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 20;	// 32-11 == 21
			uint32_t lower = SelectBitRange(instr, 30, 20);		// 11
			dec.m_immed = sign | lower;
		}
		break;

		default: // OP_FENCE
		{
			dec.m_immed = 0;
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

	//printf("#%d:: %.8X:%.8X %s%s %s %s -> %s I=%d\n", m_hartid, m_PC, instr, opnames[dec.m_opindex], alunames[dec.m_aluop], regnames[dec.m_rs1], regnames[dec.m_rs2], regnames[dec.m_rd], dec.m_immed);
}

void CRV32::InjectISRHeaderFooter()
{
	switch (m_exceptionmode)
	{
		// TMI start
		case 0x00000003: {
			for (uint32_t i = 0; i <= 11; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// TMI end
		case 0x80000003: {
			for (uint32_t i = 12; i <= 18; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// HWI start
		case 0x00000002: {
			for (uint32_t i = 19; i <= 32; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// HWI end
		case 0x80000002: {
			for (uint32_t i = 33; i <= 40; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// ecall start
		case 0x00000005: {
			for (uint32_t i = 41; i <= 52; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// ecall end
		case 0x80000005: {
			for (uint32_t i = 53; i <= 60; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// ebreak start
		case 0x00000004: {
			for (uint32_t i = 61; i <= 72; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// ebreak end
		case 0x80000004: {
			for (uint32_t i = 73; i <= 80; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// SWI start
		case 0x00000001: {
			for (uint32_t i = 81; i <= 92; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// SWI end
		case 0x80000001: {
			for (uint32_t i = 93; i <= 100; ++i)
			{
				SDecodedInstruction isrinstr;
				DecodeInstruction(m_PC, ISR_ROM[i], isrinstr);
				m_instructionfifo.push(isrinstr);
			}
		}
		break;

		// Unknown
		default:
		{
			// Nothing to inject
		}
		break;
	}
}

bool CRV32::FetchDecode(CBus& bus)
{
	if (m_fetchstate == EFetchInit)
	{
		m_pendingCPUReset = false;
		m_exceptionmode = 0;
		m_branchresolved = 0;
		m_PC = m_resetvector;
		if (m_instructionfifo.size() == 0)
			m_fetchstate = EFetchRead;
		return true;
	}

	if (m_fetchstate == EFetchWFI)
	{
		m_wficounter++;
		if (m_wficounter>=16 || m_irq)
			m_fetchstate = EFetchRead;
		return true;
	}

	if (m_fetchstate == EFetchRead)
	{
		uint32_t instruction;
		bus.Read(m_PC, instruction);
		SDecodedInstruction decoded;
		DecodeInstruction(m_PC, instruction, decoded);

		// Debug mode only, let ISR handle this
		if (decoded.m_opindex == 0)
		{
			printf("HART #%d: Illegal instruction 0x%.8X @0x%.8X exmode:%d branchres:%d irq:%d\n", m_hartid, instruction, m_PC, m_exceptionmode, m_branchresolved, m_irq);
			for (uint32_t i=0;i<32;++i)
				printf("r%d=%.8x\n", i, m_GPR[i]);
			return false;
		}

		bool isebreak = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_EBREAK;
		bool isecall = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_ECALL;
		bool ismret = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_MRET;
		bool iswfi = decoded.m_opcode == OP_SYSTEM && decoded.m_f12 == F12_WFI;
		bool isillegal = m_sie && decoded.m_opindex == 0;
		bool branchtomtvec = false;

		if (iswfi)
		{
			m_wficounter = 0;
			m_fetchstate = EFetchWFI;
		}
		else if ((isebreak || isecall || isillegal || m_irq) && m_exceptionmode == 0)
		{
			// For MRET to work properly this has to be pointing at the next instruction
			decoded.m_pc += 4;
			// Will need to route to mtvec
			branchtomtvec = true;
			if (m_irq & 1) // hardware
				m_exceptionmode = 0x00000002;
			else if (m_irq & 2) // timer
				m_exceptionmode = 0x00000003;
			else if (isillegal) // software - illegal instruction
				m_exceptionmode = 0x00000001;
			else if (isebreak) // ebreak
				m_exceptionmode = 0x00000004;
			else if (isecall) // ecall
				m_exceptionmode = 0x00000005;
			// Add header
			InjectISRHeaderFooter();
			m_lasttrap = m_exceptionmode;
		}
		else
			m_instructionfifo.push(decoded);

		// Determine next PC
		const uint32_t csrbase = (m_hartid == 0) ? CSR0BASE : CSR1BASE;
		if (branchtomtvec)
			bus.Read(csrbase + (CSR_MTVEC << 2), m_PC);
		else if (decoded.m_opcode != OP_BRANCH && decoded.m_opcode != OP_JALR && decoded.m_opcode != OP_JAL && !ismret)
			m_PC = decoded.m_pc + 4;
		else
			m_fetchstate = EFetchWaitForBranch; // wait for branch target from exec
	}
	else if (m_fetchstate == EFetchWaitForBranch)
	{
		if (m_branchresolved)
		{
			if (m_lasttrap && m_wasmret)
			{
				m_wasmret = 0;
				// Add footer
				m_exceptionmode = 0x80000000 | m_lasttrap;
				InjectISRHeaderFooter();
				// we're done
				m_exceptionmode = 0x00000000;
				m_lasttrap = 0x00000000;
			}

			m_branchresolved = 0;
			m_PC = m_branchtarget;
			m_fetchstate = EFetchRead;
		}
	}

	return true;
}

bool CRV32::Execute(CBus& bus)
{
	if (m_instructionfifo.size())
	{
		++m_cyclecounter;

		const uint32_t csrbase = (m_hartid == 0) ? CSR0BASE : CSR1BASE;
		bus.Write(csrbase + (CSR_RETILO << 2), (uint32_t)(m_retired & 0x00000000FFFFFFFFU), 0xFFFFFFFF);
		bus.Write(csrbase + (CSR_RETIHI << 2), (uint32_t)((m_retired & 0xFFFFFFFF00000000U) >> 32), 0xFFFFFFFF);
		bus.Write(csrbase + (CSR_PROGRAMCOUNTER << 2), m_PC, 0xFFFFFFFF);
		bus.Write(csrbase + (CSR_CYCLELO << 2), (uint32_t)(m_cyclecounter & 0x00000000FFFFFFFFU), 0xFFFFFFFF);
		bus.Write(csrbase + (CSR_CYCLEHI << 2), (uint32_t)((m_cyclecounter & 0xFFFFFFFF00000000U) >> 32), 0xFFFFFFFF);

		SDecodedInstruction instr;
		instr = m_instructionfifo.front();

		// Get register contents
		instr.m_rval1 = m_GPR[instr.m_rs1];
		instr.m_rval2 = m_GPR[instr.m_rs2];

		// Run ALU+BLU ops
		uint32_t aluout = ALU(instr);
		uint32_t branchout = BLU(instr);

		// Calculate future PC and other offsets
		uint32_t adjacentpc = instr.m_pc + 4;
		uint32_t rwaddress = instr.m_rval1 + instr.m_immed;
		uint32_t offsetpc = instr.m_pc + instr.m_immed;
		uint32_t jumpabs = instr.m_rval1 + instr.m_immed;
		uint32_t rdin = 0;
		uint32_t rwen = 0;
		uint32_t wdata = 0;
		uint32_t wstrobe = 0;

		/*printf("- @%.8X: op=%.8X r1=%.8x r2=%.8x imm=%.8X\n", instr.m_pc, instr.m_opcode, instr.m_rval1, instr.m_rval2, instr.m_immed);
		for (uint32_t i = 0; i < 32; ++i)
			printf("x%d=%.8x ", i, m_GPR[i]);
		printf("\n");*/

		// Execute
		switch (instr.m_opcode)
		{
			case OP_OP:
			case OP_OP_IMM:
				rdin = aluout;
				rwen = 1;
				//printf("- op/opimm %.8x\n", aluout);
			break;

			case OP_AUIPC:
				rdin = offsetpc;
				rwen = 1;
				//printf("- auipc %.8x\n", offsetpc);
			break;

			case OP_LUI:
				rdin = instr.m_immed;
				rwen = 1;
				//printf("- lui %.8x\n", instr.m_immed);
			break;

			case OP_JAL:
				m_branchresolved = 1;
				m_branchtarget = offsetpc;
				rdin = adjacentpc;
				rwen = 1;
				//printf("- jal %.8x\n", offsetpc);
			break;

			case OP_JALR:
				m_branchresolved = 1;
				m_branchtarget = jumpabs;
				rdin = adjacentpc;
				rwen = 1;
				//printf("- lalr %.8x\n", jumpabs);
			break;

			case OP_BRANCH:
				m_branchresolved = 1;
				m_branchtarget = branchout ? offsetpc : adjacentpc;
				//printf("- branch %.8x\n", branchout ? offsetpc : adjacentpc);
			break;

			case OP_FENCE:
				// NOOP for now
				//printf("- fencei\n");
			break;

			case OP_SYSTEM:
			{
				if (instr.m_f12 == F12_CDISCARD)
				{
					// cacheop=0b01
					// NOOP for now, D$ not implemented yet
					//printf("- cdiscard\n");
				}
				else if (instr.m_f12 == F12_CFLUSH)
				{
					// cacheop=0b11
					// NOOP for now, D$ not implemented yet
					//printf("- cflush\n");
				}
				else if (instr.m_f12 == F12_MRET)
				{
					m_wasmret = 1;
					m_branchresolved = 1;
					bus.Read(csrbase + (CSR_MEPC << 2), m_branchtarget);
				}
				else if (instr.m_f12 == F12_WFI)
				{
					// NOOP for now, ideally should wait for irq != 0 in a WFI state for about 16 clocks similar to real hardware
					//printf("- wfi\n");
				}
				else if (instr.m_f12 == F12_EBREAK)
				{
					//printf("- ebreak\n");
				}
				else if (instr.m_f12 == F12_ECALL)
				{
					//printf("- ecall\n");
				}
				else // CSROP
				{
					// Read previous value
					uint32_t csraddress = csrbase + (instr.m_csroffset << 2);
					uint32_t csrprevval;
					bus.Read(csraddress, csrprevval);

					// Keep it in a register
					rwen = 1;
					rdin = csrprevval;
					rwaddress = csraddress;

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

				//printf("- store %.8x\n", rwaddress);
			}
			break;

			case OP_LOAD:
			{
				uint32_t dataword;
				bus.Read(rwaddress, dataword);

				uint32_t range1 = SelectBitRange(rwaddress, 1, 1);
				uint32_t range2 = SelectBitRange(rwaddress, 1, 0);

				uint32_t b3 = SelectBitRange(dataword, 31, 24);
				uint32_t b2 = SelectBitRange(dataword, 23, 16);
				uint32_t b1 = SelectBitRange(dataword, 15, 8);
				uint32_t b0 = SelectBitRange(dataword, 7, 0);

				uint32_t h1 = SelectBitRange(dataword, 31, 16);
				uint32_t h0 = SelectBitRange(dataword, 15, 0);

				int32_t sign3 = int32_t(dataword & 0x80000000);
				int32_t sign2 = int32_t((dataword << 8) & 0x80000000);
				int32_t sign1 = int32_t((dataword << 16) & 0x80000000);
				int32_t sign0 = int32_t((dataword << 24) & 0x80000000);

				switch (instr.m_f3)
				{
					case 0b000: // BYTE with sign extension
					{
						switch (range2)
						{
							case 0b11: rdin = (sign3 >> 24) | b3; break;
							case 0b10: rdin = (sign2 >> 24) | b2; break;
							case 0b01: rdin = (sign1 >> 24) | b1; break;
							case 0b00: rdin = (sign0 >> 24) | b0; break;
						}
					}
					break;
					case 0b001: // HALF with sign extension
					{
						switch (range1)
						{
							case 0b1: rdin = (sign3 >> 16) | h1; break;
							case 0b0: rdin = (sign1 >> 16) | h0; break;
						}
					}
					break;
					case 0b100: // BYTE with zero extension
					{
						switch (range2)
						{
							case 0b11: rdin = b3; break;
							case 0b10: rdin = b2; break;
							case 0b01: rdin = b1; break;
							case 0b00: rdin = b0; break;
						}
					}
					break;
					case 0b101: // HALF with zero extension
					{
						switch (range1)
						{
							case 0b1: rdin = h1; break;
							case 0b0: rdin = h0; break;
						}
					}
					break;
					default: // WORD - 0b010
					{
						rdin = dataword;
					}
					break;
				}
				rwen = 1;

				//printf("- load @%.8x -> %.8x\n", rwaddress, instr.m_rd);
			}
			break;

			default:
			{
				//printf("- UNKNOWN\n");
			}
			break;
		}

		if (wstrobe)
		{
			//printf("- W @%.8X val=%.8x mask=%.8x\n", rwaddress, wdata, wstrobe);
			bus.Write(rwaddress, wdata, wstrobe);
		}

		if (rwen && instr.m_rd != 0)
		{
			//printf("- regw @%.8X val=%.8x\n", instr.m_rd, rdin);
			m_GPR[instr.m_rd] = rdin;
		}

		m_instructionfifo.pop();
		++m_retired;
	}

	return true;
}

bool CRV32::Tick(CBus& bus)
{
	const uint32_t csrbase = (m_hartid == 0) ? CSR0BASE : CSR1BASE;

	if (m_pendingCPUReset)
		m_fetchstate = EFetchInit;

	bool fetchok = FetchDecode(bus);
	bool execok = Execute(bus);

	if (m_cyclecounter % 15 == 0) // 150MHz vs 10Mhz
	{
		bus.Write(csrbase + (CSR_TIMELO << 2), (uint32_t)(m_wallclock & 0x00000000FFFFFFFFU), 0xFFFFFFFF);
		bus.Write(csrbase + (CSR_TIMEHI << 2), (uint32_t)((m_wallclock & 0xFFFFFFFF00000000U) >> 32), 0xFFFFFFFF);
		++m_wallclock;
	}

	return fetchok && execok;
}
