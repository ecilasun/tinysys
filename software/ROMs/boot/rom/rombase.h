#pragma once

#include "basesystem.h"
#include "core.h"
#include "encoding.h"
#include "task.h"
#include "fat32/ff.h"
#include "leds.h"
#include "elf.h"
#include "gpu.h"

#include <inttypes.h>
#include <stdbool.h>

struct STaskContext *CreateTaskContext();
struct STaskContext *GetTaskContext();
void TaskDebugMode(uint32_t _mode);
void InstallISR();
uint32_t MountDrive();
void UnmountDrive();
void ListFiles(const char *path);
uint32_t LoadExecutable(const char *filename, int _relocOffset, const bool reportError);
struct EVideoContext *GetKernelGfxContext();
int kprintf(const char *fmt, ...);
void kgetcursor(int *_x, int *_y);
void ksetcursor(const int _x, const int _y);
