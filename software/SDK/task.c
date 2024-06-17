/**
 * @file task.c
 * 
 * @brief Task management for the SDK
 */

#include "core.h"
#include "basesystem.h"
#include "task.h"
#include "leds.h"

#include <stdlib.h>

/**
 * @brief Initialize the task system
 * 
 * Initialize the task system with the given task context and HART ID.
 * @note Tasks are core local at this point, and won't migrate between cores.
 * 
 * @param _ctx Task context
 * @param _hartid HART ID
 */
void TaskInitSystem(struct STaskContext *_ctx, uint32_t _hartid)
{
	_ctx->currentTask = 0;
	_ctx->numTasks = 0;
	_ctx->debugFlags = 0; // For debugger use
	_ctx->kernelError = 0;
	_ctx->hartID = _hartid;

	// Clean out all tasks
	for (uint32_t i=0; i<TASK_MAX; ++i)
	{
		struct STask *task = &_ctx->tasks[i];
		task->HART = 0x0;				// Default affinity mask is HART#0
		task->regs[0] = 0x0;			// Initial PC
		task->regs[2] = 0x0;			// Initial stack pointer
		task->regs[8] = 0x0;			// Frame pointer
		task->state = TS_UNKNOWN;
		task->name[0] = 0; // No name
	}
}

/**
 * @brief Set the state of a task
 * 
 * Transition a task to a new state.
 * 
 * @param _ctx Task context
 * @param _taskid Task ID
 * @param _state New state
 */
void TaskSetState(struct STaskContext *_ctx, const uint32_t _taskid, enum ETaskState _state)
{
	_ctx->tasks[_taskid].state = _state;
}

/**
 * @brief Get the state of a task
 * 
 * Get the current state of a task.
 * 
 * @param _ctx Task context
 * @param _taskid Task ID
 * @return Task state
 */
enum ETaskState TaskGetState(struct STaskContext *_ctx, const uint32_t _taskid)
{
	return _ctx->tasks[_taskid].state;
}

/**
 * @brief Get the program counter of a task
 * 
 * Get the program counter of a task.
 * 
 * @param _ctx Task context
 * @param _taskid Task ID
 * @return Program counter
 *
*/
uint32_t TaskGetPC(struct STaskContext *_ctx, const uint32_t _taskid)
{
	return _ctx->tasks[_taskid].regs[0];
}

/**
 * @brief Add a new task to the task pool
 * 
 * This function adds a new task to the task pool of the given task context which belongs to a specific HART.
 * 
 * @param _ctx Task context
 * @param _name Task name
 * @param _task Task function
 * @param _initialState Initial state
 * @param _runLength Time slice dedicated to this task (can be overriden by calling TaskYield())
 * @return Task ID
 */
int TaskAdd(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength)
{
	int32_t prevcount = _ctx->numTasks;
	if (prevcount >= TASK_MAX)
		return 0;

	// Stop timer interrupts on this core during this operation
	clear_csr(mie, MIP_MTIP);

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

	++_ctx->numTasks;

	// Resume timer interrupts on this core
	set_csr(mie, MIP_MTIP);

	return prevcount;
}

/**
 * @brief Exit the current task
 * 
 * Exit the current task as soon as possible by marking it as 'terminating'.
 * The scheduler will take care of the rest on next task switch.
 * 
 * @param _ctx Task context
 * @see TaskExitTaskWithID
 */
void TaskExitCurrentTask(struct STaskContext *_ctx)
{
	uint32_t taskid = _ctx->currentTask;
	if (taskid == 0) // Can not exit taskid 0
		return;

	struct STask *task = &_ctx->tasks[taskid];
	task->state = TS_TERMINATING;
	task->exitCode = 0; // Default exit code
}

/**
 * @brief Exit a specific task
 * 
 * Exit a specific task by marking it as 'terminating'.
 * The scheduler will take care of the rest on next task switch.
 * 
 * @param _ctx Task context
 * @param _taskid Task ID
 * @param _signal Exit code to return from the task
 * @see TaskExitCurrentTask
 */
void TaskExitTaskWithID(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal)
{
	if (_taskid == 0) // Can not exit taskid 0
		return;

	struct STask *task = &_ctx->tasks[_taskid];
	task->state = TS_TERMINATING;
	task->exitCode = _signal;
}

/**
 * @brief Yield leftover time back to the next task in chain
 * 
 * Yield leftover time back to the next task in chain.
 * Also returns the current time in clock ticks.
 * 
 * @return Current time in ticks
 * @see TaskSwitchToNext
 */
uint64_t TaskYield()
{
	// Set up the next task switch interrupt to almost-now
	// so we can yield as soon as possible.
	clear_csr(mie, MIP_MTIP);
	uint64_t now = E32ReadTime();
	E32SetTimeCompare(now + 100);
	set_csr(mie, MIP_MTIP);
	return now;
}

/**
 * @brief Switch to next task and return its total time slice
 * 
 * Switch to the next task in the task pool and return time slice of the next task.
 * This function is called by the scheduler to switch between tasks.
 * The current task's registers are stored in the task structure before switching.
 * 
 * @note Please call TaskYield() from user code to yield leftover time back to the next task instead.
 * 
 * @param _ctx Task context
 * @return Time slice of the next task
 * @see TaskYield
 */
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
