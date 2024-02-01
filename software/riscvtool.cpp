#if defined(CAT_LINUX) || defined(CAT_MACOS)
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>

#include <chrono>
#include <thread>

#if defined(CAT_LINUX) || defined(CAT_MACOS)
char devicename[512] = "/dev/ttyUSB1";
#else // CAT_WINDOWS
char devicename[512] = "\\\\.\\COM3";
#endif

unsigned int getfilelength(const fpos_t &endpos)
{
#if defined(CAT_LINUX)
	return (unsigned int)endpos.__pos;
#elif defined(CAT_MACOS)
	return (unsigned int)endpos;
#else // CAT_WINDOWS
	return (unsigned int)endpos;
#endif
}

class CSerialPort{
	public:

	CSerialPort() { }
	~CSerialPort() { }

	bool Open()
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		// Open COM port
		serial_port = open(devicename, O_RDWR);
		if (serial_port < 0 )
		{
			printf("Error %i from open: %s\n", errno, strerror(errno));
			return false;
		}

		struct termios tty;
		if(tcgetattr(serial_port, &tty) != 0)
		{
			printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
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

		cfsetispeed(&tty, B115200);
		cfsetospeed(&tty, B115200); // or only cfsetspeed(&tty, B115200);

		if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
			printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));

		printf("%s open\n", devicename);
		return true;

#else // CAT_WINDOWS
		hComm = CreateFileA(devicename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hComm != INVALID_HANDLE_VALUE)
		{
			serialParams.DCBlength = sizeof(serialParams);
			if (GetCommState(hComm, &serialParams))
			{
				serialParams.BaudRate = CBR_115200;
				serialParams.ByteSize = 8;
				serialParams.StopBits = ONESTOPBIT;
				serialParams.Parity = NOPARITY;

				if (SetCommState(hComm, &serialParams) != 0)
				{
					timeouts.ReadIntervalTimeout = 50;
					timeouts.ReadTotalTimeoutConstant = 50;
					timeouts.ReadTotalTimeoutMultiplier = 10;
					timeouts.WriteTotalTimeoutConstant = 50; // added to multiplier*numbyes
					timeouts.WriteTotalTimeoutMultiplier = 10; // times numbytes
					if (SetCommTimeouts(hComm, &timeouts) != 0)
						return true;
					else
						printf("ERROR: can't set communication timeouts\n");
					
				}
				else
					printf("ERROR: can't set communication parameters\n");
			}
			else
				printf("ERROR: can't get communication parameters\n");
		}
		else
			printf("ERROR: can't open COM port %s\n", devicename);
		return false;
#endif
	}

	uint32_t Receive(void *_target, unsigned int _rcvlength)
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		int n = read(serial_port, _target, _rcvlength);
		if (n < 0)
			printf("ERROR: read() failed\n");
		return n;
#else
		DWORD bytesread = 0;
		ReadFile(hComm, _target, _rcvlength, &bytesread, nullptr);
		return bytesread;
#endif
	}

	uint32_t Send(uint8_t *_sendbytes, unsigned int _sendlength)
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		int n = write(serial_port, _sendbytes, _sendlength);
		if (n < 0)
			printf("ERROR: write() failed\n");
		return n;
#else // CAT_WINDOWS
		DWORD byteswritten = 0;
		// Send the command
		BOOL success = WriteFile(hComm, _sendbytes, _sendlength, &byteswritten, nullptr);
		return (uint32_t)byteswritten;
#endif
	}

	void Close()
	{
#if defined(CAT_LINUX) || defined(CAT_MACOS)
		close(serial_port);
#else // CAT_WINDOWS
		CloseHandle(hComm);
#endif
	}

#if defined(CAT_LINUX) || defined(CAT_MACOS)
	int serial_port{-1};
#else // CAT_WINDOWS
	HANDLE hComm{INVALID_HANDLE_VALUE};
	DCB serialParams{0};
	COMMTIMEOUTS timeouts{0};
#endif
};

