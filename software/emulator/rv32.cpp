#include "rv32.h"

CRV32::CRV32()
{
}

CRV32::~CRV32()
{

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
				PC_next = 0;
				for (int i=0;i<32;++i)
					GPR_next[i] = 0;
				m_state_next = ECPUFetch;
			}
			break;

			case ECPUFetch:
			{
				m_state_next = ECPUDecode;
			}
			break;

			case ECPUDecode:
			{
				m_state_next = ECPUExecute;
			}
			break;

			case ECPUExecute:
			{
				m_state_next = ECPURetire;
			}
			break;

			case ECPURetire:
			{
				m_state_next = ECPUFetch;
			}
			break;

			default:
			break;
		}
	}
	else
	{
		// Propagate intermediates to registers
		PC = PC_next;
		m_state = m_state_next;
		for (int i=0;i<32;++i)
			GPR[i] = GPR_next[i];
	}
}
