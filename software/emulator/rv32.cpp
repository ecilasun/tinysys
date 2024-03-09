#include <stdio.h>
#include "rv32.h"

CRV32::CRV32()
{
}

CRV32::~CRV32()
{
}

void CRV32::SetMem(CMemMan *mem)
{
	m_mem = mem;
}

uint32_t CRV32::ALU()
{
	uint32_t aluout = 0;

	uint32_t selected = m_decoded.m_selimm ? m_decoded.m_immed : m_rval2;

	switch(m_decoded.m_aluop)
	{
		case ALU_OR:
			aluout = m_rval1 | selected;
		break;
		case ALU_SUB:
			aluout = m_rval1 + (~selected + 1); // val1-val2
		break;
		case ALU_SLL:
			aluout = m_rval1 << (selected&0x1F);
		break;
		case ALU_SLT:
			aluout = ((int32_t)m_rval1 < (int32_t)selected) ? 1 : 0;
		break;
		case ALU_SLTU:
			aluout = (m_rval1 < selected) ? 1 : 0;
		break;
		case ALU_XOR:
			aluout = m_rval1 ^ selected;
		break;
		case ALU_SRL:
			aluout = m_rval1 >> (selected&0x1F);
		break;
		case ALU_SRA:
			aluout = (int32_t)m_rval1 >> (selected&0x1F);
		break;
		case ALU_ADD:
			aluout = m_rval1 + selected;
		break;
		case ALU_AND:
			aluout = m_rval1 & selected;
		break;
	}

	return aluout;
}

uint32_t CRV32::BLU()
{
	uint32_t bluout = 0;

	switch (m_decoded.m_bluop)
	{
		case BLU_EQ:
			bluout = m_rval1 == m_rval2 ? 1 : 0;
		break;
		case BLU_NE:
			bluout = m_rval1 != m_rval2 ? 1 : 0;
		break;
		case BLU_L:
			bluout = (int32_t)bluout < (int32_t)m_rval1 ? 1 : 0;
		break;
		case BLU_GE:
			bluout = (int32_t)bluout >= (int32_t)m_rval1 ? 1 : 0;
		break;
		case BLU_LU:
			bluout = bluout < m_rval1 ? 1 : 0;
		break;
		case BLU_GEU:
			bluout = bluout >= m_rval1 ? 1 : 0;
		break;
	}

	return bluout;
}

void CRV32::DecodeInstruction(uint32_t instr, SDecodedInstruction& dec)
{
	dec.m_opcode = SelectBitRange(instr, 6, 0);
	dec.m_f3 = SelectBitRange(instr, 14, 12);
	dec.m_rs1 = SelectBitRange(instr, 19, 15);
	dec.m_rs2 = SelectBitRange(instr, 24, 20);
	dec.m_rd = SelectBitRange(instr, 11, 7);

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
			int32_t sign = int32_t(instr & 0x80000000) >> 21;		// 32-11 == 21
			uint32_t upper = SelectBitRange(instr, 30, 25);	// 6
			uint32_t lower = SelectBitRange(instr, 11, 7);	// +5 == 11
			dec.m_immed = sign | (upper<<5) | lower;
		}
		break;

		case OP_JAL:
		{
			// J-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 13;			// 32-19 == 13
			uint32_t upper = SelectBitRange(instr, 19, 12);		// 8
			uint32_t middle = SelectBitRange(instr, 20, 20);	// +1
			uint32_t lower = SelectBitRange(instr, 30, 21);		// +10 == 19
			dec.m_immed = sign | (upper<<11) | (middle<<10) | lower;
		}
		break;

		case OP_BRANCH:
		{
			// B-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 20;			// 32-12 == 20
			uint32_t upper = SelectBitRange(instr, 7, 7);		// 1
			uint32_t middle = SelectBitRange(instr, 30, 25);	// +6
			uint32_t lower = SelectBitRange(instr, 11, 8);		// +4
			uint32_t zero = 0x0;								// +1 == 12
			dec.m_immed = sign | (upper<<11) | (middle<<5) | (lower<<1);
		}
		break;

		case OP_OP_IMM:
		case OP_LOAD:
		case OP_JALR:
		{
			// I-imm
			int32_t sign = int32_t(instr & 0x80000000) >> 21;			// 32-11 == 21
			uint32_t lower = SelectBitRange(instr, 30, 20);		// 11
			dec.m_immed = sign | lower;
		}
		break;

		default:
		{
			dec.m_immed = 0;
		}
		break;
	}

	dec.m_aluop = ALU_NONE;
	dec.m_bluop = BLU_NONE;

	uint32_t mathopsel = SelectBitRange(instr, 30, 30);
	if (dec.m_opcode == OP_OP)
	{
		switch (dec.m_f3)
		{
			case 0b000:	dec.m_aluop = mathopsel ? ALU_SUB : ALU_ADD; break;
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
			case 0b000:	dec.m_bluop = BLU_EQ;
			case 0b001:	dec.m_bluop = BLU_NE;
			case 0b011:	dec.m_bluop = BLU_NONE;
			case 0b010:	dec.m_bluop = BLU_NONE;
			case 0b110:	dec.m_bluop = BLU_LU;
			case 0b111:	dec.m_bluop = BLU_GEU;
			case 0b101:	dec.m_bluop = BLU_GE;
			case 0b100:	dec.m_bluop = BLU_L;
			default:	dec.m_bluop = BLU_NONE;
		}
	}

	dec.m_selimm = (dec.m_opcode==OP_JALR) || (dec.m_opcode==OP_OP_IMM) || (dec.m_opcode==OP_LOAD) || (dec.m_opcode==OP_STORE);

	printf("  OP: 0x%.8x F3: 0x%.8x RS1: 0x%.8x RS2: 0x%.8x RD: 0x%.8x IMM: 0x%.8x\n", dec.m_opcode, dec.m_f3, dec.m_rs1, dec.m_rs2, dec.m_rd, dec.m_immed);
}

