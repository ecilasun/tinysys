/**
 * @file sdcard.c
 * 
 * @brief This file contains the functions and commands to interface with the SD card
 */

#include "basesystem.h"
#include "sdcard.h"
#include "leds.h"
#include <stdio.h>

#define MAX_SDCARD_WRITE_ATTEMPTS 512

volatile uint8_t *IO_SPIRXTX = (volatile uint8_t* ) DEVICE_SPIC; // SPI read/write port
volatile uint8_t *IO_CARDDETECT = (volatile uint8_t* ) (DEVICE_SPIC+4); // SDCard insert/remove detect

typedef enum {
    CMD_NOT_SUPPORTED = -1,             /**< Command not supported error */
    CMD0_GO_IDLE_STATE = 0,             /**< Resets the SD Memory Card */
    CMD1_SEND_OP_COND = 1,              /**< Sends host capacity support */
    CMD6_SWITCH_FUNC = 6,               /**< Check and Switches card function */
    CMD8_SEND_IF_COND = 8,              /**< Supply voltage info */
    CMD9_SEND_CSD = 9,                  /**< Provides Card Specific data */
    CMD10_SEND_CID = 10,                /**< Provides Card Identification */
    CMD12_STOP_TRANSMISSION = 12,       /**< Forces the card to stop transmission */
    CMD13_SEND_STATUS = 13,             /**< Card responds with status */
    CMD16_SET_BLOCKLEN = 16,            /**< Length for SC card is set */
    CMD17_READ_SINGLE_BLOCK = 17,       /**< Read single block of data */
    CMD18_READ_MULTIPLE_BLOCK = 18,     /**< Card transfers data blocks to host until interrupted
                                                by a STOP_TRANSMISSION command */
    CMD24_WRITE_BLOCK = 24,             /**< Write single block of data */
    CMD25_WRITE_MULTIPLE_BLOCK = 25,    /**< Continuously writes blocks of data until
                                                'Stop Tran' token is sent */
    CMD27_PROGRAM_CSD = 27,             /**< Programming bits of CSD */
    CMD32_ERASE_WR_BLK_START_ADDR = 32, /**< Sets the address of the first write
                                                block to be erased. */
    CMD33_ERASE_WR_BLK_END_ADDR = 33,   /**< Sets the address of the last write
                                                block of the continuous range to be erased.*/
    CMD38_ERASE = 38,                   /**< Erases all previously selected write blocks */
    CMD55_APP_CMD = 55,                 /**< Extend to Applications specific commands */
    CMD56_GEN_CMD = 56,                 /**< General Purpose Command */
    CMD58_READ_OCR = 58,                /**< Read OCR register of card */
    CMD59_CRC_ON_OFF = 59,              /**< Turns the CRC option on or off*/
    // App Commands
    ACMD6_SET_BUS_WIDTH = 6,
    ACMD13_SD_STATUS = 13,
    ACMD22_SEND_NUM_WR_BLOCKS = 22,
    ACMD23_SET_WR_BLK_ERASE_COUNT = 23,
    ACMD41_SD_SEND_OP_COND = 41,
    ACMD42_SET_CLR_CARD_DETECT = 42,
    ACMD51_SEND_SCR = 51,
} SDCardCommand;

#define CMD8_PATTERN (0xAA)
#define SPI_CMD(x) (0x40 | (x & 0x3f))

#define G_SPI_TIMEOUT 65536

/**
 * @brief Calculate the CRC7 checksum for a given data buffer
 * 
 * @param data Data buffer
 * @param n Length of the data buffer
 * @return CRC7 checksum
 */
static uint8_t CRC7(const uint8_t* data, uint8_t n) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t d = data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) {
        crc ^= 0x09;
      }
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}

uint16_t CRC16_one(uint16_t crcIn, uint8_t data)
{
	crcIn  = (uint8_t)(crcIn >> 8)|(crcIn << 8);
	crcIn ^=  data;
	crcIn ^= (uint8_t)(crcIn & 0xff) >> 4;
	crcIn ^= (crcIn << 8) << 4;
	crcIn ^= ((crcIn & 0xff) << 4) << 1;

	return crcIn;
}

/**
 * @brief Calculate the CRC16 checksum for a given data buffer
 * 
 * @param pData Data buffer
 * @param len Length of the data buffer
 * @return CRC16 checksum
 */
static uint16_t CRC16(const uint8_t *pData, uint16_t len)
{
	uint16_t crc = 0;

	while (len--) crc = CRC16_one(crc,*pData++);

	return crc;
}

