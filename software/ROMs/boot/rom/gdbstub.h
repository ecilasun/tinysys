#include <inttypes.h>

// Signal defines
#define GDB_SIGNAL_TRAP 5
#define GDB_SIGNAL_INT 2
#define GDB_SIGNAL_ILL 4
#define GDB_SIGNAL_BUS 10
#define GDB_SIGNAL_SEGV 11
#define GDB_SIGNAL_FPE 8
#define GDB_SIGNAL_ABRT 6
#define GDB_SIGNAL_KILL 9
#define GDB_SIGNAL_STOP 19
#define GDB_SIGNAL_CONT 18
#define GDB_SIGNAL_TERM 15

uint8_t GDBChecksum(const char *data);
void GDBStubBeginPacket();
void GDBStubEndPacket();
void GDBStubAddByte(uint8_t byte);
void GDBStubInit();
uint32_t GDBIsDebugging();

// For task system
void GDBUpdateBreakpoints(const uint32_t _hartID, const uint32_t _taskID);
void GDBSignalBreakpoint(const uint32_t _hartID, const uint32_t _taskID, const uint32_t _pc, uint32_t _sigtype);