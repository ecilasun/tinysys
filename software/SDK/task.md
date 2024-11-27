# Multitasking

The OS on tinysys implements a hardware timer driven preemptive scheduler, and a few utility functions to aid in task generation.

This allows the OS to run several tasks, one of which is the user process, as well as allowing the user process to enqueue and run tasks of its own.

Tasks can yield execution to other tasks by calling the `TaskYield()` function. This function will only have an effect on the calling CPU and won't change the scheduling on other CPUs.

The task system can access the task context of other CPUs in the system from any other CPU, which allows for user tasks (always running on HART#0) to spawn tasks on either HART#0 or HART#1

### Accessing the OS task context
`struct STaskContext *TaskGetContext(uint32_t _hartid)`

This function returns the task context for a given CPU. Use the returned data pointer with care, directly modifying the returned memory contents is not recommended.

Use the returned context as the `_ctx` parameter for functions listed below.

### Shared uncached dmemory
`void *TaskGetSharedMemory()`

This function will return a shared memory space for HART to HART communications. The memory is uncached, so all writes from one HART are immediately visible to all other HARTs.

### Adding a new task
`int TaskAdd(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength)`

This will enqueue a task to be executed the next time scheduler reaches the last task in the current task list. The `_initialState` parameter should ideally be set to `TS_RUNNING` or if need be, to `TS_PAUSED` and none of the other values available. Doing so might break the consistency of the scheduler.

The `_task` function has to have a signature that matches the following:
```
typedef void(*taskfunc)();
```

The function will return a handle to the task created, which can then be stopped by calling the `TaskExitTaskWithID()` function listed below.

### Yielding time to other tasks
`void TaskYield()`

This function switches to the next task in the queue for the calling CPU, as quickly as possible.

Call this function in a tight loop that might be blocking other tasks or the OS, to yield time back to the calling CPU's scheduler before the task's run length is up.

### Stopping a task
`void TaskExitTaskWithID(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal)`

This function can be called with the same handle returned from the `TaskAdd()` function. Upon exiting, the return value of the task will be determined by the `_signal` value.

### Using the task system

For a full sample that uses the task system that prints a string and runs the debug LEDs simultaneously, see the sample code in 'samples/task' directory

### Back to [SDK Documentation](README.md)