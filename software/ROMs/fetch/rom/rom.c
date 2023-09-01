// Boot ROM

// This module contains instruction sequences for certain operations that will be inserted by the FETCH unit
// For instance, when an interrupt is encountered, enter*ISR() will be invoked and PC will be set to MTRAP

// MIP:
// Interrupt pending register for local interrupts when using CLINT modes of operation. In CLIC modes, this is hardwired to 0 and pending interrupts are handled using clicintip[i] memory mapped registers
// Machine external                  Internal timer                  Local software
// MEIP[11] WIRI[10] SEIP[9] UEIP[8] MTIP[7] WIRI[6] STIP[5] UTIP[4] MSIP[3] WIRI[2] SSIP[1] USIP[0]
//
// MIE:
// Interrupt enable register for local interrupts when using CLINT modes of operation. In CLIC modes, this is hardwired to 0 and interrupt enables are handled using clicintie[i] memory mapped registers
// Machine external                  Internal timer                  Local software
// MEIE[11] WPRI[10] SEIE[9] UEIE[8] MTIE[7] WPRI[6] STIP[5] UTIE[4] MSIE[3] WPRI[2] SSIE[1] USIE[0]
//
// MSTATUS:
// Status register containing interrupt enables for all privilege modes, previous privilege mode, and other privilege level setting
//                     Privilege stack              Enable stack                    Per-privilege global enable
// XS[16:15] FS[14:13] MPP[12:11] WPRI[10:9] SPP[8] MPIE[7] WPRI[6] SPIE[5] UPIE[4] MIE[3] WPRI[2] SIE[1] UIE[0]
//
// MCAUSE:
// Status register which indicates whether an exception or interrupt occurred, along with a code to distinguish details of each type.
// IRQ or Exception  Exception code
// ISEXCEPTION[31]   WLRL[31:0]
//
// MTVEC:
// Machine Trap Vector register which holds the base address of the interrupt vector table, as well as the interrupt mode configuration (direct or vectored) for CLINT and CLIC controllers. All synchronous exceptions also use mtvec as the base address for exception handling in all CLINT and CLIC modes.
// BASE[31:2] MODE[1:0] (modes-> 00:direct, 01:vectored, 10/11:reserved)
// Vectored interrupts (exceptions jump to BASE, interrupts jump to BASE+excode*4)
//
// Interrupt codes
// int excode   description
// 1   0        user software interrupt
// 1   1        supervisor software interrupt
// 1   2        reserved
// 1   3        machine software interrupt
// 1   4        user timer interrupt
// 1   5        supervisor timer interrupt
// 1   6        reserved
// 1   7        machine timer interrupt
// 1   8        user external interrupt
// 1   9        supervisor external interrupt
// 1   10       reserved
// 1   11       machine external interrupt
// 1   12>=     reserved
//
// Cause codes
// Interrupt	Exception Code	Description	
// 1	0	Reserved	
// 1	1	Supervisor software interrupt	
// 1	2	Reserved	
// 1	3	Machine software interrupt	
// 1	4	Reserved	
// 1	5	Supervisor timer interrupt	
// 1	6	Reserved	
// 1	7	Machine timer interrupt	
// 1	8	Reserved	
// 1	9	Supervisor external interrupt	
// 1	10	Reserved	
// 1	11	Machine external interrupt	
// 1	12–15	Reserved	
// 1	≥16	Designated for platform use
// 0	0	Instruction address misaligned	
// 0	1	Instruction access fault	
// 0	2	Illegal instruction	
// 0	3	Breakpoint	
// 0	4	Load address misaligned	
// 0	5	Load access fault	
// 0	6	Store/AMO address misaligned	
// 0	7	Store/AMO access fault	
// 0	8	Environment call from U-mode	
// 0	9	Environment call from S-mode	
// 0	10	Reserved	
// 0	11	Environment call from M-mode	
// 0	12	Instruction page fault	
// 0	13	Load page fault	
// 0	14	Reserved	
// 0	15	Store/AMO page fault	
// 0	16–23	Reserved	
// 0	24–31	Designated for custom use	
// 0	32–47	Reserved	
// 0	48–63	Designated for custom use	
// 0	≥64	Reserved
//
// Entry:
// 1) Save PC to MEPC
// 2) Save current privilege level to MSTATUS[MPP[1:0]]
// 3) Save MIE[*] to MSTATUS[MPIE]
// 4) Set MCAUSE[31] and MCAUSE[30:0] to 7 (interrupt, machine timer interrupt)
// 5) Set zero to MSTATUS[MIE]
// 6) Set PC to MTVEC (hardware)
// NOTE: Specific hardware interrupt tupe bits (UART/KEYBOARD/SDCARD etc) can be accessed via a custom CSR from the ISR itself
//
// Exit:
// 1) Set current privilege level to MSTATUS[MPP[1:0]]
// 2) Set MIE[*] to MSTATUS[MPIE]
// 4) Set MEPC to PC (hardware)

