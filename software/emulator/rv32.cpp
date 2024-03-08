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

void CRV32::DecodeInstruction(uint32_t instr, SDecodedInstruction& dec)
{
	dec.m_opcode = instr & 0x0000007F;
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
				printf("RESET\n");
				m_PC_next = m_resetvector;
				for (int i=0;i<32;++i)
					m_GPR_next[i] = 0;
				m_state_next = ECPUFetch;
			}
			break;

			case ECPUFetch:
			{
				printf("FETCH\n");
				m_instruction_next = m_mem->FetchInstruction(m_PC);
				m_state_next = ECPUDecode;
			}
			break;

			case ECPUDecode:
			{
				printf("DECODE\n");
				DecodeInstruction(m_instruction, m_decoded_next);
				m_state_next = ECPUExecute;
			}
			break;

			case ECPUExecute:
			{
				printf("EXECUTE\n");
				switch(m_decoded.m_opcode)
				{
					case OP_LUI:
					{
					}
					break;

					case OP_AUIPC:
					{
					}
					break;

					case OP_STORE:
					{
					}
					break;

					case OP_JAL:
					{
					}
					break;

					case OP_BRANCH:
					{
					}
					break;

					case OP_OP_IMM:
					{
					}
					break;

					case OP_LOAD:
					{
					}
					break;

					case OP_JALR:
					{
					}
					break;

					default:
						// illegal instruction
					break;
				}
				m_state_next = ECPURetire;
			}
			break;

			case ECPURetire:
			{
				printf("RETIRE\n");
				m_state_next = ECPUFetch;
			}
			break;

			default:
			break;
		}
	}
	else
	{
		// Propagate state
		m_state = m_state_next;

		// Propagate intermediates to registers
		m_PC = m_PC_next;
		m_instruction = m_instruction_next;
		m_decoded = m_decoded_next;
		for (int i=0; i<32; ++i)
			m_GPR[i] = m_GPR_next[i];
	}
}
