#include <inttypes.h>

void ProcessGDBRequest();
void SendDebugPacket(const char *packetString);
uint32_t IsDebuggerConnected();