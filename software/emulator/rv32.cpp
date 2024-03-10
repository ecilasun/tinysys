#include <stdio.h>
#include "rv32.h"

const char *opnames[] = {
	{"illegal"},
	{""},
	{""},
	{"auipc"},
	{"lui"},
	{"store"},
	{"load"},
	{"jal"},
	{"jalr"},
	{"branch"},
	{"fence"},
	{"system"} };

const char *alunames[] = {
	{""}, // none
	{"add"},
	{"sub"},
	{"sll"},
	{"slt"},
	{"sltu"},
	{"xor"},
	{"srl"},
	{"sra"},
	{"or"},
	{"and"} };

const char *regnames[] = {
	{"zero"},
	{"ra"}, {"sp"},
	{"gp"}, {"tp"},
	{"t0"}, {"t1"}, {"t2"},
	{"s0"}, {"s1"},
	{"a0"}, {"a1"}, {"a2"}, {"a3"}, {"a4"}, {"a5"}, {"a6"}, {"a7"},
	{"s2"}, {"s3"}, {"s4"}, {"s5"}, {"s6"}, {"s7"}, {"s8"}, {"s9"}, {"s10"}, {"s11"},
	{"t3"}, {"t4"}, {"t5"}, {"t6"} };

CRV32::CRV32()
{
}

CRV32::~CRV32()
{
}

void CRV32::SetMemManager(CMemMan *mem)
{
	m_mem = mem;
}

uint32_t CRV32::ALU()
{
	uint32_t aluout = 0;

	uint32_t selected = m_decoded_next.m_selimm ? m_decoded_next.m_immed : m_rval2_next;

	switch(m_decoded_next.m_aluop)
	{
		case ALU_OR:
			aluout = m_rval1_next | selected;
		break;
		case ALU_SUB:
			aluout = m_rval1_next + (~selected + 1); // val1-val2
		break;
		case ALU_SLL:
			aluout = m_rval1_next << (selected&0x1F);
		break;
		case ALU_SLT:
			aluout = ((int32_t)m_rval1_next < (int32_t)selected) ? 1 : 0;
		break;
		case ALU_SLTU:
			aluout = (m_rval1_next < selected) ? 1 : 0;
		break;
		case ALU_XOR:
			aluout = m_rval1_next ^ selected;
		break;
		case ALU_SRL:
			aluout = m_rval1_next >> (selected&0x1F);
		break;
		case ALU_SRA:
			aluout = (int32_t)m_rval1_next >> (selected&0x1F);
		break;
		case ALU_ADD:
			aluout = m_rval1_next + selected;
		break;
		case ALU_AND:
			aluout = m_rval1_next & selected;
		break;
	}

	return aluout;
}

