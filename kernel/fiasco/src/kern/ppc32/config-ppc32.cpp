INTERFACE [ppc32]:

#include <cstring>

EXTENSION class Config
{
public:
  enum {
    PAGE_SHIFT = ARCH_PAGE_SHIFT,
    PAGE_SIZE  = 1 << PAGE_SHIFT,
    PAGE_MASK  = ~(PAGE_SIZE - 1),

    SUPERPAGE_SHIFT = 22,
    SUPERPAGE_SIZE  = 1 << SUPERPAGE_SHIFT,
    SUPERPAGE_MASK  = ~(SUPERPAGE_SIZE -1),
    hlt_works_ok = 1,
    Irq_shortcut = 0, //TODO: set
  };

  enum
  {
    Kmem_size     = 8*1024*1024, //8 MB
    Htab_entries  = 8, //number of entries in page-table-entry group
                       //, min=1, max=8
  };

  enum 
  {
#ifdef CONFIG_ONE_SHOT
    scheduler_one_shot		= 1,
    scheduler_granularity	= 1UL,
    default_time_slice	        = 10000 * scheduler_granularity,
#else
    scheduler_one_shot		= 0,
    scheduler_granularity	= 1000UL,
    default_time_slice	        = 10 * scheduler_granularity,
#endif
  };

  static unsigned const default_console_uart = 3;
  static unsigned const default_console_uart_baudrate = 115200;
  static const char char_micro;

//TODO: check values
  static const bool getchar_does_hlt = true;
  static const bool getchar_does_hlt_works_ok = true;

//TODO: check
  static const bool enable_io_protection = false;
};


//---------------------------------------------------------------------------
IMPLEMENTATION [ppc32]:

char const Config::char_micro = '\265';
const char *const Config::kernel_warn_config_string = 0;
//static int Config::serial_esc;
//---------------------------------------------------------------------------
IMPLEMENTATION [ppc32 & serial]:

IMPLEMENT FIASCO_INIT
void Config::init()
{
  serial_esc = SERIAL_ESC_IRQ;
}

//---------------------------------------------------------------------------
IMPLEMENTATION [ppc32 & !serial]:

IMPLEMENT FIASCO_INIT
void Config::init()
{}
