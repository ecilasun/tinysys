#include <stdio.h>
#include <random>
#include <filesystem>
#include "sdcard.h"

#define SD_CMD_LEN 0x6

#define SD_CMD0 0
#define SD_CMD8 8
#define SD_CMD13 13
#define SD_CMD16 16
#define SD_CMD17 17
#define SD_CMD24 24
#define SD_CMD55 55
#define SD_CMD58 58
#define SD_ACMD41 41
#define DATA_START_BLOCK 0XFE
#define DATA_RES_ACCEPTED 0X05
#define SD_SECTOR_SIZE 512

extern "C" void SDInitBlockMem();
extern "C" void SDFreeBlockMem();
extern "C" void SDReportMemoryUsage();
extern "C" int SDReadBlock(uint32_t blockaddress, uint8_t* datablock);
extern "C" int SDWriteBlock(uint32_t blockaddress, const uint8_t* datablock);

void removeTextBeforeAndIncludingToken(std::string& str, const std::string& token) {
	size_t pos = str.find(token);
	if (pos != std::string::npos) {
		str.erase(0, pos + token.length());
		// Also replace backslashes with forward slashes
		std::replace(str.begin(), str.end(), '\\', '/');
	}
}

CSDCard::~CSDCard()
{
	delete m_fs;
	delete[] m_workbuf;
	SDFreeBlockMem();
}

void CSDCard::PopulateFileSystem()
{
	uint8_t* tmpmem = new uint8_t[16384];

	using namespace std::filesystem;
	path sourcePath = absolute("../sdcardmirror");
	std::string targetRoot = "sd:";
	for (const auto& entry : recursive_directory_iterator(sourcePath))
	{
		if (entry.is_directory())
		{
			std::string filePath = entry.path().string();
			removeTextBeforeAndIncludingToken(filePath, "sdcardmirror");
			f_mkdir((targetRoot+filePath).c_str());
		}

		if (entry.is_regular_file())
		{
			std::string filePath = entry.path().string();
			std::string actualPath = filePath;
			removeTextBeforeAndIncludingToken(filePath, "sdcardmirror");

			FILE *sourceFile = fopen(actualPath.c_str(), "rb");
			if (sourceFile)
			{
				size_t fs = entry.file_size();
				size_t numblocks = (fs+16383)/16384;

				FIL m_file;
				f_open(&m_file, (targetRoot + filePath).c_str(), FA_CREATE_ALWAYS | FA_WRITE);
				for (size_t i = 0; i < numblocks; ++i)
				{
					UINT readlen = (UINT)fread(tmpmem, 1, 16384, sourceFile);
					if (readlen)
					{
						UINT wbytes = 0;
						FRESULT fr = f_write(&m_file, tmpmem, readlen, &wbytes);
						if (fr != FR_OK || wbytes != readlen)
							printf("Failed to write file\n");
					}
				}
				f_close(&m_file);
			}
			fclose(sourceFile);
		}
	}

	delete[] tmpmem;
}

void CSDCard::Reset()
{
	// Prepare block of memory to hold the FAT32 image
	SDInitBlockMem();

	m_fs = new FATFS();
	m_workbuf = new uint8_t[4096];
	MKFS_PARM opt;
	opt.align = 0;
	opt.au_size = 4096;
	opt.n_fat = 2;
	opt.n_root = 2048;
	opt.fmt = FM_FAT32;
	FRESULT createattempt = f_mkfs("sd:", &opt, m_workbuf, 4096);
	if (createattempt != FR_OK)
		printf("Failed to create in-memory filesystem\n");
	else
	{
		FRESULT mountattempt = f_mount(m_fs, "sd:", 1);
		if (mountattempt != FR_OK)
			printf("Failed to mount in-memory filesystem\n");
		else
		{
			printf("Building file system from sdcardmirror folder\n");
			PopulateFileSystem();
		}
	}

	SDReportMemoryUsage();
}

uint32_t CSDCard::SPIRead(uint8_t *buffer, uint32_t len)
{
	uint32_t numread = 0;
	for (uint32_t i = 0; i < len; ++i)
	{
		if (m_spiinfifo.size())
		{
			buffer[i] = m_spiinfifo.front();
			m_spiinfifo.pop();
			++numread;
		}
	}

	return numread;
}

