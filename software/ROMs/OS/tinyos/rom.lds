/* With help from https://github.com/pulp-platform/pulp-riscv-gnu-toolchain/blob/master/riscv.ld */

OUTPUT_ARCH(riscv)
OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")

GROUP( -lc -lgloss -lgcc )

MEMORY{
   ROMRAM (xwr) : ORIGIN = 0x0FFE0000, LENGTH = 0xFFFF
}

ENTRY(_start)

SECTIONS
{
  /* Default stack size is 256 bytes unless overriden by linker by defining __stack_size, see Makefile */
  __stack_size = DEFINED(__stack_size) ? __stack_size : 0x100;
  PROVIDE(__stack_size$ = __stack_size);

  /*--------------------------------------------------------------------*/
  /* Code and read-only segment                                         */
  /*--------------------------------------------------------------------*/

  /* boot code */
  .text :
  {
	. = 0x00000000;
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
  }

  /* Begining of code and text segment */
  /* . = 0x00010000; */
  _ftext = .;
  PROVIDE( eprol = . );

  /* init: Code to execute before main (called by crt0.S) */
  .init : 
  {
    KEEP( *(.init) )
  }

  /* fini: Code to execute after main (called by crt0.S) */
  .fini : 
  {
    KEEP( *(.fini) )
  }

  /* rodata: Read-only data */
  .rodata : 
  {
    *(.rdata)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
  }

  /* End of code and read-only segment */
  PROVIDE( etext = . );
  _etext = .;

  /*--------------------------------------------------------------------*/
  /* Global constructor/destructor segement                             */
  /*--------------------------------------------------------------------*/

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }

  .init_array     :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array ))
    PROVIDE_HIDDEN (__init_array_end = .);
  }

  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array ))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  /*--------------------------------------------------------------------*/
  /* Other misc gcc segments (this was in idt32.ld)                     */
  /*--------------------------------------------------------------------*/
  /* I am not quite sure about these sections but it seems they are for
     C++ exception handling. I think .jcr is for "Java Class
     Registration" but it seems to end up in C++ binaries as well. */

  .eh_frame_hdr     : { *(.eh_frame_hdr)     }
  .eh_frame         : { KEEP( *(.eh_frame) ) }
  .gcc_except_table : { *(.gcc_except_table) }
  .jcr              : { KEEP (*(.jcr))       } 

  /*--------------------------------------------------------------------*/
  /* Initialized data segment                                           */
  /*--------------------------------------------------------------------*/

  /* Start of initialized data segment */
  . = ALIGN(16);
   _fdata = .;

  /* data: Writable data */
  .data : 
  {
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
  }

  /* End of initialized data segment */
  PROVIDE( edata = . );
  _edata = .;

  /* Have _gp point to middle of sdata/sbss to maximize displacement range */
  /*. = ALIGN(16);
  _gp = . + 0x800;
  PROVIDE (__global_pointer$ = _gp);*/

  /* Writable small data segment */
  .sdata : 
  {
    *(.sdata)
    *(.sdata.*)
    *(.srodata.*)
    *(.gnu.linkonce.s.*)
  }

  /*--------------------------------------------------------------------*/
  /* Uninitialized data segment                                         */
  /*--------------------------------------------------------------------*/

  /* Start of uninitialized data segment */
  . = ALIGN(8);
  _fbss = .;

  /* Writable uninitialized small data segment */
  .sbss : 
  {
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
  }

  /* bss: Uninitialized writeable data section */
  . = .;
  _bss_start = .;
  .bss : 
  {
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  }
  PROVIDE ( __BSS_END__$ = . );
  _bss_end = .;

   /*.stack :
   {
    . = ALIGN(4);
    _stack_end = .;
    . += _STACK_SIZE;
    __stack_top = .;
    __stack = __stack_top;
  }*/

  /* End of uninitialized data segment (used by syscalls.c for heap) */
  PROVIDE( end = . );
  _end = ALIGN(8);

}
