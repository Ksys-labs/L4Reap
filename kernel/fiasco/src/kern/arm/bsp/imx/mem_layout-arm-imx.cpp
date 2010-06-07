INTERFACE [arm && imx]: //----------------------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_imx {
    Device_map_base      = 0xef100000,
    Uart_map_base        = 0xef10a000,
    Timer_map_base       = 0xef103000,
    Pll_map_base         = 0xef127000,
    Watchdog_map_base    = 0xef102000,
    Pic_map_base         = 0xef140000,
    Uart_base            = Uart_map_base,
  };

  enum Phys_layout {
    Device_phys_base     = 0x10000000,
    Uart_phys_base       = 0x1000a000,
    Timer_phys_base      = 0x10003000,
    Pll_phys_base        = 0x10027000,
    Watchdog_phys_base   = 0x10002000,
    Pic_phys_base        = 0x10040000,
    Sdram_phys_base      = 0xc0000000,
    Flush_area_phys_base = 0xe0000000,
  };
};

