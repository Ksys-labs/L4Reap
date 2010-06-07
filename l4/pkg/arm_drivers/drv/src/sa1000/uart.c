/*
 * UART Driver SA1000
 */

#include <l4/arm_drivers_c/uart.h>


/* ------------------------------------------------------------- */

enum {
  PAR_NONE = 0x00,
  PAR_EVEN = 0x03,
  PAR_ODD  = 0x01,
  DAT_5    = (unsigned)-1,
  DAT_6    = (unsigned)-1,
  DAT_7    = 0x00,
  DAT_8    = 0x08,
  STOP_1   = 0x00,
  STOP_2   = 0x04,

  MODE_8N1 = PAR_NONE | DAT_8 | STOP_1,
  MODE_7E1 = PAR_EVEN | DAT_7 | STOP_1,

// these two values are to leave either mode
// or baud rate unchanged on a call to change_mode
  MODE_NC  = 0x1000000,
  BAUD_NC  = 0x1000000,

};

enum {
  UTCR0_PE  = 0x01,
  UTCR0_OES = 0x02,
  UTCR0_SBS = 0x04,
  UTCR0_DSS = 0x08,
  UTCR0_SCE = 0x10,
  UTCR0_RCE = 0x20,
  UTCR0_TCE = 0x40,

  UTCR3_RXE = 0x01,
  UTCR3_TXE = 0x02,
  UTCR3_BRK = 0x04,
  UTCR3_RIE = 0x08,
  UTCR3_TIE = 0x10,
  UTCR3_LBM = 0x20,


  UTSR0_TFS = 0x01,
  UTSR0_RFS = 0x02,
  UTSR0_RID = 0x04,
  UTSR0_RBB = 0x08,
  UTSR0_REB = 0x10,
  UTSR0_EIF = 0x20,

  UTSR1_TBY = 0x01,
  UTSR1_RNE = 0x02,
  UTSR1_TNF = 0x04,
  UTSR1_PRE = 0x08,
  UTSR1_FRE = 0x10,
  UTSR1_ROR = 0x20,

  UARTCLK = 3686400,
};

l4_addr_t address;
unsigned _irq;


inline static l4_addr_t _utcr0( l4_addr_t a )
{ return a; }

inline static l4_addr_t _utcr1( l4_addr_t a )
{ return (a+0x04); }

inline static l4_addr_t _utcr2( l4_addr_t a )
{ return (a+0x08); }

inline static l4_addr_t _utcr3( l4_addr_t a )
{ return (a+0x0c); }

inline static l4_addr_t _utcr4( l4_addr_t a )
{ return (a+0x10); }

inline static l4_addr_t _utdr( l4_addr_t a )
{ return (a+0x14); }

inline static l4_addr_t _utsr0( l4_addr_t a )
{ return (a+0x1c); }

inline static l4_addr_t _utsr1( l4_addr_t a )
{ return (a+0x20); }


static inline
unsigned utcr0_get(void)
{ return *((volatile unsigned*)(_utcr0(address))); }

static inline
unsigned utcr1_get(void)
{ return *((volatile unsigned*)(_utcr1(address))); }

static inline
unsigned utcr2_get(void)
{ return *((volatile unsigned*)(_utcr2(address))); }

static inline
unsigned utcr3_get(void)
{ return *((volatile unsigned*)(_utcr3(address))); }

static inline
unsigned utcr4_get(void)
{ return *((volatile unsigned*)(_utcr4(address))); }

static inline
unsigned utdr_get(void)
{ return *((volatile unsigned*)(_utdr(address))); }

static inline
unsigned utsr0_get(void)
{ return *((volatile unsigned*)(_utsr0(address))); }

static inline
unsigned utsr1_get(void)
{ return *((volatile unsigned*)(_utsr1(address))); }


static inline
void utcr0_set(unsigned v)
{ *((volatile unsigned*)(_utcr0(address)))= v; }

static inline
void utcr1_set(unsigned v)
{ *((volatile unsigned*)(_utcr1(address)))= v; }

static inline
void utcr2_set(unsigned v)
{ *((volatile unsigned*)(_utcr2(address)))= v; }

static inline
void utcr3_set(unsigned v)
{ *((volatile unsigned*)(_utcr3(address)))= v; }

static inline
void utcr4_set(unsigned v)
{ *((volatile unsigned*)(_utcr4(address)))= v; }

