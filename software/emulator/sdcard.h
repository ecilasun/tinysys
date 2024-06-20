#pragma once

#include <stdint.h>
#include <queue>

#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>

class Fat32Image {
private:
    std::vector<uint8_t> diskImage;
    const size_t sectorSize = 512;
    size_t totalSectors;
    size_t rootDirectoryStartSector; // Start sector of the root directory
    size_t rootDirectorySectors; // Number of sectors allocated for the root directory

    uint8_t lfnChecksum(const std::string& shortName) {
        uint8_t sum = 0;
        for (char c : shortName) {
            sum = (sum >> 1) + ((sum & 1) << 7) + static_cast<uint8_t>(c);
        }
        return sum;
    }

    void addDirectoryEntry(const std::vector<uint8_t>& entry) {
        // Simplified: Add the entry to the first available space in the root directory
        for (size_t sector = rootDirectoryStartSector; sector < rootDirectoryStartSector + rootDirectorySectors; ++sector) {
            auto sectorData = readSector(sector);
            for (size_t i = 0; i < sectorSize; i += 32) {
                if (sectorData[i] == 0x00 || sectorData[i] == 0xE5) { // Empty or deleted entry
                    std::copy(entry.begin(), entry.end(), sectorData.begin() + i);
                    writeSector(sector, sectorData);
                    return;
                }
            }
        }
        throw std::runtime_error("No space in root directory");
    }

public:
    Fat32Image(size_t totalSectors, size_t fatSectors, size_t rootDirStart, size_t rootDirSectors)
        : totalSectors(totalSectors), rootDirectoryStartSector(rootDirStart), rootDirectorySectors(rootDirSectors) {
        diskImage.resize(totalSectors * sectorSize, 0); // Initialize disk image with zeros
        // Initialize boot sector, FATs, and root directory here if needed
    }

    std::vector<uint8_t> readSector(size_t sectorNumber) {
        if (sectorNumber >= totalSectors) {
            throw std::out_of_range("Sector number out of range");
        }
        auto startIt = diskImage.begin() + (sectorNumber * sectorSize);
        return { startIt, startIt + sectorSize };
    }

    void writeSector(size_t sectorNumber, const std::vector<uint8_t>& data) {
        if (data.size() != sectorSize) {
            throw std::invalid_argument("Data size must be equal to sector size");
        }
        if (sectorNumber >= totalSectors) {
            throw std::out_of_range("Sector number out of range");
        }
        auto startIt = diskImage.begin() + (sectorNumber * sectorSize);
        std::copy(data.begin(), data.end(), startIt);
    }

    void addFileWithLongName(const std::string& longFileName, const std::string& shortFileName, int startingCluster, int fileSize) {
        uint8_t checksum = lfnChecksum(shortFileName);

        // Create and add LFN entries
        // Simplified: This example does not split the long file name into multiple LFN entries as required
        std::vector<uint8_t> lfnEntry(32, 0);
        lfnEntry[0] = 0x41; // LFN sequence number and allocation status
        lfnEntry[11] = 0x0F; // LFN attribute
        lfnEntry[13] = checksum;
        // Simplified: Copy part of the long file name into the LFN entry
        // In a real implementation, you would convert the name to UTF-16 and distribute it across multiple LFN entries
        addDirectoryEntry(lfnEntry);

        // Create and add the 8.3 entry
        std::vector<uint8_t> entry(32, 0);
        std::memcpy(entry.data(), shortFileName.c_str(), shortFileName.length());
        entry[11] = 0x20; // File attribute
        entry[26] = startingCluster & 0xFF; // Starting cluster, low byte
        entry[27] = (startingCluster >> 8) & 0xFF; // Starting cluster, high byte
        entry[28] = fileSize & 0xFF; // File size (little endian)
        entry[29] = (fileSize >> 8) & 0xFF;
        entry[30] = (fileSize >> 16) & 0xFF;
        entry[31] = (fileSize >> 24) & 0xFF;
        addDirectoryEntry(entry);
    }
};

class CSDCard
{
public:
	CSDCard() {}
	~CSDCard() {}

	void Reset();
	void Tick();
	void Read(uint32_t address, uint32_t& data);
	void Write(uint32_t address, uint32_t word, uint32_t wstrobe);

private:
    void ProcessSPI();
	uint32_t SPIRead(uint8_t* buffer, uint32_t len);
	std::queue<uint8_t> m_spiinfifo;
	std::queue<uint8_t> m_spioutfifo;
	uint32_t m_spimode{ 0 };
	uint32_t m_numdatabytes{ 0 };
	uint8_t m_databytes[8];
	uint8_t m_cmdbyte;
	bool m_app_mode{ false };

	Fat32Image* m_image;
};
