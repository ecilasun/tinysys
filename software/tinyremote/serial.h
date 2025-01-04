#pragma once

#include "platform.h"

const char* GetCommDeviceName();
void SetCommDeviceName(const char* _commdevicename);

class CSerialPort{
	public:

	CSerialPort() { }
	~CSerialPort() { }

	bool Open();
	uint32_t Receive(void *_target, unsigned int _rcvlength);
	uint32_t Send(void *_sendbytes, unsigned int _sendlength);
	void Close();

#if defined(CAT_LINUX) || defined(CAT_MACOS)
	int serial_port{-1};
#else // CAT_WINDOWS
	HANDLE hComm{INVALID_HANDLE_VALUE};
	DCB serialParams{0};
	COMMTIMEOUTS timeouts{0};
#endif
};