uint32_t CRV32::BLU()
{
	uint32_t bluout = 0;

	switch (m_decoded_next.m_bluop)
	{
		case BLU_EQ:
			bluout = m_rval1_next == m_rval2_next ? 1 : 0;
		break;
		case BLU_NE:
			bluout = m_rval1_next != m_rval2_next ? 1 : 0;
		break;
		case BLU_L:
			bluout = (int32_t)m_rval1_next < (int32_t)m_rval2_next ? 1 : 0;
		break;
		case BLU_GE:
			bluout = (int32_t)m_rval1_next >= (int32_t)m_rval2_next ? 1 : 0;
		break;
		case BLU_LU:
			bluout = m_rval1_next < m_rval2_next ? 1 : 0;
		break;
		case BLU_GEU:
			bluout = m_rval1_next >= m_rval2_next ? 1 : 0;
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
	dec.m_f12 = SelectBitRange(instr, 31, 20);
	dec.m_csroffset = (SelectBitRange(instr, 31, 25) << 5) | dec.m_rs2;

#if defined(DEBUG)
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
#endif

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

#if defined(DEBUG)
	// printf("%.8X: %s%s %s %s -> %s I=%d\n", m_PC, opnames[dec.m_opindex], alunames[dec.m_aluop], regnames[dec.m_rs1], regnames[dec.m_rs2], regnames[dec.m_rd], dec.m_immed);
#endif
}

bool CRV32::Tick(CClock& cpuclock)
{
	bool retval = true;

	if (cpuclock.m_edge == RisingEdge)
	{
		// We hack our way around with these CSR register
		// Normally they're shadowed to hardware counters
		// in the device so we don't have to emulate any latency here
		m_mem->m_csrmem[CSR_CYCLELO] = (uint32_t)(m_cyclecounter&0x00000000FFFFFFFFU);
		m_mem->m_csrmem[CSR_CYCLEHI] = (uint32_t)((m_cyclecounter&0xFFFFFFFF00000000U) >> 32);
		m_mem->m_csrmem[CSR_RETILO] = (uint32_t)(m_retired&0x00000000FFFFFFFFU);
		m_mem->m_csrmem[CSR_RETIHI] = (uint32_t)((m_retired&0xFFFFFFFF00000000U) >> 32);
		m_mem->m_csrmem[CSR_TIMELO] = (uint32_t)(m_wallclock&0x00000000FFFFFFFFU);
		m_mem->m_csrmem[CSR_TIMEHI] = (m_wallclock&0xFFFFFFFF00000000U) >> 32;

		++m_cyclecounter;
		if (m_cyclecounter%15 == 0) // 150MHz vs 10Mhz
			++m_wallclock;

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
				m_state_next = ECPUDecode;
			}
			break;

			case ECPUDecode:
			{
				DecodeInstruction(m_instruction, m_decoded_next);

				// NOTE: Register reads happen at end of this clock in hardware
				m_rval1_next = m_GPR[m_decoded_next.m_rs1];
				m_rval2_next = m_GPR[m_decoded_next.m_rs2];
				// Same deal with ALU and BLU
				m_aluout_next = ALU();
				m_branchout_next = BLU();

				m_state_next = ECPUExecute;
			}
			break;

			case ECPUExecute:
			{
				uint32_t adjacentpc = m_PC + 4;
				uint32_t rwaddress = m_rval1 + m_decoded.m_immed;
				uint32_t offsetpc = m_PC + m_decoded.m_immed;
				uint32_t jumpabs = m_rval1 + m_decoded.m_immed;
				uint32_t rdin = 0;
				uint32_t rwen = 0;
				uint32_t wdata = 0;
				uint32_t wstrobe = 0;

				if (m_decoded.m_opcode != OP_BRANCH && m_decoded.m_opcode != OP_JALR && m_decoded.m_opcode != OP_JAL)
					m_PC_next = adjacentpc;

				switch(m_decoded.m_opcode)
				{
					case OP_OP:
					case OP_OP_IMM:
						rdin = m_aluout;
						rwen = 1;
					break;

					case OP_AUIPC:
						rdin = offsetpc;
						rwen = 1;
					break;

					case OP_LUI:
						rdin = m_decoded.m_immed;
						rwen = 1;
					break;

					case OP_JAL:
						m_PC_next = offsetpc;
						rdin = adjacentpc;
						rwen = 1;
					break;

					case OP_JALR:
						m_PC_next = jumpabs;
						rdin = adjacentpc;
						rwen = 1;
					break;

					case OP_BRANCH:
						m_PC_next = m_branchout ? offsetpc : adjacentpc;
					break;

					case OP_FENCE:
					break;

					case OP_SYSTEM:
						if (m_decoded.m_f12 == F12_CDISCARD)
						{
						}
						else if (m_decoded.m_f12 == F12_CFLUSH)
						{
						}
						else if (m_decoded.m_f12 == F12_MRET)
						{
						}
						else if (m_decoded.m_f12 == F12_WFI)
						{
						}
						else if (m_decoded.m_f12 == F12_EBREAK)
						{
						}
						else if (m_decoded.m_f12 == F12_ECALL)
						{
						}
						else
						{
							// Read previous value
							uint32_t csrprevval = m_mem->FetchDataWord(CSRBASE + m_decoded.m_csroffset);

							// Keep it in a register
							rwen = 1;
							rdin = csrprevval;

							// Apply operation
							wstrobe = 0b1111;
							switch (m_decoded.m_f3)
							{
								case 0b001:	wdata = m_rval1; break;
								case 0b101:	wdata = m_decoded.m_immed; break;
								case 0b010:	wdata = csrprevval | m_rval1; break;
								case 0b110:	wdata = csrprevval | m_decoded.m_immed; break;
								case 0b011:	wdata = ~m_rval1; break;
								case 0b111:	wdata = ~m_decoded.m_immed; break;
								default:	wdata = csrprevval; break;
							}
						}
					break;

					case OP_STORE:
					{
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
						uint32_t wordmask = (ah<<3)|(ah<<2)|((1-ah)<<1)|(1-ah);
						uint32_t bytemask = wordmask & ((ab<<3)|((1-ab)<<2)|(ab<<1)|(1-ab));
						switch (m_decoded.m_f3)
						{
							case 0b000:	wstrobe = bytemask; break;
							case 0b001:	wstrobe = wordmask; break;
							default:	wstrobe = 0b1111; break;
						}
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
#if defined(DEBUG)
						printf("ILLEGAL_INSTRUCTION @PC 0x%.8X\n", m_PC);
						for (int i=0; i<32; ++i)
							printf("%s=%.8X ", regnames[i], m_GPR[i]);
						retval = false;
#endif
					break;
				}

				if(wstrobe)
					m_mem->WriteDataWord(rwaddress, wdata, wstrobe);

				if(rwen && m_decoded.m_rd != 0)
					m_GPR_next[m_decoded.m_rd] = rdin;

				m_state_next = ECPURetire;
			}
			break;

			case ECPURetire:
				++m_retired;
				m_state_next = ECPUFetch;
			break;

			default:
#if defined(DEBUG)
				printf("  ! ILLEGAL CPU STATE !\n");
#endif
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
		m_rval1 = m_rval1_next;
		m_rval2 = m_rval2_next;
		m_aluout = m_aluout_next;
		m_branchout = m_branchout_next;

		// Propagate GPR
		for (int i=0; i<32; ++i)
			m_GPR[i] = m_GPR_next[i];
	}

	return retval;
}
