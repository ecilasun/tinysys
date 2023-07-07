#pragma once

#include "core.h"
#include "gpu.h"

#define CONSOLE_COLUMNS 80
#define CONSOLE_ROWS 60

void ConsoleInit();
void ConsoleSetAttr(uint8_t _textColor);
void ConsoleShutdown();
void ConsoleClear();
void ConsoleScroll();
void ConsoleSetCursor(const int x, const int y);
void ConsoleWrite(const char *outstring);
void ConsoleWriteN(const char *outstring, const uint32_t count);
void ConsoleWriteDecimal(const int32_t i);
void ConsoleWriteHex(const int32_t i);
void ConsoleWriteHexByte(const int32_t i);
void ConsoleDraw(struct EVideoContext *_context);
void ConsoleGetCursor(int *x, int *y);
void ConsoleClearRow();
void ConsoleCursorStepBack();
void ConsoleStringAtRow(char *target);
