/* -*- Makefile -*- */
ENTRY(_real_start)


SECTIONS
{
  /* No program code/data before _stext/_prog_img_start! */

  /* Read-only sections, merged into text segment. The start address of
   * the text segment is : */

  .text :
  {
    _stext = .;
    *(.text .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    KEEP (*(.init))
    KEEP (*(.fini))
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    . = ALIGN(0x1000);
    _etext = .;
    PROVIDE (etext = .); /* don't fail if user defines this symbol */
  } = 0x9090

  /* Linux: exception section for uaccess mechanism */
  __ex_table :
  {
    *(__ex_table)
  } 

  .eh_frame_hdr : { *(.eh_frame_hdr) }

  /* exception frames for C++ */
  .eh_frame : 
  {
    KEEP (*(.eh_frame)) 
  }


  /* ensure that data starts at a new L4 page */
  . = ALIGN(0x1000);
  .data :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))    
  }

  .sdata :
  {
    PROVIDE(_SDA_BASE_ = 32768);
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }
  .sbss :
  {
    *(.sbss .sbss.* .gnu.linkonce.b.*)
    *(.scommon)
  }

  .ctors :
  {
    /*KEEP (*(.mark_beg_ctors))*/
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    /*
    KEEP (*(.mark_end_ctors))

    KEEP (*(.mark_beg_c_ctors))
    KEEP (*(SORT(.c_ctors.?)))
    KEEP (*(SORT(.c_ctors.??)))
    KEEP (*(SORT(.c_ctors.???)))
    KEEP (*(SORT(.c_ctors.????)))
    KEEP (*(SORT(.c_ctors.?????)))
    KEEP (*(.c_ctors))
    KEEP (*(.mark_end_c_ctors))
    */
  }
  .dtors :
  {
  /*
    KEEP (*(.mark_beg_dtors))*/
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*
    KEEP (*(.mark_end_dtors))

    KEEP (*(.mark_beg_c_dtors))
    KEEP (*(SORT(.c_dtors.?)))
    KEEP (*(SORT(.c_dtors.??)))
    KEEP (*(SORT(.c_dtors.???)))
    KEEP (*(SORT(.c_dtors.????)))
    KEEP (*(SORT(.c_dtors.?????)))
    KEEP (*(.c_dtors))
    KEEP (*(.mark_end_c_dtors))*/
  }
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
     KEEP (*(.init_array))
     PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }
  
  .gcc_except_table : { KEEP(*(.gcc_except_table)) }
  .dynamic :  { *(.dynamic) }

  __alt_instructions = .;
  .altinstructions : { *(.altinstructions) }
  __alt_instructions_end = .;
  .altinstr_replacement : { *(.altinstr_replacement) }

  .got : { *(.got.plt) *(.got) } 

  _edata = .;
  PROVIDE (edata = .); /* don't fail if user defines this symbol */

  __bss_start = .;
  .bss :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   _end = .;
   PROVIDE (end = .); /* don't fail if user defines this symbol */
   *(.rospace)
  }

  /* Moved here to ensure that these sections are located _after_ the text
   * section. In the other case we would get program sections with a virtual
   * address of 0 */
  .hash    : { *(.hash)    }
  .dynsym  : { *(.dynsym)  }
  .dynstr  : { *(.dynstr)  }
  .rel.dyn : { *(.rel.dyn) }
  .rel.bss : { *(.rel.bss) }
  .rel.plt : { *(.rel.plt) }

  /* drop the following sections since we don't need them for DROPS */
  /DISCARD/ : {
    *(.interp)
    *(.comment)
    *(.note)
    *(.note.*)
  }
}

