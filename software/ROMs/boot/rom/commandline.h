#include "basesystem.h"

struct SCommandLineContext
{
	int32_t cmdLen;
	int refreshConsoleOut;
	uint32_t execParamCount;
	uint32_t userTaskID;
	char cmdString[128];
	char pathtmp[PATH_MAX];
	uint32_t startAddress;
	char execName[33];
	char execParam0[33];
};

struct SCommandLineContext* CLIGetContext();
void CLIClearStatics();
void _CLITask();