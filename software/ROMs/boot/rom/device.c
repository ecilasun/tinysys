#include "device.h"

void DeviceDefaultState(int _bootTime)
{
	// Stop any pending horizontal blanking interrupt
	VPUDisableHBlankInterrupt();

	// Stop output
	APUSetSampleRate(ASR_Halt);

	// Turn off LEDs
	LEDSetState(0x0);

	// TODO: Wait for any pending raster ops to complete

	// Wait for any pending DMA to complete
	DMAWait(CPUIncoherent);

	// Set up console view
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	VPUSetWriteAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	VPUSetScanoutAddress(kernelgfx, CONSOLE_FRAMEBUFFER_START);
	VPUSetDefaultPalette(kernelgfx);
	kernelgfx->m_vmode = EVM_640_Wide;
	kernelgfx->m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(kernelgfx, EVS_Enable);

	// Preserve contents of screen for non-boot time
	if (_bootTime)
	{
		VPUConsoleSetColors(kernelgfx, CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
		VPUConsoleClear(kernelgfx);
	}

	// Clear screen to overlay loader color
	if (_bootTime == 2)
		VPUClear(kernelgfx, 0x09090909);
}