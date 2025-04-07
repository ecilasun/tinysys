# Audio Processing Unit

The APU consists of a commmand FIFO an internal hardware to stream out audio at a chosen rate, and an external audio chip (Cirrus Logic, CS4344-CZZR, stereo D/A converter) that listens to sound data over an I2S bus.

The internal audio device can drive the DAC using stereo 16bit samples, clocked at 44Khz, 22Khz or 11KHz sample rates.

APU has an internal memory region which is split into two pages. One of these pages is called the 'write page' and can be written by the CPU using the `APUStartDMA()` function. The other page is the 'read page' and is dedicated to feeding the audio chip and can't be touched by the CPU while playback is in progress.

When the APU reaches the end of the current read page, it will toggle the two pages and flip a counter bit. This mechanism can be used to wait for audio playback to complete to queue more samples.

See the playing audio section below for details on how this system works.

# Command FIFO

The command fifo accepts the following command codes as 32bit words, followed by zero or more words containing data as required.

To write a command, the SDK functions simply write the command word and the parameters to the FIFO at memory address `0x80070000`

However, to be compatible with future hardware, it is advised that the writes go through the SDK functions listed below.

### Allocating CPU side audio buffers
`uint8_t *APUAllocateBuffer(const uint32_t _size)`

This function will allocate a 16 byte aligned region of `_size` bytes of memory that can be used for your mixer output.

To copy this data to the write page of the audio device, call the `APUStartDMA` function shown below.

### Set audio buffer size
`void APUSetBufferSize(uint32_t audioBufferSize)`

This command will set the source sample buffer size (and the output size) of audio data.

### Start audio data DMA
`void APUStartDMA(uint32_t audioBufferAddress16byteAligned)`

This command will start copying new bytes to the write page of the audio device, while the playback page is still outputting samples.

The `audioBufferAddress16byteAligned` parameter should point at 16 bit stereo audio data located at a 16 byte aligned address.

### Set audio output rate
`void APUSetSampleRate(enum EAPUSampleRate sampleRate)`

The `sampleRate` parameter can be one of the following 2 bit binary values (packed into lowest two bits of parameter word):
```
ASR_44_100_Hz: Set audio output rate to 44.1KHz
ASR_22_050_Hz: Set audio output rate to 22.05KHz
ASR_11_025_Hz: Set audio output rate to 11.025KHz
ASR_Halt: Stop audio output
```

### Tracking playback state
`uint32_t APUFrame()`

This function will return either 0 or 1, depending on which page is now available for writes by the CPU.

### Playing audio

To play audio, first we need to initialize the audio system by allocating some mixer memory and deciding on the audio playback rate.

```
#define BUFFER_SAMPLES 512
apubuffer = (short*)APUAllocateBuffer(BUFFER_SAMPLES*NUM_CHANNELS*sizeof(short));
APUSetBufferSize(BUFFER_SAMPLES);
APUSetSampleRate(ASR_22_050_Hz);
```

Next step is to read which page is our write page by calling the following sequence of instructions:
```
uint32_t prevframe = APUFrame();
```

Now we can sit in a loop and push new audio data every time `prevframe` value mismatches the new `APUFrame()` value:
```
do{
	// TODO: fill apubuffer with some stereo 16 bit audio data
	CFLUSH_D_L1(); // Make sure the CPU writes can be seen by the DMA device by flushing them to memory
	APUStartDMA((uint32_t)apubuffer); // Kick new data to the APU

	// Wait for the sample playback to finish.
	// This means the APU has reached the end of the read page and has set the other page as the new read page.
	uint32_t currframe;
	do
	{
		currframe = APUFrame();
	} while (currframe == prevframe);
	prevframe = currframe;

	// See task.md about this command
	TaskYield();
} while(1);
```

For a full sample that can play mod/xm files, please see the sample code in 'samples/mod' directory

### Back to [SDK Documentation](README.md)