static inline
void utdr_set(unsigned v)
{ *((volatile unsigned*)(_utdr(address)))= v; }

static inline
void utsr0_set(unsigned v)
{ *((volatile unsigned*)(_utsr0(address)))= v; }

static inline
void utsr1_set(unsigned v)
{ *((volatile unsigned*)(_utsr1(address)))= v; }

static inline
int tx_empty(void)
{
  return !(utsr1_get() & UTSR1_TBY);
}


void uart_init(void)
{
  address = (unsigned)-1;
  _irq = (unsigned)-1;
}

int uart_startup(l4_addr_t _address, unsigned irq)
{
  address =_address;
  _irq = irq;
  utsr0_set((unsigned)-1); //clear pending status bits
  utcr3_set(UTCR3_RXE | UTCR3_TXE); //enable transmitter and receiver
  return true;
}

void uart_shutdown(void)
{
  utcr3_set(0);
}

int uart_change_mode(TransferMode m, BaudRate baud)
{
  unsigned old_utcr3, quot;
  proc_status st;
  if(baud == (BaudRate)-1)
    return false;
  if(baud != BAUD_NC && (baud>115200 || baud<96))
    return false;
  if(m == (TransferMode)-1)
    return false;

  st = proc_cli_save();
  old_utcr3 = utcr3_get();
  utcr3_set(old_utcr3 & ~(UTCR3_RIE|UTCR3_TIE));
  proc_sti_restore(st);

  while(utsr1_get() & UTSR1_TBY);

  /* disable all */
  utcr3_set(0);

  /* set parity, data size, and stop bits */
  if(m!=MODE_NC)
    utcr0_set(m & 0x0ff);

  /* set baud rate */
  if(baud!=BAUD_NC)
    {
      quot = (UARTCLK / (16*baud)) -1;
      utcr1_set((quot & 0xf00) >> 8);
      utcr2_set(quot & 0x0ff);
    }

  utsr0_set((unsigned)-1);

  utcr3_set(old_utcr3);
  return true;
}

int uart_write( const char *s, unsigned count )
{
  unsigned old_utcr3;
  proc_status st;
  unsigned i;

  st = proc_cli_save();
  old_utcr3 = utcr3_get();
  utcr3_set( (old_utcr3 & ~(UTCR3_RIE|UTCR3_TIE)) | UTCR3_TXE );

  /* transmission */
  for(i =0; i<count; i++)
    {
      while(!(utsr1_get() & UTSR1_TNF)) ;
      utdr_set(s[i]);
      if( s[i]=='\n' )
        {
          while(!(utsr1_get() & UTSR1_TNF)) ;
          utdr_set('\r');
        }
    }

  /* wait till everything is transmitted */
  while(utsr1_get() & UTSR1_TBY) ;

  utcr3_set(old_utcr3);
  proc_sti_restore(st);
  return 1;
}

int uart_getchar(int blocking)
{
  unsigned old_utcr3, ch;

  old_utcr3 = utcr3_get();
  utcr3_set( old_utcr3 & ~(UTCR3_RIE|UTCR3_TIE) );
  while(!(utsr1_get() & UTSR1_RNE))
    if(!blocking)
      return -1;

  ch = utdr_get();
  utcr3_set(old_utcr3);
  return ch;
}

int uart_char_avail(void)
{
  if((utsr1_get() & UTSR1_RNE))
    {
      return 1;
    }
  else
    return 0;
}

l4_addr_t uart_base(void)
{
  //return 0x80050000;
  return 0x80010000;
}

int uart_get_mode(enum uart_mode_type type)
{
  enum {
    PAR_NONE = 0x00,
    PAR_EVEN = 0x03,
    PAR_ODD  = 0x01,
    DAT_5    = (unsigned)-1,
    DAT_6    = (unsigned)-1,
    DAT_7    = 0x00,
    DAT_8    = 0x08,
    STOP_1   = 0x00,
    STOP_2   = 0x04,

    MODE_8N1 = PAR_NONE | DAT_8 | STOP_1,
    MODE_7E1 = PAR_EVEN | DAT_7 | STOP_1,

    // these two values are to leave either mode
    // or baud rate unchanged on a call to change_mode
    MODE_NC  = 0x1000000,
    BAUD_NC  = 0x1000000,
  };

  switch (type)
    {
    case UART_MODE_TYPE_8N1:
      return MODE_8N1;
    case UART_MODE_TYPE_NONE:
      return 0;
    }
  return 0;
}
