#include "device.h"
#include "uart.h"

extern void UserMain();

// We drop here for boot time or after an executable exits
void DeviceDefaultState(int _bootTime)
{
	// Re-enable UART interrupts if disabled earlier
	UARTInterceptSetState(0);

	// Always reset CPU#1
	E32SetupCPU(1, UserMain);
	E32ResetCPU(1);

	// Stop output
	APUSetSampleRate(ASR_Halt);

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
	else
		LEDSetState(0x0); // Turn off all LEDs

	// For overlay loader, blank the entire screen to red (CONSOLERED == 0x0C)
	if (_bootTime == 2)
		VPUClear(kernelgfx, 0x0C0C0C0C);
}
