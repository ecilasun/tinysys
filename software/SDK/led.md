# Debug LEDs

Debug LEDs are useful to reflect status when there is no chance to modify onscreen bitmaps, use the UART or generate audio. They can be useful visual clues to aid in debugging your software.

### Debug LED status
`uint32_t LEDGetState()`

This function will get the current state of the debug LEDs. Only the lower 4 bits have meaningful values, and each of these low bits correspond to a single physical LED on the device.

`void LEDSetState(const uint32_t state)`

This function will control the state of the debug LEDs on the device. Setting one of the lowest 4 bits will turn the corresponding LED on.

Upon termination of a process, all LEDs will revert back to off state.

### Using the debug LEDs

For a full sample that blinks the debug LEDs, see the sample code in 'samples/led' directory

### Back to [SDK Documentation](README.md)