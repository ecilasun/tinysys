# Video Processing Unit

The video processing unit is formed of internal circuity that fills up a scanline cache and controls output clock generation, tied to an external video output chip (Lattice, SII164CTG64, DVI 1.0)

The video output chip itself supports 24bit video (8 bits per component), however tinysys only implements a 16 bit bus to the device.

The color formats supported by the VPU are either of 16 bit RGB values per pixel, or 8 bit colors from a 256 entry color palette.

The device can generate video at 320x240 or 640x480 pixels in size. However in reality, the true video output is always running at 640x480, therefore the 320x240 mode is achieved by pixel doubling. This, combined with carefully timed video mode switching, can allow for some interesting effects such as scanning parts of the display at different resolutions, simultaneously.

### Utilities
`uint8_t *VPUAllocateBuffer(const uint32_t _size)`

This function will allocate an adequately aligned block of memory to be used as a backbuffer or a scan-out buffer by the VPU. The allocated buffers can be set in the current video context and be swapped on vsync signal to implement double-buffering effects.

---

`void VPUGetDimensions(const enum EVideoMode _mode, uint32_t *_width, uint32_t *_height)`

This function fills in the with and height values with those of a given video mode provided by `_mode` parameter.

---

`void VPUClear(struct EVideoContext *_context, const uint32_t _colorWord)`

This function writes the `_colorWord` to current write page (the backbuffer) to fill it to a desired color. Each byte of the color word can be different up to 4 colors or 2 colors for 16bit modes to generate vertical strips if desired.

### Color palette
`void VPUSetDefaultPalette(struct EVideoContext *_context)`

This function will reset the color palette to the default VGA color palette, which is the same palette used by the OS on startup.

---

`void VPUSetPal(const uint8_t _paletteIndex, const uint32_t _red, const uint32_t _green, const uint32_t _blue)`

This function will set a certain color in the 256 entry color palette to given color values.

Any changes to the color paltte will be reset back to the original VGA colors when the current user task stops running.

### Video output control
`void VPUSetVMode(struct EVideoContext *_context, const enum EVideoScanoutEnable _scanEnable)`

This function will set the current video output mode to one that is controlled by the following values:
```
EVM_320_Wide: 320x240 pixels
EVM_640_Wide: 640x480 pixels
ECM_8bit_Indexed: 8 bit paletted scanout buffer
ECM_16bit_RGB: 16 bit RGB scanout buffer
```

---

`void VPUSetScanoutAddress(struct EVideoContext *_context, const uint32_t _scanOutAddress64ByteAligned)`

This function will set the video scan out buffer address, which contains the pixels that will be sent out to the DVI chip, and subsequently to a display monitor.

The address should be 64 byte aligned, and a suitable buffer can be obtained by using the `VPUAllocateBuffer()` function.

The scan out address can be manipulated on the fly, generating interesting visual effects if timed properly. However it is advised to use `VPUSwapPages()` function to avoid any mistakes to switch between the CPU write and VPU scanout pages.

One thing to note here is that all memory writes to the scan out buffer should be completed for a coherent image to be visible on the display, therefore make sure to call the following function when all writes to the backbuffer are complete before calling `VPUSwapPages()`:

```
CFLUSH_D_L1();
```

`uint32_t VPUReadVBlankCounter();`

This non-blocking function immediately returns the current vertical blank counter value. This counter is incremented every time the video scan reaches the last row of the visible screen.

The normal usage case is to read this value once, do some work, then come back to check if the vertical blank point has changed, and subsequently call `VPUSwapPages()` to show what was drawn into the back buffer.

If the synchonization point is well known (typically end of frame for a game) then it is more suitable to use the blocking `VPUWaitVSync()` function instead.

---

`uint32_t VPUGetScanline()`

This function will return the scanline from the VPU at the time this function was called. It is not ordinarily useful from regular code, except if you are precise with your timing.

The returned value has a range of 0 to 1023, however the hardware returns only the valid range, which is from 0 to 525.

---

`void VPUSwapPages(struct EVideoContext* _vx, struct EVideoSwapContext *_sc)`

This function will swap the pointers for video scan output and CPU accessible backbuffer. If the CPU always reads the backbuffer address from the video context, this function will ensure the correct value is visible to both the CPU and the VPU.

If the caller does not wait for vblank, the swap occurs immediately, typically in the middle of a scanline, and will most likely cause a rolling / tearing artifact. It is adviseable to have a vsync wait function before the swap occurs to avoid this.

---

`void VPUWaitVSync()`

This blocking function will stall the CPU until a vertical blank event occurs. A vertical blank event is tracked by detecting the changes in the vertical blank counter in a tight loop.

If you do not wish to block while waiting, please see `VPUReadVBlankCounter()` instead. However that is not a recommended way to wait for vblank, as having a split read and swap will most likely cause undesired tearing effects.

### Video backbuffer
`void VPUSetWriteAddress(struct EVideoContext *_context, const uint32_t _cpuWriteAddress64ByteAligned)`

This function will set the CPU writeable address (i.e. the backbuffer) so that the CPU can draw graphics or blit some images to be later shown by calling the `VPUSwapPages()` function.

### Text functions
`void VPUPrintString(struct EVideoContext *_context, const uint8_t _foregroundIndex, const uint8_t _backgroundIndex, const uint16_t _x, const uint16_t _y, const char *_message, int _length)`

### Color encoding for 16bit mode

The 16 bit color mode in fact uses 12 bit colors packed as follows:

```
#define MAKECOLORRGB12(_r, _g, _b) ((((_r&0xF)<<8) | (_b&0xF)<<4) | (_g&0xF))
```

The lowest 4 bits are green, followed by 4 bits of blue, and 4 bits of red.

The remaining 4 bits are reserved for future and should always be set to zeros.

### Back to [SDK Documentation](README.md)