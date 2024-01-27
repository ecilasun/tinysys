#include <inttypes.h>

void HandleSerialInput();
void SendDebugPacket(const char *packetString);
uint32_t IsDebuggerConnected();