/**
 * @brief Perform a SPI transaction
 * 
 * This function performs a SPI transaction by writing a byte to the SPI slave and reading a byte from the SPI slave.
 * 
 * @param outbyte Byte to write
 * @return Byte read from the SPI slave
 */
uint8_t __attribute__ ((noinline)) SPITxRx(const uint8_t outbyte)
{
   *IO_SPIRXTX = outbyte;
   return *IO_SPIRXTX;
}

/**
 * @brief Send a command to the SD card
 * 
 * This function sends a command to the SD card and returns the response.
 * 
 * @param cmd Command to send
 * @param args Arguments for the command
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDCmd(const SDCardCommand cmd, uint32_t args)
{
   uint8_t buf[8];

   buf[0] = SPI_CMD(cmd);
   buf[1] = (uint8_t)((args&0xFF000000)>>24);
   buf[2] = (uint8_t)((args&0x00FF0000)>>16);
   buf[3] = (uint8_t)((args&0x0000FF00)>>8);
   buf[4] = (uint8_t)(args&0x000000FF);
   buf[5] = CRC7(buf, 5);

   uint8_t incoming;

   SPITxRx(0xFF); // Dummy byte

   for (uint32_t i=0;i<6;++i)
      incoming = SPITxRx(buf[i]);

   return incoming;
}

/**
 * @brief Wait for the SD card to become idle
 * 
 * This function waits for the SD card to become idle by sending 0xFF until the SD card responds with 0x00.
 * 
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDWaitNotBusy()
{
	uint8_t res = 0xFF;

	do
	{
		res = SPITxRx(0xFF);
	} while (res == 0x00);

	return res;
}

/**
 * @brief Get the single value response from the SD card
 * 
 * This function gets the single value response from the SD card by sending 0xFF until the SD card responds with a non-0xFF value.
 * 
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDResponse1()
{
   uint8_t res1 = 0xFF;

   int timeout = 0;
   while((res1 = SPITxRx(0xFF)) == 0xFF) {
      ++timeout;
      if (timeout > G_SPI_TIMEOUT)
         break;
   };

   return res1;
}

/**
 * @brief Get the 4 byte response from the SD card
 * 
 * This function gets the 4 byte response from the SD card by sending 0xFF until the SD card responds with a non-0xFF value.
 * It will then read the next 4 bytes and store them in the data pointer provided.
 * 
 * @param data Pointer to store the 4 byte response
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDResponse7(uint32_t *data)
{
   *data = 0x00000000;
   uint8_t res1 = SDResponse1();
   if (res1 > 1) return 0xFF;

   uint8_t d[4];
   d[0] = SPITxRx(0xFF);
   d[1] = SPITxRx(0xFF);
   d[2] = SPITxRx(0xFF);
   d[3] = SPITxRx(0xFF);

   *data = (d[0]<<24) | (d[1]<<16) | (d[2]<<8) | (d[3]);

   return res1;
}

/** 
 * @brief Put the SD card into idle mode
 * 
 * This function puts the SD card into idle mode by sending the CMD0_GO_IDLE_STATE command.
 * 
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDIdle()
{
   E32Sleep(1); // Wait for at least 1ms after power-on

   // At least 74 cycles with CS low to go to SPI mode
   //*IO_SPICTL = 0x1; // CS high power low
   for (int i=0; i<80; ++i)
      SPITxRx(0xFF);
   //*IO_SPICTL = 0x3; // CS + power low (these signals are inverted)

   SDCmd(CMD0_GO_IDLE_STATE, 0);
   uint8_t response = SDResponse1(); // Expected: 0x01

   return response;
}

/**
 * @brief Check the voltage range of the SD card
 * 
 * This function checks the voltage range of the SD card by sending the CMD8_SEND_IF_COND command.
 * 
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDCheckVoltageRange()
{
   SDCmd(CMD8_SEND_IF_COND, 0x000001AA);

   // Read the response and 00 00 01 AA sequence back from the SD CARD
   uint32_t databack;
   uint8_t response = SDResponse7(&databack); // Expected: 0x01(version 2 SDCARD) or 0x05(version 1 or MMC card)

   return response;
}

/**
 * @brief Initialize the SD card
 * 
 * This function initializes the SD card by sending the ACMD41_SD_SEND_OP_COND command.
 * 
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDCardInit()
{
   // Set high capacity mode on
   int timeout = 0;
   uint8_t rB = 0xFF;
   do {
      // ACMD header
      SDCmd(CMD55_APP_CMD, 0x00000000);
      /*rA =*/ rB = SDResponse1(); // Expected: 0x00?? - NOTE: Won't handle old cards!
      SDCmd(ACMD41_SD_SEND_OP_COND, 0x40000000);
      rB = SDResponse1(); // Expected: 0x00 eventually, but will also get several 0x01 (idle)
      ++timeout;
   } while (rB != SD_READY && timeout < G_SPI_TIMEOUT);

   return rB;
}

