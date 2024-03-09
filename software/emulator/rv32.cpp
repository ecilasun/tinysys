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

				uint32_t rwaddress = m_PC + m_decoded.m_immed;
				uint32_t rdin = 0;
				uint32_t rwen = 0;

				switch(m_decoded.m_opcode)
				{
					case OP_OP:
					case OP_OP_IMM:
						rdin = m_aluout;
						rwen = 1;
					break;

					case OP_AUIPC:
						rdin = m_PC + m_decoded.m_immed; // offsetpc
						rwen = 1;
					break;

					case OP_LUI:
						rdin = m_decoded.m_immed;
						rwen = 1;
					break;

					case OP_JAL:
						rdin = m_PC + 4;
						rwen = 1;
					break;

					case OP_JALR:
						rdin = m_GPR[m_decoded.m_rs1] + 4; // adjacentpc
						rwen = 1;
						m_PC_next = rwaddress;
					break;

					case OP_BRANCH:
						m_PC_next = m_branchout ? m_PC + m_decoded.m_immed : m_PC + 4;
					break;

					case OP_STORE:
					{
						uint32_t wdata = 0;
						uint32_t wstrobe = 0;
						uint32_t byte = SelectBitRange(m_rval2, 7, 0);
						uint32_t half = SelectBitRange(m_rval2, 15, 0);
						switch (m_decoded.m_f3)
						{
							case 0b000:	wdata = (byte<<24)|(byte<<16)|(byte<<8)|byte; break;
							case 0b001:	wdata = (half<<16)|half; break;
							default:	wdata = m_rval2; break;
						}
						uint32_t ab = SelectBitRange(rwaddress, 0, 0);
						uint32_t ah = SelectBitRange(rwaddress, 1, 1);
						uint32_t wordmask = (ah<<3)|(ah<<2)|((~ah)<<1)|(~ah);
						uint32_t bytemask = wordmask & ((ab<<3)|((~ab)<<2)|(ab<<1)|(~ab));
						switch (m_decoded.m_f3)
						{
							case 0b000:	wstrobe = bytemask; break;
							case 0b001:	wstrobe = wordmask; break;
							default:	wstrobe = 0b1111; break;
						}

						m_mem->WriteDataWord(rwaddress, wdata, wstrobe);
					}
					break;

					case OP_LOAD:
					{
						uint32_t dataword = m_mem->FetchDataWord(rwaddress);
						uint32_t range1 = SelectBitRange(rwaddress,1,1);
						uint32_t range2 = SelectBitRange(rwaddress,1,0);

						uint32_t b3 = SelectBitRange(dataword,31,24);
						uint32_t b2 = SelectBitRange(dataword,23,16);
						uint32_t b1 = SelectBitRange(dataword,15,8);
						uint32_t b0 = SelectBitRange(dataword,7,0);

						uint32_t h1 = SelectBitRange(dataword,31,16);
						uint32_t h0 = SelectBitRange(dataword,15,0);

						int32_t sign3 = int32_t(dataword & 0x80000000);
						int32_t sign2 = int32_t((dataword<<8) & 0x80000000);
						int32_t sign1 = int32_t((dataword<<16) & 0x80000000);
						int32_t sign0 = int32_t((dataword<<24) & 0x80000000);

						switch(m_decoded.m_f3)
						{
							case 0b000: // BYTE with sign extension
							{
								switch(range2)
								{
									case 0b11: rdin = (sign3>>24) | b3; break;
									case 0b10: rdin = (sign2>>24) | b2; break;
									case 0b01: rdin = (sign1>>24) | b1; break;
									case 0b00: rdin = (sign0>>24) | b0; break;
								}
							}
							break;
							case 0b001: // HALF with sign extension
							{
								switch(range1)
								{
									case 0b1: rdin = (sign3>>16) | h1; break;
									case 0b0: rdin = (sign1>>16) | h0; break;
								}
							}
							break;
							case 0b100: // BYTE with zero extension
							{
								switch(range2)
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
								switch(range1)
								{
									case 0b1: rdin = h1; break;
									case 0b0: rdin = h0; break;
								}
							}
							default: // WORD - 0b010
							{
								rdin = dataword;
							}
							break;
						}
						rwen = 1;
					}
					break;

					default:
						// TODO: trap illegal instruction
						printf("  ! ILLEGAL INSTRUCTION !\n");
					break;
				}

				if(rwen && m_decoded.m_rd != 0)
					m_GPR_next[m_decoded.m_rd] = rdin;

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
