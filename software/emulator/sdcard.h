#include <stdint.h>

class CSDCard
{
public:
	CSDCard() {}
	~CSDCard() {}

	uint32_t Read(uint32_t address);
	void Write(uint32_t address, uint32_t word);
};
