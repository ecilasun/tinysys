MEMORY
{
    ROM   (xr)  : ORIGIN = 0x00100000, LENGTH = 0x100000	/* 1024k */
    PSRAM (xrw) : ORIGIN = 0x01000000, LENGTH = 0x800000	/* 8192k */
    BRAM  (xrw) : ORIGIN = 0x00000010, LENGTH = 0x03f0		/*  ~ 1k */
}
ENTRY(_start)
SECTIONS {
    __stacktop = ORIGIN(PSRAM) + LENGTH(PSRAM);
    .text :
    {
        . = ALIGN(4);
        *(.text.start)
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.srodata)
        *(.srodata*)
        . = ALIGN(4);
        _etext = .;
        _sidata = _etext;
    } >ROM
    .data : AT ( _sidata )
    {
        . = ALIGN(4);
        _sdata = .;
        _ram_start = .;
        . = ALIGN(4);
        *(.data)
        *(.data*)
        *(.sdata)
        *(.sdata*)
        . = ALIGN(4);
        _edata = .;
    } >PSRAM
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(.sbss)
        *(.sbss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >PSRAM
    .heap :
    {
        . = ALIGN(4);
        _heap_start = .;
    } >PSRAM
}