void CSDCard::ProcessSPI()
{
	// Run the SPI bus
	if (m_spiinfifo.size())
	{
		if (m_spimode == 0) // cmd
		{
			int rd = SPIRead(&m_cmdbyte, 1);
			// First two bits of transfer must be 0b01 to be a valid command
			if (rd && ((m_cmdbyte & 0xC0) == 0x40))
			{
				m_numdatabytes = 0;
				// We need data now
				m_spimode = 1;
			}
		}

		if (m_spimode == 1) // data
		{
			int rd = SPIRead(&m_databytes[m_numdatabytes], 1);
			m_numdatabytes += rd;
			if (m_numdatabytes == SD_CMD_LEN)
			{
				m_numdatabytes = 0;
				// TODO: process command and put something into m_spioutfifo as response
				uint8_t cmd = m_cmdbyte & 0b00111111;

				if (!m_app_mode)
				{
					switch (cmd)
					{
						case SD_CMD0: // GO_IDLE_STATE
						{
							m_spioutfifo.push(0x01);
						}
						break;

						case SD_CMD8: // SEND_IF_COND
						{
							// R7 response
							m_spioutfifo.push(0x01);
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(m_databytes[3]);
							m_spioutfifo.push(m_databytes[4]);
						}
						break;

						case SD_CMD13: // SEND_STATUS
						{
							// R2 response
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(0x00);
						}
						break;

						case SD_CMD16: // SET_BLOCKLEN
						{
							// R1 response
							m_spioutfifo.push(0x00);
						}
						break;

						case SD_CMD17: // READ_BLOCK
						{
							m_spioutfifo.push(0x00);				// expect to return 0 for ack
							m_spioutfifo.push(DATA_START_BLOCK);	// return start token

							m_readblock = (m_databytes[0] << 24) | (m_databytes[1] << 16) | (m_databytes[2] << 8) | (m_databytes[3]);

							// Return block from FAT32 image in memory
							SDReadBlock(m_readblock, m_datablock);
							for (int i = 0; i <SD_SECTOR_SIZE; ++i)
								m_spioutfifo.push(m_datablock[i]);

							m_spioutfifo.push(0xFF);
							m_spioutfifo.push(0xFF); // CRC
						}
						break;

						case SD_CMD24: // WRITE_BLOCK
						{
							m_writeblock = (m_databytes[0] << 24) | (m_databytes[1] << 16) | (m_databytes[2] << 8) | (m_databytes[3]);

							m_numdatabytes = 0;
							m_havestarttoken = 0;
							m_spimode = 2; // data write mode
						}
						break;

						case SD_CMD55: // APP_CMD
						{
							m_app_mode = true;
							m_spioutfifo.push(0x01);
						}
						break;

						case SD_CMD58: // READ_OCR
						{
							// R3 response
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(0xC0); // SDHC
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(0x00);
						}
						break;
					}
				}
				else
				{
					switch (cmd)
					{
						case SD_ACMD41: // SD_SEND_OP_COMD
						{
							// R1 response
							m_spioutfifo.push(0x00);
						}
						break;
					}
					m_app_mode = false;
				}

				// Done
				m_spimode = 0; // cmd
			}
		}

		// Data write mode
		if (m_spimode == 2)
		{
			// Wait for data start token or data
			int rd = SPIRead(&m_cmdbyte, 1);
			if (rd)
			{
				if (m_havestarttoken)
					m_datablock[m_numdatabytes++] = m_cmdbyte;
				else if (m_cmdbyte == DATA_START_BLOCK)
					m_havestarttoken = 1;
			}
			if (m_numdatabytes == SD_SECTOR_SIZE)
			{
				SDWriteBlock(m_writeblock, m_datablock);
				// now we wait for checksum
				m_spimode = 3;
				m_numdatabytes = 0;
			}
		}

		// Wait for checksum
		if (m_spimode == 3)
		{
			int rd = SPIRead(&m_cmdbyte, 1);
			if (rd)
				m_datablock[m_numdatabytes++] = m_cmdbyte;
			if (m_numdatabytes == 2)
			{
				m_spioutfifo.push(0x05); // data accepted
				m_spimode = 0;
				m_numdatabytes = 0;
			}
		}
	}
}

void CSDCard::Tick()
{
	ProcessSPI();
}

void CSDCard::Read(uint32_t address, uint32_t& data)
{
	if (m_spioutfifo.size())
	{
		data = m_spioutfifo.front();
		m_spioutfifo.pop();
	}
	else
		data = 0xFF;
	//printf("SDR:%.8X -> %.8X\n", address, data);
}

void CSDCard::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	// SPI bus write
	m_spiinfifo.push(word&0xFF);
	//printf("SDW:%.8X <- %.8X\n", address, word);
}
