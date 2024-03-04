#include "basesystem.h"
#include "core.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

// Task context set up by OS
static struct STaskContext *s_taskctx = (struct STaskContext *)KERNEL_TASK_CONTEXT;

void MyTask()
{
	while(1)
	{
		printf("!");
	}
}

int main(int argc, char *argv[])
{
	printf("Task test\n");

	int taskID = TaskAdd(s_taskctx, "idle", MyTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	for (uint32_t i=0; i<1000; ++i)
	{
		printf(".");
		TaskYield();
	}

	TaskExitTaskWithID(s_taskctx, taskID, 0);

	return 0;
}
