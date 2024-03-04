#pragma once

#include <inttypes.h>

#define TASK_MAX 8

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
	TS_TERMINATED
};

struct STask {
	uint32_t HART;			// HART affinity mask (for migration)
	uint32_t runLength;		// Time slice dedicated to this task
	enum ETaskState state;	// State of this task
	uint32_t exitCode;		// Task termination exit code
	uint32_t regs[32];		// Integer registers - NOTE: register zero here is actually the PC

	// Debug support - this will probably move somewhere else
	char name[16];				// Name of this task
};

struct STaskContext {
	struct STask tasks[TASK_MAX];	// List of all the tasks
	int32_t currentTask;	// Current task index
	int32_t numTasks;		// Number of tasks
	int32_t debugFlags;		// Data for debugger
};

// Start up the task system
void TaskInitSystem(struct STaskContext *_ctx);

// Add a new task to the pool
int TaskAdd(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength);

// Switch to next task and return its time slice
uint32_t TaskSwitchToNext(struct STaskContext *_ctx);

// Exit current active task as soon as possible
void TaskExitCurrentTask(struct STaskContext *_ctx);

// Exit a specific task
void TaskExitTaskWithID(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal);

// Yield leftover time back to the next task in chain
// Also returns the current time
uint64_t TaskYield();

void TaskSetState(struct STaskContext *_ctx, const uint32_t _taskid, enum ETaskState _state);
enum ETaskState TaskGetState(struct STaskContext *_ctx, const uint32_t _taskid);
uint32_t TaskGetPC(struct STaskContext *_ctx, const uint32_t _taskid);

// Utilities
uint32_t TaskRead4Bytes(const uint32_t _address);
void TaskWrite4Bytes(const uint32_t _address, const uint32_t _bytes);
void TaskWrite2Bytes(const uint32_t _address, const uint16_t _bytes);
