# Direct Memory Access Unit

The DMA unit consists of a commmand FIFO and built in hardware that can transfer up to 256 units of 128bit values, which makes a total of up to 4096 bytes per memory tranfer operation.

Optionally, the DMA can skip writing bytes that contain zeros, which is called the masked DMA mode. This mode is most commonly useful for graphics overlays.

See the transferring memory section below for details on how this system works.

# Command FIFO

The command fifo accepts the following command codes as 32bit words, followed by zero or more words containing data as required.

To write a command, the SDK functions simply write the command word and the parameters to the FIFO at memory address `0x80050000`

However, to be compatible with future hardware, it is advised that the writes go through the SDK functions listed below.

### Copy a 4K block
`void DMACopy4K(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned);`

This function moves a 4Kbyte block of memory from a 16 byte aligned memory address to another 16 byte aligned memory address. It is most commonly used to blit an offscreen image to the scan-out buffer of the video controller, or to quickly clear large blocks of memory to the values in a source block.

`void DMACopyAutoByteMask4K(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned);`

This function behaves exactly the same way as `DMACopy4K`, however it will not write any zeros seen in the source byte stream. It can be used to overlay data onto a memory location without destroying masked areas, denoted by zero.

### Copy an arbitrary block size
`void DMACopy(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned, const uint8_t _blockCountInMultiplesOf16bytes);`

This function will copy given number of 16 byte blocks from a 16 byte aligned source address to a 16 byte aligned target address. The function will return immediately. Caller can then do other operations and use `DMAWait()` at a later point to wait for DMA operations to complete or periodically poll using the non-blocking `DMAPending()` function.

`void DMACopyAutoByteMask(const uint32_t _sourceAddress16ByteAligned, const uint32_t _targetAddress16ByteAligned, const uint8_t _blockCountInMultiplesOf16bytes);`

This function behaves exactly the same way as `DMACopy`, however it will not write any zeros seen in the source byte stream. It can be used to overlay data onto a memory location without destroying masked areas, denoted by zero.

### Insert DMA synchronization marker
`void DMATag(const uint32_t _tag);`

This function inserts a waitable noop command into the DMA FIFO.

The `_tag` value is not used for now.

### Wait for DMA transfer completion
`void DMAWait(enum ECPUCoherency _cpucoherency);`

This function will wait for all current pending DMA operations to fully complete. Before it returns, it will make sure the CPU can see DMA writes if `_cpucoherency` flag is set to `CPUCoherent`, or do nothing if the value is `CPUIncoherent`

`uint32_t DMAPending();`

This function will check to see if any DMA operations are still pending or in progress, and will return immediately.

### DMA - CPU synchronization

The key to correct memory transfer is to make sure the data CPU cache is flushed to memory by using the following command before a copy:

```
CFLUSH_D_L1;
```

If the CPU data cache is not written back to memory, the DMA device will see a stale copy of memory contents which won't reflect the latest CPU side memory write operations.

Likewise, the following command will ensure that the DMA writes can be seen by the CPU by invalidating the data cache contents:

```
CDISCARD_D_L1;
```

Since it's risky to do this by hand, it is advised to use the `DMAWait()` function and pass it the `CPUCoherent` flag instead. This command will then ensure the CPU sees DMA writes after completion.

For a full sample that uses plain and masked DMA, please see the sample code in 'samples/dma' directory

### Back to [SDK Documentation](README.md)