#include "core.h"
#include "basesystem.h"
#include "task.h"
#include "leds.h"

#include <stdlib.h>

// NOTE: Tasks are core local at this point, and won't migrate between cores.

// NOTE: Call with memory allocated for task tracking purposes
// with sufficient space for MAX_TASKS*sizeof(STaskContext) bytes
void TaskInitSystem(struct STaskContext *_ctx)
{
	_ctx->currentTask = 0;
	_ctx->numTasks = 0;
	_ctx->debugFlags = 0; // For debugger use

	// Clean out all tasks
	for (uint32_t i=0; i<TASK_MAX; ++i)
	{
		struct STask *task = &_ctx->tasks[i];
		task->HART = 0x1;				// Default affinity mask is HART#0
		task->regs[0] = 0x0;			// Initial PC
		task->regs[2] = 0x0;			// Initial stack pointer
		task->regs[8] = 0x0;			// Frame pointer
		task->ctrlc = 0;
		task->breakhit = 0;
		task->state = TS_UNKNOWN;
		task->name[0] = 0; // No name
		task->num_breakpoints = 0;
		for (int j=0; j<TASK_MAX_BREAKPOINTS; ++j)
		{
			task->breakpoints[j].address = 0x0;
			task->breakpoints[j].originalinstruction = 0x0;
		}
	}
}

int TaskInsertBreakpoint(struct STaskContext *_ctx, const uint32_t _taskid, uint32_t _address)
{
	struct STask *task = &(_ctx->tasks[_taskid]);
	uint32_t brk = task->num_breakpoints;

	if (brk == TASK_MAX_BREAKPOINTS-1)
		return 0;

	task->breakpoints[brk].address = _address;
	uint32_t instr = *(uint32_t*)_address;
	task->breakpoints[brk].originalinstruction = instr;

	// Replace it with EBREAK or C.EBREAK instruction depending on comression
	if ((instr&3) == 0x2)
		*(uint16_t*)_address = 0x9002;
	else 
		*(uint32_t*)_address = 0x00100073;

	// Make sure the write makes it to RAM and also visible to I$
	CFLUSH_D_L1;
	FENCE_I;

	return 1;
}

int TaskRemoveBreakpoint(struct STaskContext *_ctx, const uint32_t _taskid, uint32_t _address)
{
	struct STask *task = &(_ctx->tasks[_taskid]);
	uint32_t brk = task->num_breakpoints;

	for (uint32_t i=0; i<brk; ++i)
	{
		if (task->breakpoints[i].address == _address)
		{
			uint32_t instr = task->breakpoints[i].originalinstruction;

			// Replace it with EBREAK or C.EBREAK instruction depending on comression
			if ((instr&3) == 0x2)
				*(uint16_t*)_address = instr;
			else 
				*(uint32_t*)_address = instr;

			// Make sure the write makes it to RAM and also visible to I$
			CFLUSH_D_L1;
			FENCE_I;

			// Swap last entry over
			if (i != brk-1)
				task->breakpoints[i] = task->breakpoints[brk];
			task->num_breakpoints--;
			return 1;
		}
	}

	return 0;
}

int TaskAdd(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength)
{
	int32_t prevcount = _ctx->numTasks;
	if (prevcount >= TASK_MAX)
		return 0;

	// Stop timer interrupts on this core during this operation
	//write_csr(mie, MIP_MSIP | MIP_MEIP);

	++_ctx->numTasks;

	// Task stacks
	const uint32_t stacksizeword = 1024;
	uint32_t stackpointer = TASKMEM_END_STACK_END - (prevcount*stacksizeword);

	// Insert the task before we increment task count
	struct STask *task = &(_ctx->tasks[prevcount]);
	task->regs[0] = (uint32_t)_task;	// Initial PC
	task->regs[2] = stackpointer;		// Stack pointer
	task->regs[8] = stackpointer;		// Frame pointer
	task->runLength = _runLength;		// Time slice dedicated to this task

	char *np = (char*)_name;
	int idx = 0;
	while(np!=0 && idx<15)
	{
		task->name[idx++] = *np;
		++np;
	}
	task->name[idx] = 0;

	// We assume running state as soon as we start
	task->state = _initialState;

	// Resume timer interrupts on this core
	//write_csr(mie, MIP_MSIP | MIP_MEIP | MIP_MTIP);

	return 1;
}

