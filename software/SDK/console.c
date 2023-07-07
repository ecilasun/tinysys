#include <stdlib.h>
#include "basesystem.h"
#include "core.h"
#include "console.h"
#include "gpu.h"

static uint16_t *s_consoleText = NULL;
static int cursorx = 0;
static int cursory = 0;
static uint16_t currattr = 0x0200; // Green

void ConsoleInit()
{
    s_consoleText = (uint16_t*)HEAP_END_CONSOLEMEM_START;
    ConsoleClear();
}

void ConsoleSetAttr(uint8_t _textColor)
{
    currattr = _textColor<<8;
}

void ConsoleShutdown()
{
    s_consoleText = NULL;
}

void ConsoleClear()
{
    if (!s_consoleText) return;

    cursorx = 0;
    cursory = 0;
    for (int cy=0;cy<CONSOLE_ROWS;++cy)
        for (int cx=0;cx<CONSOLE_COLUMNS;++cx)
            s_consoleText[cx+cy*CONSOLE_COLUMNS] = 0x0220;
}

void ConsoleClearRow()
{
    if (!s_consoleText) return;

    // Clear the last row
    for (int cx=0;cx<CONSOLE_COLUMNS;++cx)
        s_consoleText[cx+cursory*CONSOLE_COLUMNS] = 0x0220;
}

void ConsoleSetCursor(const int x, const int y)
{
    if (!s_consoleText) return;

    cursorx = x;
    cursory = y;

    cursorx = cursorx<0 ? 0:cursorx;
    cursorx = cursorx>CONSOLE_COLUMNS-1 ? CONSOLE_COLUMNS-1:cursorx;
    cursory = cursory<0 ? 0:cursory;
    cursory = cursory>CONSOLE_ROWS-1 ? CONSOLE_ROWS-1:cursory;
}

void ConsoleScroll()
{
    if (!s_consoleText) return;

    for (int cy=0;cy<CONSOLE_ROWS-1;++cy)
        for (int cx=0;cx<CONSOLE_COLUMNS;++cx)
            s_consoleText[cx+cy*CONSOLE_COLUMNS] = s_consoleText[cx+(cy+1)*CONSOLE_COLUMNS];
    ConsoleSetCursor(cursorx, CONSOLE_ROWS-1);
    ConsoleClearRow();
}

void ConsoleGetCursor(int *x, int *y)
{
    *x = cursorx;
    *y = cursory;
}

void ConsoleCursorStepBack()
{
    if (!s_consoleText) return;

    --cursorx;
    if (cursorx<0)
    {
        cursorx = CONSOLE_COLUMNS-1;
        --cursory;
    }
    if (cursory<0)
    {
        cursorx = 0;
        cursory = 0;
    }
}

void ConsoleWrite(const char *outstring)
{
    if (!s_consoleText) return;

    char *str = (char*)outstring;
    while (*str != 0)
    {
        if (*str == '\r' || *str == '\n')
        {
            cursorx = 0;
			++cursory;
        }
        else
        {
            s_consoleText[cursorx+cursory*CONSOLE_COLUMNS] = currattr | (*str);
            ++cursorx;
        }

        if (cursorx>CONSOLE_COLUMNS-1)
        {
            cursorx=0;
            cursory++;
        }
        if (cursory>CONSOLE_ROWS-1)
        {
            cursory=CONSOLE_ROWS-1;
            cursorx=0;
            ConsoleScroll();
        }
        ++str;
    }
}

void ConsoleWriteN(const char *outstring, const uint32_t count)
{
    if (!s_consoleText) return;

    char *str = (char*)outstring;
    uint32_t i=0;
    while (*str != 0 && i!=count)
    {
        if (*str == '\r' || *str == '\n')
        {
            cursorx = 0;
			++cursory;
        }
        else
        {
            s_consoleText[cursorx+cursory*CONSOLE_COLUMNS] = currattr | (*str);
            ++cursorx;
        }

        if (cursorx>CONSOLE_COLUMNS-1)
        {
            cursorx=0;
            cursory++;
        }
        if (cursory>CONSOLE_ROWS-1)
        {
            cursory=CONSOLE_ROWS-1;
            cursorx=0;
            ConsoleScroll();
        }
        ++str;
        ++i;
    }
}

void ConsoleWriteDecimal(const int32_t i)
{
    if (!s_consoleText) return;

    const char digits[] = "0123456789";
    char msg[] = "                   ";

    int d = 1000000000;
    uint32_t enableappend = 0;
    uint32_t m = 0;
    if (i<0)
        msg[m++] = '-';
    for (int c=0;c<10;++c)
    {
        uint32_t r = abs(i/d)%10;
        // Ignore preceeding zeros
        if ((r!=0) || enableappend || d==1)
        {
            enableappend = 1; // Rest of the digits can be appended
            msg[m++] = digits[r];
        }
        d = d/10;
    }
    msg[m] = 0;

    ConsoleWrite(msg);
}

void ConsoleWriteHex(const int32_t i)
{
    if (!s_consoleText) return;

    const char hexdigits[] = "0123456789ABCDEF";
    char msg[] = "        ";
    msg[0] = hexdigits[((i>>28)%16)];
    msg[1] = hexdigits[((i>>24)%16)];
    msg[2] = hexdigits[((i>>20)%16)];
    msg[3] = hexdigits[((i>>16)%16)];
    msg[4] = hexdigits[((i>>12)%16)];
    msg[5] = hexdigits[((i>>8)%16)];
    msg[6] = hexdigits[((i>>4)%16)];
    msg[7] = hexdigits[(i%16)];
    ConsoleWrite(msg);
}

void ConsoleWriteHexByte(const int32_t i)
{
    if (!s_consoleText) return;

    const char hexdigits[] = "0123456789ABCDEF";
    char msg[] = "  ";
    msg[0] = hexdigits[((i>>4)%16)];
    msg[1] = hexdigits[(i%16)];
    ConsoleWrite(msg);
}

void ConsoleDraw(struct EVideoContext *_context)
{
    if (!s_consoleText) return;

    // Dump all to the terminal
    for (int cy=0;cy<CONSOLE_ROWS;++cy)
        for (int cx=0;cx<CONSOLE_COLUMNS;++cx)
            GPUCharOut(_context, cx, cy, s_consoleText[cx+cy*CONSOLE_COLUMNS]);
}

void ConsoleStringAtRow(char *target)
{
    if (!s_consoleText) return;

    // NOTE: Input string must be >CONSOLE_COLUMNS bytes long to accomodate the null terminator
    int cx;
    for (cx=0; cx<cursorx; ++cx)
        target[cx] = s_consoleText[cursory*CONSOLE_COLUMNS+cx]&0xFF;
    target[cx] = 0;
}