/**
 * @brief Set the block size of the SD card to 512 bytes
 * 
 * This function sets the block size of the SD card to 512 bytes by sending the CMD16_SET_BLOCKLEN command.
 * For modern cards this is the default block size.
 * 
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDSetBlockSize512()
{
   // Set block length
   SDCmd(CMD16_SET_BLOCKLEN, 0x00000200);
   uint8_t response = SDResponse1();

   return response;
}

/**
 * @brief Read a single block from the SD card
 * 
 * This function reads a single block from the SD card at given sector.
 * It will read but ignore any checksum errors for now.
 * 
 * @param sector Sector to read
 * @param datablock Pointer to store the data block
 * @param checksum Pointer to store the checksum
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDReadSingleBlock(uint32_t sector, uint8_t *datablock, uint8_t checksum[2])
{
   uint32_t oldstate = LEDGetState();
   LEDSetState(oldstate|0x2); // GREEN

   // Read single block
   // NOTE: sector<<9 for non SDHC cards
   SDCmd(CMD17_READ_SINGLE_BLOCK, sector);
   uint8_t response = SDResponse1(); // R1: expect 0x00

   if (response != 0xFF) // == 0x00
   {
      response = SDResponse1(); // R2: expect 0xFE

      if (response == SD_START_TOKEN)
      {
         // Data burst follows
         // 512 bytes of data followed by 16 bit CRC, total of 514 bytes
         int x=0;
         do {
            datablock[x++] = SPITxRx(0xFF);
         } while(x<512);

         // Checksum
         checksum[0] = SPITxRx(0xFF);
         checksum[1] = SPITxRx(0xFF);
      }
   }

   LEDSetState(oldstate);

   return response;
}

/**
 * @brief Read multiple blocks from the SD card
 * 
 * This function reads multiple blocks from the SD card starting at the given block address.
 * However, it will use repeated calls to SDReadSingleBlock to read the blocks, so it is less efficient.
 * Ideally, if the SD card supports CMD18_READ_MULTIPLE_BLOCK command, this function should be modified to use it.
 * 
 * @param datablock Pointer to store the data blocks
 * @param numblocks Number of blocks to read
 * @param blockaddress Block address to start reading from
 * @return 0 on success, -1 on failure
 */
int __attribute__ ((noinline)) SDReadMultipleBlocks(uint8_t *datablock, uint32_t numblocks, uint32_t blockaddress)
{
	if (numblocks == 0)
		return -1;

	uint32_t cursor = 0;

	uint8_t checksum[2];

	for(uint32_t b=0; b<numblocks; ++b)
	{
		uint8_t* target = (uint8_t*)(datablock+cursor);
		uint8_t response = SDReadSingleBlock(b+blockaddress, target, checksum);
		if (response != SD_START_TOKEN)
			return -1;
		cursor += 512;
	}

	return 0;
}

/**
 * @brief Write a single block to the SD card
 * 
 * This function writes a single block to the SD card at the given sector.
 * It will calculate the checksum and send it along with the data block.
 * However, it will not check for any checksum errors for now.
 * 
 * @param sector Sector to write
 * @param datablock Pointer to the data block
 * @return Response from the SD card
 */
uint8_t __attribute__ ((noinline)) SDWriteSingleBlock(uint32_t sector, uint8_t *datablock)
{
	uint32_t oldstate = LEDGetState();
	LEDSetState(oldstate|0x1); // RED

	uint16_t crc = CRC16(datablock, 512);

	SDCmd(CMD24_WRITE_BLOCK, sector);
	uint8_t response = SDResponse1(); // R1: expect 0x00

	//int haserror = 0;
	if (response == SD_READY)
	{
		// Send start token - single block (0xFD->multiblock)
		response = SPITxRx(SD_START_TOKEN);

		int x=0;
		do {
			response = SPITxRx(datablock[x++]);
		} while(x<512);

		// This seems to be unused in most samples, we'll emit 0xFFFF
		SPITxRx(crc >> 8);
		SPITxRx(crc & 0xff);

		// Read response token
		uint8_t writeAcceptState = SDResponse1() & 0x1F;

		if (writeAcceptState == 0x05)
		{
			// Wait for writes to finish
			SDWaitNotBusy();
			response = SD_READY;
		}
		else
		{
			// Error
			//if (writeAcceptState == 0x0b) UARTWrite("CRC error\n");
			//if (writeAcceptState == 0x0d) UARTWrite("Data rejected\n");
			// Expected status&x1F==0x05
			// 010:accepted, 101:rejected-crcerror, 110:rejected-writeerror
			SDCmd(CMD13_SEND_STATUS, 0);
			response = SDResponse1();
			//haserror = 1;
		}
	}
	else
	{
		//UARTWrite("Expected 0x00, ");
		//haserror = 1;
	}

	LEDSetState(oldstate);

	// Error
	/*if (haserror)
	{
		UARTWrite("SDWriteSingleBlock: write response: 0x");
		UARTWriteHexByte(response);
		UARTWrite("\n");
	}*/

	return response;
}

