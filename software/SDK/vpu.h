#pragma once

#include <inttypes.h>

#define VPUCMD_SETVPAGE				0x00000000
#define VPUCMD_SETPAL				0x00000001
#define VPUCMD_SETVMODE				0x00000002
#define VPUCMD_CTLREGSEL			0x00000003
#define VPUCMD_CTLREGSET			0x00000004
#define VPUCMD_CTLREGCLR			0x00000005

#define CONSOLEDIMGRAY 0x00
#define CONSOLEDIMBLUE 0x01
#define CONSOLEDIMGREEN 0x02
#define CONSOLEDIMCYAN 0x03
#define CONSOLEDIMRED 0x04
#define CONSOLEDIMMAGENTA 0x05
#define CONSOLEDIMYELLOW 0x06
#define CONSOLEDIMWHITE 0x07

#define CONSOLEGRAY 0x08
#define CONSOLEBLUE 0x09
#define CONSOLEGREEN 0x0A
#define CONSOLECYAN 0x0B
#define CONSOLERED 0x0C
#define CONSOLEMAGENTA 0x0D
#define CONSOLEYELLOW 0x0E
#define CONSOLEWHITE 0x0F

#define CONSOLEDEFAULTFG CONSOLEWHITE
#define CONSOLEDEFAULTBG CONSOLEBLUE

// Hardware format is: 12bit R:G:B
#define MAKECOLORRGB12(_r, _g, _b) ((((_r&0xF)<<8) | (_g&0xF)<<4) | (_b&0xF))

enum EVideoMode
{
	EVM_320_Wide,
	EVM_640_Wide,
	EVM_Count
};

enum EColorMode
{
	ECM_8bit_Indexed,
	ECM_16bit_RGB,
	ECM_Count
};

enum EVideoScanoutEnable
{
	EVS_Disable,
	EVS_Enable,
	EVS_Count
};

struct EVideoContext
{
	enum EVideoMode m_vmode;
	enum EColorMode m_cmode;
	enum EVideoScanoutEnable m_scanEnable;
	uint32_t m_strideInWords;
	uint32_t m_scanoutAddressCacheAligned;
	uint32_t m_cpuWriteAddressCacheAligned;
	uint32_t m_graphicsWidth, m_graphicsHeight;
	uint16_t m_consoleWidth, m_consoleHeight;
	uint16_t m_cursorX, m_cursorY;
	uint16_t m_consoleUpdated;
	uint16_t m_caretX;
	uint16_t m_caretY;
	uint8_t m_consoleColor;
	uint8_t m_caretBlink;
};

struct EVideoSwapContext
{
	// Swap cycle counter
	uint32_t cycle;
	// Current read and write pages based on cycle
	uint8_t *readpage;
	uint8_t *writepage;
	// Frame buffers to toggle between
	uint8_t *framebufferA;
	uint8_t *framebufferB;
};

// Utilities
uint8_t *VPUAllocateBuffer(const uint32_t _size);
void VPUGetDimensions(const enum EVideoMode _mode, uint32_t *_width, uint32_t *_height);

// VPU side
void VPUSetDefaultPalette(struct EVideoContext *_context);
void VPUSetVMode(struct EVideoContext *_context, const enum EVideoScanoutEnable _scanEnable);
void VPUSetScanoutAddress(struct EVideoContext *_context, const uint32_t _scanOutAddress64ByteAligned);
void VPUSetWriteAddress(struct EVideoContext *_context, const uint32_t _cpuWriteAddress64ByteAligned);
void VPUSetPal(const uint8_t _paletteIndex, const uint32_t _red, const uint32_t _green, const uint32_t _blue);
uint32_t VPUReadVBlankCounter();
uint32_t VPUGetScanline();
void VPUSwapPages(struct EVideoContext* _vx, struct EVideoSwapContext *_sc);
void VPUWaitVSync();
void VPUPrintString(struct EVideoContext *_context, const uint8_t _foregroundIndex, const uint8_t _backgroundIndex, const uint16_t _x, const uint16_t _y, const char *_message, int _length);

// Console
void VPUConsoleSetColors(struct EVideoContext *_context, const uint8_t _foregroundIndex, const uint8_t _backgroundIndex);
void VPUConsoleClear(struct EVideoContext *_context);
void VPUConsoleSetCursor(struct EVideoContext *_context, const uint16_t _x, const uint16_t _y);
void VPUConsolePrint(struct EVideoContext *_context, const char *_message, int _length);
void VPUConsoleResolve(struct EVideoContext *_context);
void VPUConsoleSetCaret(struct EVideoContext *_context, const uint16_t _x, const uint16_t _y, const uint16_t _blink);
void VPUConsoleClearLine(struct EVideoContext *_context, const uint16_t _y);
int VPUConsoleFillLine(struct EVideoContext *_context, const char _character);
void VPUConsoleScrollUp(struct EVideoContext *_context);

// Software emulated
void VPUClear(struct EVideoContext *_context, const uint32_t _colorWord);

// OS graphics context
struct EVideoContext *VPUGetKernelGfxContext();
