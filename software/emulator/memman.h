#include <stdlib.h>
#include <stdint.h>

class CMemMan
{
public:
    CMemMan();
    ~CMemMan();

    void *m_devicemem;

    uint32_t FetchInstruction(uint32_t address);
    uint32_t FetchDataWord(uint32_t address);
};
