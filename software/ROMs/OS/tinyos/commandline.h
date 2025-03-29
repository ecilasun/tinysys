#include "basesystem.h"

// Console input and user ELF context
struct SCommandLineContext
{
	int32_t cmdLen;
	int refreshConsoleOut;
	uint32_t execParamCount;
	uint32_t userTaskID;
	char prevCmdString[128];
	char cmdString[128];
	char pathtmp[PATH_MAX];
	uint32_t startAddress;
	char execName[36];
	char execParam0[33];
};

struct SCommandLineContext* CLIGetContext();
void CLIClearStatics();
void _CLITask();