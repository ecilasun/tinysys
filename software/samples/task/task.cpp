#include "basesystem.h"
#include "core.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

// Task context set up by OS
static struct STaskContext *s_taskctx = (struct STaskContext *)KERNEL_TASK_CONTEXT;

static const char *s_taskstates[]={
	"UNKNOWN    ",
	"PAUSED     ",
	"RUNNING    ",
	"TERMINATING",
	"TERMINATED " };

void MyTask()
{
	while(1)
	{
		printf("!");
		TaskYield();
	}
}

int main(int argc, char *argv[])
{
	printf("Current tasks:\n");
	if (s_taskctx->numTasks==1)
		printf("No tasks running\n");
	else
	{
		for (int i=0; i<s_taskctx->numTasks; ++i)
		{
			struct STask *task = &s_taskctx->tasks[i];
			printf("#%d:%s PC:0x%lx name:'%s'\n", i, s_taskstates[task->state], task->regs[0], task->name);
		}
	}

	int taskID = TaskAdd(s_taskctx, "mytask", MyTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	if (taskID == 0)
	{
		printf("Error: No room to add new task\n");
		return -1;
	}
	else
	{
		struct STask *task = &s_taskctx->tasks[taskID];
		printf("#%d:%s PC:0x%lx name:'%s'\n", taskID, s_taskstates[task->state], task->regs[0], task->name);
	}

	for (uint32_t i=0; i<2000; ++i)
	{
		printf(".");
		TaskYield();
		E32Sleep(HALF_SECOND_IN_TICKS);
	}

	// We're done with the test, remove our task
	TaskExitTaskWithID(s_taskctx, taskID, 0);

	return 0;
}
