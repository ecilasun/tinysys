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

	switch(m_decoded.m_aluop)
	{
		case ALU_OR:
		{

		}
		break;
		case ALU_SUB:
		{

		}
		break;
		case ALU_SLL:
		{

		}
		break;
		case ALU_SLT:
		{

		}
		break;
		case ALU_SLTU:
		{

		}
		break;
		case ALU_XOR:
		{

		}
		break;
		case ALU_SRL:
		{

		}
		break;
		case ALU_SRA:
		{

		}
		break;
		case ALU_ADD:
		{

		}
		break;
		case ALU_AND:
		{

		}
		break;
	}

	return aluout;
}

uint32_t CRV32::BLU()
{
	
}

void CRV32::DecodeInstruction(uint32_t instr, SDecodedInstruction& dec)
{
	dec.m_opcode = instr & 0x0000007F;
	dec.m_aluop = ;
	dec.m_bluop = ;
	dec.m_f3 = (instr & 0x00007000) >> 12;
	dec.m_rs1 = (instr & 0x000F8000) >> 15;
	dec.m_rs2 = (instr & 0x01F00000) >> 20;
	dec.m_rd = (instr & 0x00000F80) >> 7;

	switch (dec.m_opcode)
	{
		case OP_LUI:
		case OP_AUIPC:
		{
			// U-imm
			dec.m_immed = instr & 0xFFFFF000;
		}
		break;

		case OP_STORE:
		{
			// S-imm
			int32_t sign = (instr & 0x80000000) >> 18;
			uint32_t upper = (instr & 0x7E000000) >> 18;
			uint32_t lower = (instr & 0x00003F80) >> 7;
			dec.m_immed = sign | upper | lower;
		}
		break;

		case OP_JAL:
		{
			// J-imm
			int32_t sign = (instr & 0x80000000) >> 11;
			uint32_t upper = (instr & 0x000FF000);
			uint32_t middle = (instr & 0x00100000) >> 8;
			uint32_t lower = (instr & 0x7FE00000) >> 20;
			dec.m_immed = sign | upper | middle | lower;
		}
		break;

		case OP_BRANCH:
		{
			// B-imm
			int32_t sign = (instr & 0x80000000) >> 19;
			uint32_t upper = (instr & 0x00000080) << 4;
			uint32_t middle = (instr & 0x7E000000) >> 20;
			uint32_t lower = (instr & 0x00000F00) >> 7;
			dec.m_immed = sign | upper | middle | lower;
		}
		break;

		case OP_OP_IMM:
		case OP_LOAD:
		case OP_JALR:
		{
			// I-imm
			int32_t sign = (instr & 0x80000000) >> 20;
			uint32_t lower = (instr & 0x7FF00000) >> 20;
			dec.m_immed = sign | lower;
		}
		break;

		default:
		{
			dec.m_immed = 0;
		}
		break;
	}


	dec.m_opcode = instr & 0x0000007F;
	dec.m_f3 = (instr & 0x00007000) >> 12;
	dec.m_rs1 = (instr & 0x000F8000) >> 15;
	dec.m_rs2 = (instr & 0x01F00000) >> 20;
	dec.m_rd = (instr & 0x00000F80) >> 7;
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
				printf("[RESET]\n");
				m_PC_next = m_resetvector;
				for (int i=0;i<32;++i)
					m_GPR_next[i] = 0;
				m_state_next = ECPUFetch;
			}
			break;

			case ECPUFetch:
			{
				printf("[FETCH]\n");
				m_instruction_next = m_mem->FetchInstruction(m_PC);
				printf("  PC: %.8X INSTR: %.8x\n", m_PC, m_instruction_next);
				m_state_next = ECPUDecode;
			}
			break;

			case ECPUDecode:
			{
				printf("[DECODE]\n");
				DecodeInstruction(m_instruction, m_decoded_next);

				// TODO: this could potentially go to next clock, assuming comb. circuit
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
					{
						m_GPR_next[m_decoded.m_rd] = m_aluout;
					}
					break;

					case OP_AUIPC:
					{
						m_GPR_next[m_decoded.m_rd] = m_PC + m_decoded.m_immed; // offsetpc
					}
					break;

					case OP_LUI:
					{
						m_GPR_next[m_decoded.m_rd] = m_decoded.m_immed;
					}
					break;

					case OP_JAL:
					{
						m_GPR_next[m_decoded.m_rd] = m_PC + 4;
					}
					break;

					case OP_JALR:
					{
						m_GPR_next[m_decoded.m_rd] = m_GPR[m_decoded.m_rs1] + 4; // adjacentpc
						m_PC_next = m_PC + m_decoded.m_immed; // rwaddrs
					}
					break;

					case OP_BRANCH:
					{
						m_PC_next = m_branchout ? m_PC + m_decoded.m_immed : m_PC + 4;
					}
					break;

					case OP_STORE:
					{
						printf("  STORE\n");
					}
					break;

					case OP_LOAD:
					{
						printf("  OP\n");
					}
					break;

					default:
						// TODO: trap illegal instruction
						printf("  ! ILLEGAL INSTRUCTION !\n");
					break;
				}
				m_state_next = ECPURetire;
			}
			break;

			case ECPURetire:
			{
				printf("[RETIRE]\n");
				m_state_next = ECPUFetch;
			}
			break;

			default:
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
