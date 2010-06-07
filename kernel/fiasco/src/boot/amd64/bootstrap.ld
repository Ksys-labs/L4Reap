SECTIONS
{
  .bootstrap.text 0 : 
    {
      *(.init)
      *(.text .text.* .gnu.linkonce.t.*)
      *(.fini)
      *(.rodata .rodata.* .gnu.linkonce.r.*)
      *(.data .data.* .gnu.linkonce.d.*)
      *(.anno)
    } = 0x90909090

  .bootstrap.bss 0 :
    {
      *(.bss .gnu.linkonce.b.* COMMON)
    }
}
