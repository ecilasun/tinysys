#include "device.h"

// We drop here for boot time or after an executable exits
void DeviceDefaultState(int _bootTime)
{
	// Stop any pending horizontal blanking interrupt
	VPUDisableHBlankInterrupt();

	// Stop output
	APUSetSampleRate(ASR_Halt);

	// Turn off LEDs
	LEDSetState(0x0);

	// Wait for any pending DMA to complete
	DMAWait(CPUIncoherent);

	// Set up video mode and default color tables
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	VPUSetWriteAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	VPUSetScanoutAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	VPUSetDefaultPalette(kernelgfx);
	kernelgfx->m_vmode = EVM_640_Wide;
	kernelgfx->m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(kernelgfx, EVS_Enable);
	VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);

	// Clear console buffer on initial boot, but not for when an executable exits
	if (_bootTime)
		VPUConsoleClear(kernelgfx);

	// For overlay loader, blank the entire screen to red (CONSOLERED == 0x0C)
	if (_bootTime == 2)
		VPUClear(kernelgfx, 0x0C0C0C0C);
}
