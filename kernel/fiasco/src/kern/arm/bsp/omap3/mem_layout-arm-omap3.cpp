INTERFACE [arm && omap3]: //--------------------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_omap3 {
    Devices1_map_base       = Registers_map_start,
    L4_addr_prot_map_base   = Devices1_map_base + 0x00040000,
    Uart1_map_base          = Devices1_map_base + 0x0006a000,
    Gptimer10_map_base      = Devices1_map_base + 0x00086000,
    Wkup_cm_map_base        = Devices1_map_base + 0x00004c00,

    Devices2_map_base       = Registers_map_start + 0x00100000,
    Intc_map_base           = Devices2_map_base   + 0x0,

    Devices3_map_base       = Registers_map_start + 0x00200000,
    Gptimer1_map_base       = Devices3_map_base   + 0x00018000,
    Prm_global_reg_map_base = Devices3_map_base   + 0x00007200,

    Devices4_map_base       = Registers_map_start + 0x00300000,
    Uart3_map_base          = Devices4_map_base   + 0x00020000,

    Timer_base              = Gptimer1_map_base,
  };

  enum Phys_layout_omap3 {
    Devices1_phys_base       = 0x48000000,
    L4_addr_prot_phys_base   = Devices1_phys_base + 0x00040000,
    Uart1_phys_base          = Devices1_phys_base + 0x0006a000,
    Gptimer10_phys_base      = Devices1_phys_base + 0x00086000,
    Wkup_cm_phys_base        = Devices1_phys_base + 0x00004c00,

    Devices2_phys_base       = 0x48200000,
    Intc_phys_base           = Devices2_phys_base + 0x0,

    Devices3_phys_base       = 0x48300000,
    Gptimer1_phys_base       = Devices3_phys_base + 0x00018000,
    Prm_global_reg_phys_base = Devices3_phys_base + 0x00007200,

    Devices4_phys_base       = 0x49000000,
    Uart3_phys_base          = Devices4_phys_base + 0x00020000,

    Sdram_phys_base          = 0x80000000,

    Flush_area_phys_base     = 0xe0000000,
  };
};

INTERFACE [arm && omap3_evm]: //-------------------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_omap3_evm {
    Uart_base               = Uart1_map_base,
  };
};

INTERFACE [arm && omap3_beagleboard]: //-----------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_omap3_beagleboard {
    Uart_base               = Uart3_map_base,
  };
};

