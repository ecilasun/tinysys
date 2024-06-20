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
extern "C" int SDReadMultipleBlocks(uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress);
extern "C" int SDWriteMultipleBlocks(const uint8_t* datablock, uint32_t numblocks, uint32_t blockaddress);

void removeTextBeforeAndIncludingToken(std::string& str, const std::string& token) {
	size_t pos = str.find(token);
	if (pos != std::string::npos) {
		str.erase(0, pos + token.length());
		// Also replace backslashes with forward slashes
		std::replace(str.begin(), str.end(), '\\', '/');
	}
}

void CSDCard::PopulateFileSystem()
{
	uint8_t tmpmem[4096];
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
				FIL m_file;
				f_open(&m_file, (targetRoot + filePath).c_str(), FA_CREATE_ALWAYS | FA_WRITE);

				size_t fs = entry.file_size();
				size_t numblocks = (fs+4093)/4096;
				for (size_t i = 0; i < numblocks; ++i)
				{
					UINT readlen = (UINT)fread(tmpmem, 1, 4096, sourceFile);
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
}

void CSDCard::Reset()
{
	// Prepare a 64 MByte range of block memory to hold the FAT32 image
	SDInitBlockMem();

	m_fs = new FATFS();
	uint8_t buf[1024];
	MKFS_PARM mkfs_param = { 0 };
	mkfs_param.align = 0x10000;
	mkfs_param.au_size = 0x1000;
	mkfs_param.fmt = FM_FAT32;
	mkfs_param.n_fat = 2;
	mkfs_param.n_root = 512;
	f_mkfs("sd:", &mkfs_param, buf, 1024);

	FRESULT mountattempt = f_mount(m_fs, "sd:", 1);
	if (mountattempt != FR_OK)
		printf("Failed to mount filesystem\n");
	else
	{
		printf("Building file system\n");
		PopulateFileSystem();
	}
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
							// Return an R1 response
							m_spioutfifo.push(0x01);
							// We always emulate an SD card, the arg is always block number
							uint32_t block_num = (uint32_t)m_databytes[0] << 24 | (uint32_t)m_databytes[1] << 16 | (uint32_t)m_databytes[2] << 8 | (uint32_t)m_databytes[3];
							m_spioutfifo.push(DATA_START_BLOCK);
							// Return block from FAT32 image in memory
							uint8_t datablock[512];
							SDReadMultipleBlocks(datablock, 1, block_num);
							for (int i = 0; i < SD_SECTOR_SIZE; ++i)
								m_spioutfifo.push(datablock[i]); // Return empty contents for now
							m_spioutfifo.push(0x00);
							m_spioutfifo.push(0x00); // CRC
						}
						break;

						case SD_CMD24: // WRITE_BLOCK
						{
							// Return an R1 response
							m_spioutfifo.push(0xFF); // fail
							// We always emulate an SD card, the arg is always block number
							uint32_t block_num = (uint32_t)m_databytes[0] << 24 | (uint32_t)m_databytes[1] << 16 | (uint32_t)m_databytes[2] << 8 | (uint32_t)m_databytes[3];
							//SDWriteMultipleBlocks...
							// Request the block from the application, can be read only
							/*uint8_t block[SD_SECTOR_SIZE];
							for (int i = 0; i < SD_SECTOR_SIZE; ++i)
								block[i] = m_databytes[4 + i];
								//std::vector<uint8_t> dataToWrite(512, 0xFF); // Example data to write
								//m_image.writeSector(10, dataToWrite); // Write to sector 10*/
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
	}
}

void CSDCard::Tick()
{
	// ProcessSPI
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
	ProcessSPI();
}