void CRV32::Tick(CClock& cpuclock)
{
	if (cpuclock.m_edge == RisingEdge)
	{
		// Process input and prepare intermediates
		switch (m_state)
		{
			case ECPUReset:
			{
				m_PC_next = m_resetvector;
				for (int i=0;i<32;++i)
					m_GPR_next[i] = 0;
				m_state_next = ECPUFetch;
			}
			break;

			case ECPUFetch:
			{
				m_instruction_next = m_mem->FetchInstruction(m_PC);
				printf("PC: %.8X INSTR: %.8x\n", m_PC, m_instruction_next);
				m_state_next = ECPUDecode;
			}
			break;

			case ECPUDecode:
			{
				DecodeInstruction(m_instruction, m_decoded_next);

				// TODO: this could potentially go to next clock, assuming comb. circuit for now
				m_rval1_next = m_GPR[m_decoded_next.m_rs1];
				m_rval2_next = m_GPR[m_decoded_next.m_rs2];

				m_state_next = ECPUExecute;
			}
			break;

			case ECPUExecute:
			{
				if (m_decoded.m_opcode != OP_JALR && m_decoded.m_opcode != OP_BRANCH)
					m_PC_next = m_PC + 4; // adjacentpc

				switch(m_decoded.m_opcode)
				{
					case OP_OP:
					case OP_OP_IMM:
						m_GPR_next[m_decoded.m_rd] = m_aluout;
					break;

					case OP_AUIPC:
						m_GPR_next[m_decoded.m_rd] = m_PC + m_decoded.m_immed; // offsetpc
					break;

					case OP_LUI:
						m_GPR_next[m_decoded.m_rd] = m_decoded.m_immed;
					break;

					case OP_JAL:
						m_GPR_next[m_decoded.m_rd] = m_PC + 4;
					break;

					case OP_JALR:
						m_GPR_next[m_decoded.m_rd] = m_GPR[m_decoded.m_rs1] + 4; // adjacentpc
						m_PC_next = m_PC + m_decoded.m_immed; // rwaddrs
					break;

					case OP_BRANCH:
						m_PC_next = m_branchout ? m_PC + m_decoded.m_immed : m_PC + 4;
					break;

					case OP_STORE:
						printf("  STORE\n");
					break;

					case OP_LOAD:
						printf("  LOAD\n");
					break;

					default:
						// TODO: trap illegal instruction
						printf("  ! ILLEGAL INSTRUCTION !\n");
					break;
				}
				fflush(stdout);
				m_state_next = ECPURetire;
			}
			break;

			case ECPURetire:
				m_state_next = ECPUFetch;
			break;

			default:
				printf("  ! ILLEGAL CPU STATE !\n");
			break;
		}

		m_aluout_next = ALU();
		m_branchout_next = BLU();
	}
	else
	{
		// Propagate state
		m_state = m_state_next;

		// Propagate intermediates to registers
		m_PC = m_PC_next;
		m_instruction = m_instruction_next;
		m_decoded = m_decoded_next;
		m_rval1 = m_rval1_next;
		m_rval2 = m_rval2_next;
		m_aluout = m_aluout_next;
		m_branchout = m_branchout_next;

		// Propagate GPR
		for (int i=0; i<32; ++i)
			m_GPR[i] = m_GPR_next[i];
	}
}