void TaskExitCurrentTask(struct STaskContext *_ctx)
{
	uint32_t taskid = _ctx->currentTask;
	if (taskid == 0) // Can not exit taskid 0
		return;

	struct STask *task = &_ctx->tasks[taskid];
	task->state = TS_TERMINATING;
	task->exitCode = 0;
}

void TaskExitTaskWithID(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal)
{
	if (_taskid == 0) // Can not exit taskid 0
		return;

	struct STask *task = &_ctx->tasks[_taskid];
	task->state = TS_TERMINATING;
	task->exitCode = _signal;
}

void TaskYield()
{
	// Set up the next task switch interrupt to now
	// so we can yield as soon as possible.
	write_csr(mstatus, 0);
	uint64_t now = E32ReadTime();
	E32SetTimeCompare(now);
	write_csr(mstatus, MSTATUS_MIE);
}

uint32_t TaskSwitchToNext(struct STaskContext *_ctx)
{
	// Load current process ID from TP register
	int32_t currentTask = _ctx->currentTask;

	// Get current task's register stash
	uint32_t *regs = _ctx->tasks[currentTask].regs;

	// Store register back-ups in current task's structure
	regs[0] = read_csr(0x8A0);	// PC of halted task
	regs[1] = read_csr(0x8A1);	// ra
	regs[2] = read_csr(0x8A2);	// sp
	regs[3] = read_csr(0x8A3);	// gp
	regs[4] = read_csr(0x8A4);	// tp
	regs[5] = read_csr(0x8A5);	// t0
	regs[6] = read_csr(0x8A6);	// t1
	regs[7] = read_csr(0x8A7);	// t2
	regs[8] = read_csr(0x8A8);	// s0
	regs[9] = read_csr(0x8A9);	// s1
	regs[10] = read_csr(0x8AA);	// a0
	regs[11] = read_csr(0x8AB);	// a1
	regs[12] = read_csr(0x8AC);	// a2
	regs[13] = read_csr(0x8AD);	// a3
	regs[14] = read_csr(0x8AE);	// a4
	regs[15] = read_csr(0x8AF);	// a5
	regs[16] = read_csr(0x8B0);	// a6
	regs[17] = read_csr(0x8B1);	// a7
	regs[18] = read_csr(0x8B2);	// s2
	regs[19] = read_csr(0x8B3);	// s3
	regs[20] = read_csr(0x8B4);	// s4
	regs[21] = read_csr(0x8B5);	// s5
	regs[22] = read_csr(0x8B6);	// s6
	regs[23] = read_csr(0x8B7);	// s7
	regs[24] = read_csr(0x8B8);	// s8
	regs[25] = read_csr(0x8B9);	// s9
	regs[26] = read_csr(0x8BA);	// s10
	regs[27] = read_csr(0x8BB);	// s11
	regs[28] = read_csr(0x8BC);	// t3
	regs[29] = read_csr(0x8BD);	// t4
	regs[30] = read_csr(0x8BE);	// t5
	regs[31] = read_csr(0x8BF);	// t6

	// Break
	if (_ctx->tasks[currentTask].ctrlc == 1)
	{
		_ctx->tasks[currentTask].state = TS_PAUSED;
		_ctx->tasks[currentTask].ctrlc = 0;					// Avoid re-entry
		_ctx->tasks[currentTask].ctrlcaddress = regs[0];	// Save PC of the instruction on which we stopped
		uint32_t instr = *(uint32_t*)(regs[0]);				// Grab the instruction at this PC
		_ctx->tasks[currentTask].ctrlcbackup = instr;		// Save it into our backup space
		if ((instr&3) == 0x2)								// Is it compressed?
			*(uint16_t*)(regs[0]) = 0x9002;					// Replace it with C.EBREAK instruction
		else
			*(uint32_t*)(regs[0]) = 0x00100073;				// Replace it with EBREAK instruction
		CFLUSH_D_L1;										// Make sure the write makes it to RAM
		FENCE_I;											// Make sure I$ is flushed so it can see this write
	}

	// Resume
	if (_ctx->tasks[currentTask].ctrlc == 8)
	{
		_ctx->tasks[currentTask].state = TS_RUNNING;
		_ctx->tasks[currentTask].breakhit = 0;
		_ctx->tasks[currentTask].ctrlc = 0;											// Resuming
		uint32_t instr = _ctx->tasks[currentTask].ctrlcbackup;						// Grab stored instruction
		if ((instr&3) == 0x2)															// Is it compressed?
			*(uint16_t*)(_ctx->tasks[currentTask].ctrlcaddress) = (uint16_t)instr;	// Restore compressed instruction
		else
			*(uint32_t*)(_ctx->tasks[currentTask].ctrlcaddress) = instr;			// Restore full instruction
		CFLUSH_D_L1;																// Make sure the write makes it to RAM
		FENCE_I;																	// Make sure I$ is flushed so it can see this write
	}

	// Terminate task and visit OS task
	// NOTE: Task #0 cannot be terminated
	if (_ctx->tasks[currentTask].state == TS_TERMINATING)
	{
		if (currentTask != 0)
		{
			// Mark as 'terminated'
			_ctx->tasks[currentTask].state = TS_TERMINATED;
			// Replace with task at end of list, if we're not the end of list
			if (currentTask != _ctx->numTasks-1)
				__builtin_memcpy(&_ctx->tasks[currentTask], &_ctx->tasks[_ctx->numTasks-1], sizeof(struct STask));
			// One less task to run
			--_ctx->numTasks;
			// Rewind back to OS Idle task (always guaranteed to be alive)
			// We will visit it briefly once and then go to the next task in queue
			_ctx->currentTask = 0;
			currentTask = 0;
		}
		/*else
		{
			UARTWrite("\nKernel stub can't be terminated.\n");
		}*/
	}
	else
	{
		// Switch to next task
		// TODO: Next task state should not be `TS_PAUSED`
		currentTask = (_ctx->numTasks <= 1) ? 0 : ((currentTask+1) % _ctx->numTasks);
	}

	_ctx->currentTask = currentTask;

	regs = _ctx->tasks[currentTask].regs;

	// Load next tasks's registers into CSR file
	write_csr(0x8A0, regs[0]);	// PC of next task
	write_csr(0x8A1, regs[1]);	// ra
	write_csr(0x8A2, regs[2]);	// sp
	write_csr(0x8A3, regs[3]);	// gp
	write_csr(0x8A4, regs[4]);	// tp
	write_csr(0x8A5, regs[5]);	// t0
	write_csr(0x8A6, regs[6]);	// t1
	write_csr(0x8A7, regs[7]);	// t2
	write_csr(0x8A8, regs[8]);	// s0
	write_csr(0x8A9, regs[9]);	// s1
	write_csr(0x8AA, regs[10]);	// a0
	write_csr(0x8AB, regs[11]);	// a1
	write_csr(0x8AC, regs[12]);	// a2
	write_csr(0x8AD, regs[13]);	// a3
	write_csr(0x8AE, regs[14]);	// a4
	write_csr(0x8AF, regs[15]);	// a5
	write_csr(0x8B0, regs[16]);	// a6
	write_csr(0x8B1, regs[17]);	// a7
	write_csr(0x8B2, regs[18]);	// s2
	write_csr(0x8B3, regs[19]);	// s3
	write_csr(0x8B4, regs[20]);	// s4
	write_csr(0x8B5, regs[21]);	// s5
	write_csr(0x8B6, regs[22]);	// s6
	write_csr(0x8B7, regs[23]);	// s7
	write_csr(0x8B8, regs[24]);	// s8
	write_csr(0x8B9, regs[25]);	// s9
	write_csr(0x8BA, regs[26]);	// s10
	write_csr(0x8BB, regs[27]);	// s11
	write_csr(0x8BC, regs[28]);	// t3
	write_csr(0x8BD, regs[29]);	// t4
	write_csr(0x8BE, regs[30]);	// t5
	write_csr(0x8BF, regs[31]);	// t6

	return _ctx->tasks[currentTask].runLength;
}