/**
 * @brief Write multiple blocks to the SD card
 * 
 * This function writes multiple blocks to the SD card starting at the given block address.
 * It will use repeated calls to SDWriteSingleBlock to write the blocks, so it is less efficient.
 * Ideally, if the SD card supports CMD25_WRITE_MULTIPLE_BLOCK command, this function should be modified to use it.
 * 
 * @param datablock Pointer to the data blocks
 * @param numblocks Number of blocks to write
 * @param blockaddress Block address to start writing from
 * @return 0 on success, -1 on failure
 */
int __attribute__ ((noinline)) SDWriteMultipleBlocks(const uint8_t *datablock, uint32_t numblocks, uint32_t blockaddress)
{
	if (numblocks == 0)
		return -1;

	uint32_t cursor = 0;

	for(uint32_t b=0; b<numblocks; ++b)
	{
		uint8_t* source = (uint8_t*)(datablock+cursor);
		uint8_t response = SDWriteSingleBlock(b+blockaddress, source);
		if (response != SD_READY)
			return -1;
		cursor += 512;
	}

	return 0;
}

/**
 * @brief Pass an I/O control command to the SD card
 * 
 * This function controls the SD card by sending an I/O command.
 *
 * @note This function is a stub and does nothing.
 * 
 * @param cmd Command to send
 * @param buffer Buffer to store the data
 * @return 0 on success, -1 on failure
 */
int __attribute__ ((noinline)) SDIOControl(const uint8_t cmd, void *buffer)
{
//CTRL_SYNC			   0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
//GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
//GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
//GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
//CTRL_TRIM			   4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */
//CTRL_POWER			5	/* Get/Set power status */
//CTRL_LOCK			   6	/* Lock/Unlock media removal */
//CTRL_EJECT			7	/* Eject media */
//CTRL_FORMAT			8	/* Create physical format on the media */
//MMC_GET_TYPE	   	10	/* Get card type */
//MMC_GET_CSD			11	/* Get CSD */
//MMC_GET_CID			12	/* Get CID */
//MMC_GET_OCR			13	/* Get OCR */
//MMC_GET_SDSTAT		14	/* Get SD status */
//ISDIO_READ			55	/* Read data form SD iSDIO register */
//ISDIO_WRITE			56	/* Write data to SD iSDIO register */
//ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */
//ATA_GET_REV			20	/* Get F/W revision */
//ATA_GET_MODEL		21	/* Get model name */
//ATA_GET_SN			22	/* Get serial number */

   switch (cmd)
   {
      case 0: /*CTRL_SYNC*/
      {
         // E32Sleep(ONE_MILLISECOND_IN_TICKS); // NOTE: All writes are blocking by default therefore always complete
      }
      break;
      /*default:
      {
         UARTWrite("SDIOControl: unknown ctrl\n");
      }
      break;*/
   }
   return 0;
}

/**
 * @brief Initialize the SD card
 * 
 * This function initializes the SD card by performing the following steps:
 * 1. Put the SD card into idle mode
 * 2. Check the voltage range of the SD card
 * 3. Initialize the SD card
 * 4. Set the block size of the SD card to 512 bytes
 * 
 * @return 0 on success, -1 on failure
 */
int __attribute__ ((noinline)) SDCardStartup()
{
   uint8_t response[4];

   response[0] = SDIdle();
   if (response[0] != 0x01)
      return -1;

   response[1] = SDCheckVoltageRange();
   if (response[1] != 0x01)
      return -1;

   response[2] = SDCardInit();
   if (response[2] != SD_READY)
      return -1;

   response[3] = SDSetBlockSize512();
   if (response[3] == SD_READY) // OK
      return 0;

   return -1;
}
