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

// Task - internals
struct STaskContext *_task_get_context(uint32_t _hartid);
void _task_init_context(uint32_t _hartid);
int _task_add(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength);
uint32_t _task_switch_to_next(struct STaskContext *_ctx);
void _task_exit_task_with_id(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal);
void _task_exit_current_task(struct STaskContext *_ctx);
uint64_t _task_yield();
