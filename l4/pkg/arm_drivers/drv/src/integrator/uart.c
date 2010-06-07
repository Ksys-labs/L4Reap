/*
 * UART Driver AMBA -- based on the Linux driver in 2.6.14
 */

#include <l4/arm_drivers_c/uart.h>


/* ------------------------------------------------------------- */

enum {
  UART011_TXIM = (1 << 5),

  UART01x_CR_UARTEN = 1, // UART enable
  UART011_CR_LBE    = 0x080, // loopback enable
  UART011_CR_TXE    = 0x100, // transmit enable
  UART011_CR_RXE    = 0x200, // receive enable

  UART01x_FR_BUSY   = 0x008,
  UART01x_FR_TXFF   = 0x020,

  UART01x_LCRH_PEN    = 0x02, // parity enable
  UART01x_LCRH_WLEN_8 = 0x60,
};

l4_addr_t address;
unsigned _irq;


inline static l4_addr_t _UART01x_DR(l4_addr_t a)
{ return a + 0x00; }

inline static l4_addr_t _UART01x_FR(l4_addr_t a)
{ return a + 0x18; }

inline static l4_addr_t _UART011_IBRD(l4_addr_t a)
{ return a + 0x24; }

inline static l4_addr_t _UART011_FBRD(l4_addr_t a)
{ return a + 0x28; }

inline static l4_addr_t _UART011_LCRH(l4_addr_t a)
{ return a + 0x2c; }

inline static l4_addr_t _UART011_CR(l4_addr_t a)
{ return a + 0x30; }

inline static l4_addr_t _UART011_IMSC(l4_addr_t a)
{ return a + 0x38; }

inline static l4_addr_t _UART011_ICR(l4_addr_t a)
{ return a + 0x44; }


// getters
static inline
unsigned UART01x_FR_get(void)
{ return *((volatile unsigned *)(_UART01x_FR(address))); }

static inline
unsigned UART011_CR_get(void)
{ return *((volatile unsigned *)(_UART011_CR(address))); }

static inline
unsigned UART011_IMSC_get(void)
{ return *((volatile unsigned *)(_UART011_IMSC(address))); }


// setters
static inline
void UART01x_DR_set(unsigned v)
{ *((volatile unsigned*)(_UART01x_DR(address))) = v; }

static inline
void UART011_IBRD_set(unsigned v)
{ *((volatile unsigned *)(_UART011_IBRD(address))) = v;}

static inline
void UART011_FBRD_set(unsigned v)
{ *((volatile unsigned *)(_UART011_FBRD(address))) = v;}

static inline
void UART011_LCRH_set(unsigned v)
{ *((volatile unsigned *)(_UART011_LCRH(address))) = v;}

static inline
void UART011_CR_set(unsigned v)
{ *((volatile unsigned *)(_UART011_CR(address))) = v;}

static inline
void UART011_IMSC_set(unsigned v)
{ *((volatile unsigned *)(_UART011_IMSC(address))) = v; }

static inline
void UART011_ICR_set(unsigned v)
{ *((volatile unsigned *)(_UART011_ICR(address))) = v; }

void uart_init(void)
{
  address = (unsigned)-1;
  _irq = (unsigned)-1;
}

int uart_startup(l4_addr_t _address, unsigned irq)
{
  unsigned cr = UART01x_CR_UARTEN | UART011_CR_TXE | UART011_CR_LBE;

  address =_address;
  _irq = irq;

  UART011_CR_set(cr);
  UART011_FBRD_set(0);
  UART011_IBRD_set(1);
  UART011_LCRH_set(0);
  UART01x_DR_set(1);

  while (UART01x_FR_get() & UART01x_FR_BUSY)
    asm volatile("" : : : "memory");

  return true;
}

void uart_shutdown(void)
{
  UART011_IMSC_set(0); // all interrupts off
  UART011_ICR_set(0xffff);

  UART011_CR_set(UART01x_CR_UARTEN | UART011_CR_TXE);
}

int uart_change_mode(TransferMode m, BaudRate baud)
{
  /* Be lazy, only support one mode */
  if (baud != 115200)
    return false;

  unsigned old_cr = UART011_CR_get();
  UART011_CR_set(0);

  UART011_FBRD_set(0x0);
  UART011_IBRD_set(0x4);

  UART011_LCRH_set(UART01x_LCRH_WLEN_8 | UART01x_LCRH_PEN);
  UART011_CR_set(old_cr);

  return true;
}

int uart_write( const char *s, unsigned count )
{
  unsigned old_im;
  proc_status st;
  unsigned i;

  st = proc_cli_save();
  old_im = UART011_IMSC_get();
  UART011_IMSC_set(old_im | UART011_TXIM);

  /* transmission */
  for(i =0; i<count; i++)
    {
      while (UART01x_FR_get() & UART01x_FR_TXFF)
	;
      UART01x_DR_set(s[i]);
      if( s[i]=='\n' )
        {
	  while (UART01x_FR_get() & UART01x_FR_TXFF)
	    ;
          UART01x_DR_set('\r');
        }
    }

  /* wait till everything is transmitted */
  while (UART01x_FR_get() & UART01x_FR_BUSY)
    ;

  UART011_IMSC_set(old_im & ~UART011_TXIM);

  proc_sti_restore(st);
  return 1;
}

int uart_getchar(int blocking)
{
  return 0;
}

int uart_char_avail(void)
{
  return 0;
}

l4_addr_t uart_base(void)
{
  return 0x16000000; // ttyAMA0 at MMIO 0x16000000 (irq = 1) is a AMBA/PL011
  //return 0x17000000; // ttyAMA1 at MMIO 0x17000000 (irq = 2) is a AMBA/PL011
}

int uart_get_mode(enum uart_mode_type type)
{
  return 0;
}
