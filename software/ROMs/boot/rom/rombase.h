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

// Task related
void ClearTaskMemory();
struct STaskContext *GetTaskContext(uint32_t _hartid);
void InitializeTaskContext(uint32_t _hartid);
void InstallISR(uint32_t _hartid, bool _allowMachineHwInt, bool _allowMachineSwInt);

// Disk access and ELF loader
void SetWorkDir(const char *_workdir);
const char* GetWorkDir();
uint32_t MountDrive();
void UnmountDrive();
void ListFiles(const char *path);
uint32_t LoadExecutable(const char *filename, int _relocOffset, const bool reportError);

// Kernel print
void ksetcolor(int8_t fg, int8_t bg);
int kprintf(const char *fmt, ...);
void kgetcursor(int *_x, int *_y);
void ksetcursor(const int _x, const int _y);

// Path helpers
#define PATH_MAX 64
char *krealpath(const char *path, char resolved[PATH_MAX]);