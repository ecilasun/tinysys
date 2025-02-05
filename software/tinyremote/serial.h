#pragma once

#include "platform.h"

const char* GetCommDeviceName();
void SetCommDeviceName(const char* _commdevicename);
void SetCommDeviceName(const uint32_t _commdeviceindex);

class CSerialPort{
	public:

	CSerialPort() { }
	~CSerialPort() { }

	bool Open();
	bool AttemptOpen();
	uint32_t Receive(void *_target, unsigned int _rcvlength);
	uint32_t Send(void *_sendbytes, unsigned int _sendlength);
	void Close();

#if defined(CAT_LINUX) || defined(CAT_DARWIN)
	int serial_port{-1};
#else // CAT_WINDOWS
	HANDLE hComm{INVALID_HANDLE_VALUE};
	DCB serialParams{0};
	COMMTIMEOUTS timeouts{0};
#endif
	// Default port index is 1 for \\.\COM1 or /dev/ttyUSB1 etc
	uint32_t portIndex = 1;
};
