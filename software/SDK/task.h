#pragma once

#include <inttypes.h>

#define TASK_MAX 5

typedef void(*taskfunc)();

struct STaskBreakpoint
{
	uint32_t address;				// Address of replaced instruction / breakpoint
	uint32_t originalinstruction;	// Instruction we replaced with EBREAK
};

enum ETaskState
{
	TS_UNKNOWN,
	TS_PAUSED,
	TS_RUNNING,
	TS_TERMINATING,
	TS_TERMINATED,
	TS_CREATED
};

enum ETaskHandlerState
{
	TASK_IDLE = 0,		// Idle state for task handler (no tasks to run)
	TASK_SWITCHING = 1,	// Task switching in progress (for synchronization)
};

enum EBreakRequest
{
	BR_NONE = 0,				// No break request
	BR_HALTFORTASKSWITCH = 1,	// Let the core know we want to add a new task
};

// 152 bytes total for one task
struct STask {	
	uint32_t HART;					// HART affinity mask (for migration)
	uint32_t runLength;				// Time slice dedicated to this task
	enum ETaskState state;
	enum ETaskState targetstate;
	uint32_t exitCode;				// Task termination exit code
	uint32_t regs[32];				// Integer registers - NOTE: register zero here is actually the PC, 128 bytes
	uint32_t name;					// Pointer to task name (in external memory)
};

struct STaskContext {
	// 152 x 4 bytes total for all tasks
	struct STask tasks[TASK_MAX];	// List of all the tasks plus one for adding new tasks
	// 32 bytes total below
	enum ETaskHandlerState state;	// State of the task handler
	enum EBreakRequest brkreq;		// Break request
	int32_t brkack;					// Break acknowledgement
	int32_t currentTask;			// Current task index
	int32_t numTasks;				// Number of tasks
	int32_t kernelError;			// Current kernel error
	int32_t kernelErrorData[3];		// Data relevant to the crash
	int32_t hartID;					// Id of the HART where this task context runs
};

// Get task context for given HART
struct STaskContext *TaskGetContext(uint32_t _hartid);

// Shared memory space for all HARTs
void *TaskGetSharedMemory();

// Add a new task to the pool
int TaskAdd(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength, const uint32_t _stackAddress);

// Switch to next task and return its time slice
uint32_t TaskSwitchToNext(struct STaskContext *_ctx);

// Exit current active task as soon as possible
void TaskExitCurrentTask(struct STaskContext *_ctx);

// Exit a specific task
void TaskExitTaskWithID(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal);

// Yield leftover time back to the next task in chain
void TaskYield();

void TaskSetState(struct STaskContext *_ctx, const uint32_t _taskid, enum ETaskState _state);
enum ETaskState TaskGetState(struct STaskContext *_ctx, const uint32_t _taskid);
uint32_t TaskGetPC(struct STaskContext *_ctx, const uint32_t _taskid);
