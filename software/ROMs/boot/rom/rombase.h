#pragma once

#include "basesystem.h"
#include "core.h"
#include "encoding.h"
#include "task.h"
#include "ff.h"
#include "leds.h"
#include "elf.h"
#include "vpu.h"

#include <inttypes.h>
#include <stdbool.h>

struct STaskContext *GetTaskContext(uint32_t _hartid);
void InitializeTaskContext(uint32_t _hartid);

void InstallISR(uint32_t _hartid, bool _allowMachineHwInt, bool _allowMachineSwInt);

uint32_t MountDrive();
void UnmountDrive();
void ListFiles(const char *path);
uint32_t LoadExecutable(const char *filename, int _relocOffset, const bool reportError);
struct EVideoContext *GetKernelGfxContext();

int kprintf(const char *fmt, ...);
void kgetcursor(int *_x, int *_y);
void ksetcursor(const int _x, const int _y);