#pragma pack(push,1)
struct SElfFileHeader32
{
	unsigned int m_Magic;       // Magic word (0x7F followed by 'ELF' -> 0x464C457F)
	unsigned char m_Class;
	unsigned char m_Data;
	unsigned char m_EI_Version;
	unsigned char m_OSABI;
	unsigned char m_ABIVersion;
	unsigned char m_Pad[7];
	unsigned short m_Type;      // NONE(0x00)/REL(0x01)/EXEC(0x02)/DYN(0x03)/CORE(0x04)/LOOS(0xFE00)/HIOS(0xFEFF)/LOPROC(0xFF00)/HIPROC(0xFFFF)
	unsigned short m_Machine;   // RISC-V: 0xF3
	unsigned int m_Version;
	unsigned int m_Entry;
	unsigned int m_PHOff;       // Program header offset in file
	unsigned int m_SHOff;
	unsigned int m_Flags;
	unsigned short m_EHSize;
	unsigned short m_PHEntSize;
	unsigned short m_PHNum;
	unsigned short m_SHEntSize;
	unsigned short m_SHNum;
	unsigned short m_SHStrndx;
};
struct SElfProgramHeader32
{
	unsigned int m_Type;
	unsigned int m_Offset;      // Offset of entry point
	unsigned int m_VAddr;
	unsigned int m_PAddr;
	unsigned int m_FileSz;
	unsigned int m_MemSz;       // Length of code in memory (includes code and data sections)
	unsigned int m_Flags;
	unsigned int m_Align;
};
struct SElfSectionHeader32
{
	unsigned int m_NameOffset;
	unsigned int m_Type;
	unsigned int m_Flags;
	unsigned int m_Addr;
	unsigned int m_Offset;
	unsigned int m_Size;
	unsigned int m_Link;
	unsigned int m_Info;
	unsigned int m_AddrAlign;
	unsigned int m_EntSize;
};
#pragma pack(pop)

#define EAlignUp(_x_, _align_) ((_x_ + (_align_ - 1)) & (~(_align_ - 1)))

void parseelfheader(FILE *_op, unsigned char *_elfbinary, unsigned int _filebytesize, unsigned int groupsize, bool includeCOEHeader, bool dumpBinary)
{
	SElfFileHeader32 *fheader = (SElfFileHeader32 *)_elfbinary;

	if (fheader->m_Magic != 0x464C457F)
	{
		printf("Unknown magic, expecting 0x7F followed by 'ELF', got '%c%c%c%c' (%.8X)\n", fheader->m_Magic&0x000000FF, (fheader->m_Magic&0x0000FF00)>>8, (fheader->m_Magic&0x00FF0000)>>16, (fheader->m_Magic&0xFF000000)>>24, fheader->m_Magic);
		return;
	}

	// Gather a block large enough to fit everything
	uint32_t workblocksize = 0;
	for (uint32_t ph=0; ph<fheader->m_PHNum; ++ph)
	{
		SElfProgramHeader32 *pheader = (SElfProgramHeader32 *)(_elfbinary+fheader->m_PHOff+fheader->m_PHEntSize*ph);
		workblocksize += pheader->m_MemSz;
	}
	if (workblocksize == 0)
	{
		printf("Error: estimated output binary size is zero\n");
		return;
	}

	uint32_t *workblock = new uint32_t[workblocksize/4 + 16];

	if (includeCOEHeader)
		fprintf(_op, "memory_initialization_radix=16;\nmemory_initialization_vector=\n");

	int totalout = 0;
	for (uint32_t ph=0; ph<fheader->m_PHNum; ++ph)
	{
		SElfProgramHeader32 *pheader = (SElfProgramHeader32 *)(_elfbinary+fheader->m_PHOff+fheader->m_PHEntSize*ph);

		totalout += pheader->m_MemSz/4;
		if (pheader->m_MemSz != 0)
		{
			int woffset = (pheader->m_PAddr-fheader->m_Entry)/4;

			unsigned int *litteendian = (unsigned int *)(_elfbinary+pheader->m_Offset);

			// Reset blank space
			memset(workblock, 0x0, sizeof(uint32_t)*pheader->m_MemSz/4);

			// Fill rest with data from file
			if (groupsize == 4) // 32bit groups (4 bytes)
			{
				for (unsigned int i=0; i<pheader->m_FileSz/4; ++i)
				{
					workblock[(woffset&0xFFFFFFFC) + (woffset%4)] = litteendian[i];
					woffset++;
				}
			}
			else if (groupsize == 16) // 128bit groups (16 bytes)
			{
				for (unsigned int i=0; i<pheader->m_FileSz/4; ++i)
				{
					workblock[(woffset&0xFFFFFFFC) + (3-(woffset%4))] = litteendian[i];
					woffset++;
				}
			}
			else if (groupsize == 32) // 256bit groups (32 bytes)
			{
				for (unsigned int i=0; i<pheader->m_FileSz/4; ++i)
				{
					workblock[(woffset&0xFFFFFFF8) + (7-(woffset%8))] = litteendian[i];
					woffset++;
				}
			}
		}
	}

	// Pad to avoid misaligned data
	if (groupsize == 4)
		totalout = EAlignUp(totalout, groupsize);
	if (groupsize == 16)
		totalout = EAlignUp(totalout, groupsize/4);
	if (groupsize == 32)
		totalout = EAlignUp(totalout, groupsize/8);

	if (dumpBinary)
	{
		for(int i=0; i<totalout; ++i)
		{
			uint32_t dat = workblock[i];
			fwrite(&dat, 4, 1, _op);
		}
	}
	else
	{
		for(int i=0; i<totalout; ++i)
		{
			int breakpoint = (i!=0) && (groupsize == 4 ? 1 : (groupsize == 16 ? (i%4==0) : (groupsize == 32 ? (i%8==0) : 0)));
			if (breakpoint)
				fprintf(_op, "\n");
			fprintf(_op, "%.8X", workblock[i]);
		}
	}

	delete [] workblock;

	if (includeCOEHeader)
		fprintf(_op, ";");
}

