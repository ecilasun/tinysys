#ifdef CAT_WINDOWS
#include <winsock2.h>
#define socket_t SOCKET
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define socket_t int
#endif

#include "emulator.h"

void gdbprocesscommand(socket_t gdbsocket, CEmulator* emulator, char* buffer);
void gdbsendstopreason(socket_t gdbsocket, uint32_t cpu, CEmulator* emulator);
void gdbresponseack(socket_t gdbsocket);