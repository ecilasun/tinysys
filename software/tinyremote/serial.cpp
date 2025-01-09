#include "serial.h"
#include <stdio.h>

#if defined(CAT_LINUX) || defined(CAT_MACOS)
//char commdevicename[512] = "/dev/ttyUSB0";
char commdevicename[512] = "/dev/ttyACM0";
#else // CAT_WINDOWS
char commdevicename[512] = "\\\\.\\COM6";
#endif

const char* GetCommDeviceName()
{
	return commdevicename;
}

void SetCommDeviceName(const char* _commdevicename)
{
	strcpy(commdevicename, _commdevicename);
}

bool CSerialPort::Open()
{
#if defined(CAT_LINUX)
	// Open COM port
	serial_port = open(commdevicename, O_RDWR);
	if (serial_port < 0 )
	{
		fprintf(stderr, "Error %i from open('%s'): %s\nPlease try another COM port path.\n", errno, commdevicename, strerror(errno));
		return false;
	}

	struct termios tty;
	if(tcgetattr(serial_port, &tty) != 0)
	{
		fprintf(stderr, "Error %i from tcgetattr: %s\nPlease make sure your user has access to the COM device %s.\n", errno, strerror(errno), commdevicename);
		return false;
	}

	// Set tty. flags
	tty.c_cflag &= ~PARENB; // No parity
	tty.c_cflag &= ~CSTOPB; // One stop bit
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; // 8 bits
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL; // Not model (local), write

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ISIG;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

	tty.c_oflag &= ~OPOST;
	tty.c_oflag &= ~ONLCR;
	//tty.c_oflag &= ~OXTABS;
	//tty.c_oflag &= ~ONOEOT;

	tty.c_cc[VTIME] = 50;
	tty.c_cc[VMIN] = 10;

	cfsetispeed(&tty, B460800);
	cfsetospeed(&tty, B460800); // or only cfsetspeed(&tty, B460800);

	if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
		fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));

	fprintf(stderr, "%s open\n", commdevicename);
	return true;
#elif defined(CAT_DARWIN)
	// MacOS
	return false;
#else // CAT_WINDOWS
	hComm = CreateFileA(commdevicename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hComm != INVALID_HANDLE_VALUE)
	{
		serialParams.DCBlength = sizeof(serialParams);
		if (GetCommState(hComm, &serialParams))
		{
			serialParams.BaudRate = /*CBR_*/460800;		// 460800 baud
			serialParams.fBinary = true;
			serialParams.fDtrControl = DTR_CONTROL_DISABLE;
			serialParams.fRtsControl = RTS_CONTROL_DISABLE;
			serialParams.fParity = 0;
			serialParams.ByteSize = 8;				// 8 bit bytes
			serialParams.StopBits = ONESTOPBIT;		// 1 stop bit
			serialParams.Parity = NOPARITY;			// no parity
			serialParams.fOutX = 0;					// no xon/xoff
			serialParams.fInX = 0;
			if (SetCommState(hComm, &serialParams) != 0)
			{
				fprintf(stderr, "%s open\n", commdevicename);
				timeouts.ReadIntervalTimeout = MAXDWORD;
				timeouts.ReadTotalTimeoutConstant = 0;
				timeouts.ReadTotalTimeoutMultiplier = 0;
				timeouts.WriteTotalTimeoutConstant = 0;
				timeouts.WriteTotalTimeoutMultiplier = 0;
				if (SetCommTimeouts(hComm, &timeouts) != 0)
					return true;
				else
					fprintf(stderr, "ERROR: can't set communication timeouts\n");
			}
			else
				fprintf(stderr, "ERROR: can't set communication parameters\n");
		}
		else
			fprintf(stderr, "ERROR: can't get communication parameters\n");
	}
	else
		fprintf(stderr, "ERROR: can't open COM port %s\n", commdevicename);
	return false;
#endif
}

uint32_t CSerialPort::Receive(void *_target, unsigned int _rcvlength)
{
#if defined(CAT_LINUX)
	int n = read(serial_port, _target, _rcvlength);
	if (n < 0)
		fprintf(stderr, "ERROR: read() failed\n");
	return n;
#elif defined(CAT_DARWIN)
	// MacOS
	return 0;
#else
	DWORD bytesread = 0;
	BOOL success = ReadFile(hComm, _target, _rcvlength, &bytesread, nullptr);
	return success ? bytesread : 0;
#endif
}

uint32_t CSerialPort::Send(void *_sendbytes, unsigned int _sendlength)
{
#if defined(CAT_LINUX)
	int n = write(serial_port, _sendbytes, _sendlength);
	if (n < 0)
		fprintf(stderr, "ERROR: write() failed\n");
	return n;
#elif defined(CAT_DARWIN)
	// MacOS
	return 0;
#else // CAT_WINDOWS
	DWORD byteswritten = 0;
	// Send the command
	BOOL success = WriteFile(hComm, _sendbytes, _sendlength, &byteswritten, nullptr);
	if (!success)
		fprintf(stderr, "ERROR: write() failed\n");
	return success ? (uint32_t)byteswritten : 0;
#endif
}

void CSerialPort::Close()
{
#if defined(CAT_LINUX)
	close(serial_port);
#elif defined(CAT_DARWIN)
	// MacOS
#else // CAT_WINDOWS
	CloseHandle(hComm);
#endif
}
