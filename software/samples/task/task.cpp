#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "leds.h"

#include <stdio.h>
#include <string.h>

void MyTask()
{
	uint32_t state = 0;
	while(1)
	{
		LEDSetState(state++);
		E32Sleep(300*ONE_MILLISECOND_IN_TICKS);
		TaskYield();
	}
}

int main(int argc, char *argv[])
{
	// Grab task context of CPU#1
	struct STaskContext *taskctx = TaskGetContextOfCPU(1);

	// Add a new task to run
	int taskID = TaskAdd(taskctx, "mytask", MyTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS);

	if (taskID == 0)
	{
		printf("Error: No room to add new task\n");
		return -1;
	}

	for (uint32_t i=0; i<50; ++i)
	{
		printf("tick:%ld\n", i);
		TaskYield();
		E32Sleep(250*ONE_MILLISECOND_IN_TICKS);
	}

	// We're done with the test, remove our task
	TaskExitTaskWithID(taskctx, taskID, 0);

	return 0;
}