void dumpelf(char *_filename, char *_outfilename, unsigned int groupsize, bool includeCOEHeader, bool dumpBinary)
{
	FILE *fp;
	fp = fopen(_filename, "rb");
	if (!fp)
	{
		printf("ERROR: can't open ELF file %s\n", _filename);
		return;
	}

	FILE *op = fopen(_outfilename, "wb");
	if (!op)
	{
		printf("ERROR: can't open output file %s\n", _outfilename);
		return;
	}

	unsigned int filebytesize = 0;
	fpos_t pos, endpos;
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fsetpos(fp, &pos);
	filebytesize = getfilelength(endpos);

	// TODO: Actual binary to send starts at 0x1000
	unsigned char *bytestosend = new unsigned char[filebytesize];
	fread(bytestosend, 1, filebytesize, fp);
	fclose(fp);

	parseelfheader(op, bytestosend, filebytesize, groupsize, includeCOEHeader, dumpBinary);

	fclose(op);

	delete [] bytestosend;
}

void sendfile(char *_filename)
{
	char tmpstring[128];

	FILE *fp;
	fp = fopen(_filename, "rb");
	if (!fp)
	{
		printf("ERROR: can't open ELF file %s\n", _filename);
		return;
	}

	unsigned int filebytesize = 0;
	fpos_t pos, endpos;
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fsetpos(fp, &pos);
	filebytesize = getfilelength(endpos);

	uint8_t *bytestoread = new uint8_t[filebytesize];
	fread(bytestoread, 1, filebytesize, fp);
	fclose(fp);

	CSerialPort serial;
	if (serial.Open() == false)
		return;

	uint32_t dummy = 0;

	// Send file transfer start
	snprintf(tmpstring, 128, "~");
	serial.Send((uint8_t*)tmpstring, 1);
	// Wait for !
	while (serial.Receive(&dummy, 1) == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }

	// Send name+zero terminator
	snprintf(tmpstring, 128, "!%s", _filename);
	serial.Send((uint8_t*)tmpstring, strlen(tmpstring)+1);
	// Wait for !
	while (serial.Receive(&dummy, 1) == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }

	// Send file length+zero terminator
	snprintf(tmpstring, 128, "!%d", filebytesize);
	serial.Send((uint8_t*)tmpstring, strlen(tmpstring)+1);
	// Wait for !
	while (serial.Receive(&dummy, 1) == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }

	// Send the file bytes
	uint32_t num64BytePackets = filebytesize/128;
	uint32_t leftoverBytes = filebytesize%128;
	uint32_t i = 0;
	for (i=0; i<num64BytePackets; ++i)
	{
		serial.Send(&bytestoread[i*128], 128);
		// Wait for !
		while (serial.Receive(&dummy, 1) == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }
	}

	if (leftoverBytes)
	{
		// Receive 'ready', then send
		serial.Send(&bytestoread[i*128], leftoverBytes);
		// Wait for !
		while (serial.Receive(&dummy, 1) == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }
	}

	serial.Close();
	printf("File sent\n");

	delete [] bytestoread;
}


int main(int argc, char **argv)
{
	if (argc <= 3)
	{
		printf("RISCVTool 1.0\n");
		printf("Usage: riscvtool.exe binaryfilename {-sendfile [usbdevicename]} | {-makerom|makemem|makebin groupbytesize} outputfilename\n");
		printf("NOTE: Default device name is %s", devicename);
		return -1;
	}

	if (strstr(argv[2], "-makerom"))
	{
		unsigned int groupsize = (unsigned int)strtoul(argv[3], nullptr, 10);
		dumpelf(argv[1], argv[4], groupsize, true, false);
	}
	if (strstr(argv[2], "-makemem"))
	{
		unsigned int groupsize = (unsigned int)strtoul(argv[3], nullptr, 10);
		dumpelf(argv[1], argv[4], groupsize, false, false);
	}
	if (strstr(argv[2], "-makebin"))
	{
		unsigned int groupsize = (unsigned int)strtoul(argv[3], nullptr, 10);
		dumpelf(argv[1], argv[4], groupsize, false, true);
	}
	if (strstr(argv[2], "-sendfile"))
	{
		if (argc > 3)
			strcpy(devicename, argv[3]);
		sendfile(argv[1]);
	}
	return 0;
}
