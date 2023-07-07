#pragma once

#include <stdint.h>

namespace VGM
{
#pragma pack(push, 1)
struct VGM
{
    static const uint32_t IDENT = 0x206d6756; // "Vgm "

    struct Header
    {
        uint32_t ident;
        uint32_t eofOffset;
        uint32_t version; // BCD version
        uint32_t sn76489Clock;
        uint32_t ym2413Clock;
        uint32_t gd3Offset;
        uint32_t totalSamples;
        uint32_t loopOffset;
        uint32_t loopNumSamples;

        // VGM 1.01
        uint32_t rate;

        // VGM 1.10
        uint16_t sn76489Feedback;
        uint8_t sn76489ShiftRegisterWidth;

        // VGM 1.51
        uint8_t sn76489Flags;

        // VGM 1.10
        uint32_t ym2612Clock;
        uint32_t ym2151Clock;

        // VGM 1.50
        uint32_t vgmDataOffset;

        // VGM 1.51
        uint32_t segaPCMClock;
        uint32_t segaPCMInterfaceRegister;
        uint32_t RF5C68Clock;
        uint32_t YM2203Clock;
        uint32_t YM2608Clock;
        uint32_t YM2610Clock;
        uint32_t YM3812Clock;
        uint32_t YM3526Clock;
        uint32_t Y8950Clock;
        uint32_t YMF262Clock;
        uint32_t YMF278BClock;
        uint32_t YMF271Clock;
        uint32_t YMZ280BClock;
        uint32_t RF5C164Clock;
        uint32_t PWMClock;
        uint32_t AY8910Clock;
        uint8_t AY8910ChipType;
        uint8_t AY8910Flags;
        uint8_t YM2203Flags;
        uint8_t YM2608Flags;

        // VGM 1.60
        uint8_t volumeModifier;
        uint8_t reserved;
        uint8_t loopBase;

        // VGM 1.51
        uint8_t loopModifier;

        // VGM 1.61
        uint32_t gameBoyDMGClock;
        uint32_t nesAPUClock;
        uint32_t multiPCMClock;
        uint32_t upd7759Clock;
        uint32_t okim6258Clock;
        uint8_t okim6258Flags;
        uint8_t k054539Flags;
        uint8_t c140ChipType;
        uint8_t reserved0;
        uint32_t okim6295Clock;
        uint32_t k051649Clock;
        uint32_t k054539Clock;
        uint32_t huC6280Clock;
        uint32_t c140Clock;
        uint32_t k053260Clock;
        uint32_t pokeyClock;
        uint32_t qsoundClock;

        // VGM 1.71
        uint32_t scspClock;

        // VGM 1.70
        uint32_t extraHeaderOffset;

        // VGM 1.71
        uint32_t wonderSwanClock;
        uint32_t vsuClock;
        uint32_t saa1099Clock;
        uint32_t es5503Clock;
        uint32_t es5505Clock;
        uint8_t es5503OutputChannels;
        uint8_t es5505OutputChannels;
        uint8_t c352ClockDivider;
        uint8_t reserved1;
        uint32_t x1010Clock;
        uint32_t c352Clock;
        uint32_t ga20Clock;

        // Reserved
        uint8_t reserved2[256 - 0xe4];
    };

    Header* header;
    uint8_t* commands;
    uint32_t currentCommand;
};
#pragma pack(pop)

static_assert(sizeof(VGM::Header) == 256);

VGM* Load(const char* filename);
void Destroy(VGM* vgm);
void Update(VGM* vgm);
}
