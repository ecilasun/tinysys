#include "clock.h"

class CEmulator
{
public:
    CEmulator() {}
    ~CEmulator() {}

    void Reset();
    bool Step();

    CClock m_clock;
};
