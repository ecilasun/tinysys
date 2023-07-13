#include <stdio.h>
#include "vgmopl2.h"

#include "basesystem.h"

// Disclaimer:
// Portions of this software based on, uses code or information from the following works:
//
// https://github.com/samizzo/opl2vgmplay
// https://vgmrips.net/wiki/VGM_Specification

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Usage:\nvgmplayer filename[.vgm]\n");
        return 0;
    }

    printf("Now playing '%s'\n", argv[1]);

    VGM::VGM* vgm = VGM::Load(argv[1]);
    VGM::Update(vgm);
    VGM::Destroy(vgm);

    return 0;
}
