#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vgmopl2.h"

#include "basesystem.h"
#include "opl2.h"

// OPL2 output rate is supposed to be 49.716 KHz
// but on our hardware timing it seems to yield 48 KHz is a better match
static const float SampleRate = 48000.f;

namespace VGM
{

VGM* Load(const char* filename)
{
    FILE* fp = nullptr;
    fp = fopen(filename, "rb");
    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    VGM* vgm = (VGM*)malloc(sizeof(VGM) + fileSize);
    assert(vgm);
    fread(vgm + 1, 1, fileSize, fp);

    vgm->header = (VGM::Header*)(vgm + 1);
    vgm->commands = ((uint8_t*)&vgm->header->vgmDataOffset) + vgm->header->vgmDataOffset;
    vgm->currentCommand = 0;

    assert(vgm->header->ident == VGM::IDENT);
    assert(vgm->header->eofOffset == (uint32_t)(fileSize - 4));
    assert(vgm->header->version >= 0x00000151);

    fclose(fp);

    return vgm;
}

void Destroy(VGM* vgm)
{
    assert(vgm);
    free(vgm);
}

void Update(VGM* vgm)
{
    assert(vgm);

    uint64_t waitEnd = 0;

    do
    {
        uint64_t currentTicks = E32ReadTime();
        if (currentTicks <= waitEnd)
            continue;

        uint8_t command = vgm->commands[vgm->currentCommand];

        if (command >= 0x70 && command < 0x80)
        {
            // 0x7n - wait n+1 samples.
            uint32_t sampleCount = ((uint32_t)command & 0x0f) + 1;
            waitEnd = currentTicks + uint64_t(ONE_SECOND_IN_TICKS / (SampleRate/sampleCount));
            vgm->currentCommand++;
        }
        else
        {
            assert(command != 0);
            switch (command)
            {
                case 0x5a:
                {
                    // YM3812 write command.
                    uint8_t reg = vgm->commands[vgm->currentCommand + 1];
                    uint8_t val = vgm->commands[vgm->currentCommand + 2];
                    OPL2WriteReg(reg);
                    E32Sleep(320);
                    OPL2WriteVal(val);
                    E32Sleep(320);
                    vgm->currentCommand += 3;
                    break;
                }
                case 0x61:
                {
                    // Wait N samples command.
                    uint8_t sampleLo = vgm->commands[vgm->currentCommand + 1];
                    uint8_t sampleHi = vgm->commands[vgm->currentCommand + 2];
                    uint32_t  sampleCount = (sampleHi<<8) | sampleLo;
                    waitEnd = currentTicks + uint64_t(ONE_SECOND_IN_TICKS / (SampleRate/sampleCount));
                    vgm->currentCommand += 3;
                    break;
                }
                case 0x62:
                {
                    // Wait 735 samples (1/60th of a second).
                    waitEnd = currentTicks + uint64_t(ONE_SECOND_IN_TICKS / (SampleRate/735.f));
                    vgm->currentCommand++;
                    break;
                }
                case 0x63:
                {
                    // Wait 882 samples (1/50th of a second).
                    waitEnd = currentTicks + uint64_t(ONE_SECOND_IN_TICKS / (SampleRate/882.f));
                    vgm->currentCommand++;
                    break;
                }
                case 0x66:
                {
                    // End of sound data command (this will cause the song to loop).
                    vgm->currentCommand = 0;
                    break;
                }
                case 0x67:
                {
                    // Data block - skip this as we don't use it.
                    vgm->currentCommand++;
                    vgm->currentCommand++; // skip 0x66
                    vgm->currentCommand++; // skip data type
                    uint32_t size = *(uint32_t*)(&vgm->commands[vgm->currentCommand++]);
                    vgm->currentCommand += size;
                    break;
                }
                default:
                {
                    printf("cmd:%x\n", command);
                    assert(!"Unknown command");
                };
            }
        }
    }
    while(1);
}
}