// NOTE: Hardware calls these instructions with the same PC
// of the instruction after which the IRQ occurs

// TIMER INTERRUPT

void __attribute__((naked)) enterTimerISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"auipc a5, 0;"              // Grab PC from INJECT stage of the CPU
		"csrw mepc, a5;"            // Set MEPC to PC for mret
		"li a5, 128;"               // Generate mask for bit 7
		"csrrc a5, mie, a5;"        // Extract MIE[7(MTIE)] and set it to zero
		"csrrs a5, mstatus, a5;"    // Copy it to MSTATUS[7(MPIE)]
		"li a5, 0x80000007;"        // Set MCAUSE[31] for interrupt and set MCAUSE[30:0] to 7
		"csrw mcause, a5;"
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mstatus, a5;"    // Clear MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware branches to mtvec
	);
}

void __attribute__((naked)) leaveTimerISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"li a5, 128;"               // Generate mask for bit 7
		"csrrc a5, mstatus, a5;"    // Extract MSTATUS[7(MPIE)] and set it to zero
		"csrrs a5, mie, a5;"        // Copy it to MIE[7(MTIE)]
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrs a5, mstatus, a5;"    // Set MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware sets PC <= MEPC;
		);
}

// HARDWARE INTERRUPT

void __attribute__((naked)) enterHWISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"auipc a5, 0;"              // Grab PC from INJECT stage of the CPU
		"csrw mepc, a5;"            // Set MEPC to PC+4 for mret
		"li a5, 2048;"              // Generate mask for bit 11
		"csrrc a5, mie, a5;"        // Extract MIE[11(MEIE)] and set it to zero
		"srl a5, a5, 4;"            // Shift to 7th bit position
		"csrrs a5, mstatus, a5;"    // Copy it to MSTATUS[7(MPIE)]
		"li a5, 0x8000000B;"        // Set MCAUSE[31] for interrupt and set MCAUSE[30:0] to 11
		"csrw mcause, a5;"
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mstatus, a5;"    // Clear MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware branches to mtvec
	);
}

void __attribute__((naked)) leaveHWSR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"li a5, 128;"               // Generate mask for bit 7
		"csrrc a5, mstatus, a5;"    // Extract MSTATUS[7(MPIE)] and set it to zero
		"sll a5, a5, 4;"            // Shift to 11th bit position
		"csrrs a5, mie, a5;"        // Copy it to MIE[11(MEIE)]
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrs a5, mstatus, a5;"    // Set MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware sets PC <= MEPC;
		);
}

// SOFTWARE INTERRUPT (ecall)

void __attribute__((naked)) enterSWecallISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"auipc a5, 0;"              // Grab PC+4 from INJECT stage of the CPU
		"csrw mepc, a5;"            // Set MEPC to PC+4 for mret
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mie, a5;"        // Extract MIE[3(MSIE)] and set it to zero
		"sll a5, a5, 4;"            // Shift to 7th bit position
		"csrrs a5, mstatus, a5;"    // Copy it to MSTATUS[7(MPIE)]
		"li a5, 0x0000000B;"        // Clear MCAUSE[31] for trap and set MCAUSE[30:0] to 0xB (machine ecall)
		"csrw mcause, a5;"
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mstatus, a5;"    // Clear MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware branches to mtvec
	);
}

