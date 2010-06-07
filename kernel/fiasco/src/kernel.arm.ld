/* -*- c -*- */

/* this file defines the construction of the kernel's virtual address
   space */

_tcbs_1            = 0xd0000000;	/* assumption: 256MB-aligned */
/*_unused1_1         = _mappings_end_1;	/* assumption: 4MB-aligned */
_unused2_1         = 0xea400000;	/* assumption: 4MB-aligned */
_unused3_1         = 0xea800000;	/* assumption: 4MB-aligned */
_service           = 0xeac00000;	/* assumption: 4MB-aligned */
/* 0xeb000000 - 0xee000000 unused */
_ipc_window0_1     = 0xee000000;	/* assumption: 8MB-aligned */
_ipc_window1_1     = 0xee800000;	/* assumption: 8MB-aligned */
/* 0xef000000 - 0xef800000 unused */
_kstatic1_1        = 0xef800000;	/* assumption: 4MB-aligned */
_iobitmap_1        = 0xefc00000;	/* assumption: 4MB-aligned */
_unused4_io_1      = 0xefc80000;        /* assumption: 2nd level field in 
				           page table for IO bitmap */
_physmem_1         = 0x0000000;	        /* assumption: 256MB-aligned */
_kernel_phys_start = _physmem_1;
_kernel_phys_end   = 0xffffffff;


virt_address       = 0xf0000000;
phys_offset        = virt_address - kernel_load_addr;


OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
              "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS {


  /DISCARD/ : {			/* Exit code and data		*/
		*(.exitcall.exit)
	      }
	      
  . = kernel_load_addr + 0x1000;
	      
  .text : {
    bootstrap*(.init)
    bootstrap*(.text .text.* .gnu.linkonce.t.*)
    bootstrap*(.glue_7t) *(.glue_7)
    bootstrap*(.got .got.*)
    bootstrap*(.fini)
    
    bootstrap*(.rodata .rodata.* .gnu.linkonce.r.* .rodata1)

    . = ALIGN(0x40);
    
    bootstrap*(.data .data.* .gnu.linkonce.d.*)
    bootstrap*(.anno)

    bootstrap*(.data) 
    bootstrap*(.gnu.linkonce.d*)
  }

  PROVIDE(bootstrap_bss_start = .);
  .bss_bootstrap : {
    bootstrap*(.bss .bss.* COMMON .gnu.linkonce.b.*)
  }
  PROVIDE(bootstrap_bss_end = .);

  . = ALIGN(4K);
  end_of_loader = .;

  . = end_of_loader - kernel_load_addr + virt_address;

  .kip : AT (end_of_loader) {
    _kernel_image_start = .;
    *(.kernel_info_page)
    _initkip_start = .;
    KEEP(*(.initkip.version))
    KEEP(*(.initkip.features))
    KEEP(*(.initkip.features.fini))
    _initkip_end = .;
    . = ALIGN(4K);
  }

  .mp_tramp : AT (ADDR(.mp_tramp) - phys_offset) {
    KEEP(*(.mp_tramp))
  }

  .text_kernel : AT (ADDR(.text_kernel) - phys_offset) {
    crt0.o(.text)
    *(.init)
    *(.text .text.* .gnu.linkonce.t.*)
    *(.glue_7t) *(.glue_7)
    *(.got .got.*)
    *(.fini)

    *(.text.jdb)
     
    PROVIDE (_ecode = .);

    *(.rodata .rodata.* .gnu.linkonce.r.* .rodata1)

    . = ALIGN(4);
    PROVIDE (_log_table = .);
    /*
     * The section must start with ".debug" because else the linker
     * complains about references to discarded sections
     */
    KEEP(*(SORT(.debug.jdb.log_table)))
    PROVIDE (_log_table_end = .);

    . = ALIGN(0x40);
    
    PROVIDE (_etext = .);

    *(.data .data.* .gnu.linkonce.d.*)
    *(.anno)

    *(.data) 
    *(.gnu.linkonce.d*)
    *(.anno)
    *(.data.jdb)

    *(.init.data)

/* Constructor and destructor lists, ordered by priority.  The lists
   are traversed from end to start, therefore the *_END__ symbols
   precede the *_LIST__ symbols. */

    __PER_CPU_CTORS_END__ = .;
    KEEP (*(.ctors.00010))
    KEEP (*(.ctors.00009))
    KEEP (*(.ctors.00008))
    KEEP (*(.ctors.00007))
    KEEP (*(.ctors.00006))
    KEEP (*(.ctors.00005))
    KEEP (*(.ctors.00004))
    KEEP (*(.ctors.00003))
    KEEP (*(.ctors.00002))
    KEEP (*(.ctors.00001))
    __PER_CPU_CTORS_LIST__ = .;

    __CTOR_END__ = .;
    CONSTRUCTORS
    KEEP (*(.ctors))
    KEEP (*(SORT (.ctors.*)))
    __CTOR_LIST__ = .;

    __DTOR_END__ = .;    
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_LIST__ = .;

    PROVIDE(_edata  =  .);

  }

  . = ALIGN(8);
  .per_cpu_data : AT (ADDR(.per_cpu_data) - phys_offset) {
    PROVIDE (_per_cpu_data_start = .);
    *(.per_cpu.data)
    . = ALIGN(8);
    PROVIDE (_per_cpu_data_end = .);
  }

  /* ARM AEABI */
  . = ALIGN(8);
  .ARM.exidx : { *(.ARM.exidx.*) }
  .ARM.extab : { *(.ARM.extab.*) }



  . = ALIGN(4K);
  .bss : AT (ADDR(.bss) - phys_offset) {
    PROVIDE(__bss_start = .);
    *(.bss .bss.* COMMON .gnu.linkonce.b.*)
    *(.bss.jdb)
    PROVIDE(__bss_end = .);
    . = ALIGN(4K);
  }
  __end_of_the_kernel = .;

  ivt_start = ADDR(.bss) + SIZEOF(.bss) - phys_offset;
  .excp 0xffff0000 : AT (ivt_start) {
    *(.vect .excp.text)
    *(.vect.extra)
  }

  . = __end_of_the_kernel + SIZEOF(.excp);

  PROVIDE( _end = . );
    
  . = ALIGN(4K);
  .initcall : AT(ADDR(.initcall) - phys_offset) {
    _initcall_start = .;
    *(.initcall.*)
    . = ALIGN(4K);
    _initcall_end = .;
  } 

  /DISCARD/ :{
    *(.stab .stabstr .stab.excl .stab.exclstr)
    *(.stab.index .stab.indexstr .comment)
  }

}
