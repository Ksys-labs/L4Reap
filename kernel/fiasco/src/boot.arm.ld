
/*load_address = 0xc0000000;*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
              "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS {
/*  . = load_address;*/

  /DISCARD/ : {			/* Exit code and data		*/
    *(.exitcall.exit)
  }

  .text : {
    crt0.o(.text)
    *(.init)
    *(.text .text.* .gnu.linkonce.t.*)
    *(.glue_7t) *(.glue_7)
    *(.got*)
    *(.fini)

    . = ALIGN(0x40);
    *(.rodata .rodata.* .gnu.linkonce.r.* .rodata1)
    . = ALIGN(4);

    *(.data) 
    *(.gnu.linkonce.d*)
    *(.anno)

    /* Constructor and destructor lists, ordered by priority.  The lists
       are traversed from end to start, therefore the *_END__ symbols
       precede the *_LIST__ symbols. */

    __CTOR_END__ = .;
    CONSTRUCTORS
    KEEP(*(.ctors))
    KEEP(*(SORT(.ctors.*)))
    LONG( (__CTOR_LIST__ - __CTOR_END__) / 4 - 1 ) /* number of constructors */
    __CTOR_LIST__ = .;

    __DTOR_END__ = .;    
    KEEP( *(SORT(.dtors.*)))
    KEEP( *(.dtors))
    LONG( (__DTOR_LIST__ - __DTOR_END__) / 4 - 1) /* number of destructors */
    __DTOR_LIST__ = .;

    *(.init.data)
/*
  }

  . = ALIGN(0x1000);
  .bss : {
*/

    PROVIDE( __bss_start = .);
    *(.bss COMMON)
  }

  /DISCARD/ :{
    *(.stab .stabstr .stab.excl .stab.exclstr)
    *(.stab.index .stab.indexstr .comment)
  }


}