void __attribute__((naked)) leaveSWecallISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"li a5, 128;"               // Generate mask for bit 7
		"csrrc a5, mstatus, a5;"    // Extract MSTATUS[7(MPIE)] and set it to zero
		"srl a5, a5, 4;"            // Shift to 3rd bit position
		"csrrs a5, mie, a5;"        // Copy it to MIE[3(MSIE)]
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrs a5, mstatus, a5;"    // Set MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware sets PC <= MEPC;
		);
}

// SOFTWARE INTERRUPT (ebreak)

void __attribute__((naked)) enterSWebreakISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"auipc a5, 0;"              // Grab PC from INJECT stage of the CPU
		"csrw mepc, a5;"            // Set MEPC to PC for mret
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mie, a5;"        // Extract MIE[3(MSIE)] and set it to zero
		"sll a5, a5, 4;"            // Shift to 7th bit position
		"csrrs a5, mstatus, a5;"    // Copy it to MSTATUS[7(MPIE)]
		"li a5, 0x00000003;"        // Clear MCAUSE[31] for trap and set MCAUSE[30:0] to 0x3 (debug breakpoint)
		"csrw mcause, a5;"
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mstatus, a5;"    // Clear MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware branches to mtvec
	);
}

void __attribute__((naked)) leaveSWebreakISR() // Same as leaveSWecallISR
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"li a5, 128;"               // Generate mask for bit 7
		"csrrc a5, mstatus, a5;"    // Extract MSTATUS[7(MPIE)] and set it to zero
		"srl a5, a5, 4;"            // Shift to 3rd bit position
		"csrrs a5, mie, a5;"        // Copy it to MIE[3(MSIE)]
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrs a5, mstatus, a5;"    // Set MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware sets PC <= MEPC;
		);
}

// SOFTWARE INTERRUPT (illegal instruction)

void __attribute__((naked)) enterSWillegalISR()
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"auipc a5, 0;"              // Grab PC from INJECT stage of the CPU
		"csrw mepc, a5;"            // Set MEPC to PC for mret
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mie, a5;"        // Extract MIE[3(MSIE)] and set it to zero
		"sll a5, a5, 4;"            // Shift to 7th bit position
		"csrrs a5, mstatus, a5;"    // Copy it to MSTATUS[7(MPIE)]
		"li a5, 0x00000002;"        // Clear MCAUSE[31] for trap and set MCAUSE[30:0] to 0x2 (illegal instruction)
		"csrw mcause, a5;"
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrc a5, mstatus, a5;"    // Clear MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware branches to mtvec
	);
}

void __attribute__((naked)) leaveSWillegalISR() // Same as leaveSWecallISR
{
	asm volatile(
		"csrw 0xFD0, a5;"           // Save current A5
		"li a5, 128;"               // Generate mask for bit 7
		"csrrc a5, mstatus, a5;"    // Extract MSTATUS[7(MPIE)] and set it to zero
		"srl a5, a5, 4;"            // Shift to 3rd bit position
		"csrrs a5, mie, a5;"        // Copy it to MIE[3(MSIE)]
		"li a5, 8;"                 // Generate mask for bit 3
		"csrrs a5, mstatus, a5;"    // Set MSTATUS[3(MIE)]
		"csrr a5, 0xFD0;"           // Restore old A5
		// Hardware sets PC <= MEPC;
		);
}

void __attribute__((naked, section (".boot"))) _start()
{
	// Make sure we call every function so that there's an asm listing produced for all
	enterTimerISR();
	leaveTimerISR();
	enterHWISR();
	leaveHWSR();
	enterSWecallISR();
	leaveSWecallISR();
	enterSWebreakISR();
	leaveSWebreakISR();
	enterSWillegalISR();
	leaveSWillegalISR();
}

void __attribute__((noreturn, naked, section (".boot"))) _exit(int x)
{
	// NOOP
}
