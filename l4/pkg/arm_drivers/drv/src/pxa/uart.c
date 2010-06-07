/*
 * UART Driver PXA
 */

#include <l4/arm_drivers_c/uart.h>

enum {
  Base_rate     = 921600,
  Base_ier_bits = 1 << 6,
};

enum {
  PAR_NONE = 0x00,
  PAR_EVEN = 0x18,
  PAR_ODD  = 0x08,
  DAT_5    = 0x00,
  DAT_6    = 0x01,
  DAT_7    = 0x02,
  DAT_8    = 0x03,
  STOP_1   = 0x00,
  STOP_2   = 0x04,

  MODE_8N1 = PAR_NONE | DAT_8 | STOP_1,
  MODE_7E1 = PAR_EVEN | DAT_7 | STOP_1,

  // these two values are to leave either mode
  // or baud rate unchanged on a call to change_mode
  MODE_NC  = 0x1000000,
  BAUD_NC  = 0x1000000,
};

enum Registers {
  TRB      = 0, // Transmit/Receive Buffer  (read/write)
  BRD_LOW  = 0, // Baud Rate Divisor LSB if bit 7 of LCR is set  (read/write)
  IER      = 1, // Interrupt Enable Register  (read/write)
  BRD_HIGH = 1, // Baud Rate Divisor MSB if bit 7 of LCR is set  (read/write)
  IIR      = 2, // Interrupt Identification Register  (read only)
  FCR      = 2, // 16550 FIFO Control Register  (write only)
  LCR      = 3, // Line Control Register  (read/write)
  MCR      = 4, // Modem Control Register  (read/write)
  LSR      = 5, // Line Status Register  (read only)
  MSR      = 6, // Modem Status Register  (read only)
  SPR      = 7, // Scratch Pad Register  (read/write)
};

unsigned port;
int _irq;

static inline
void outb(char b, enum Registers reg)
{
  *(volatile char *)((port+reg)*4) = b;
}

static inline
char inb(enum Registers reg)
{
  return *(volatile char *)((port+reg)*4);
}


static inline
void mcr_set(char b)
{
  outb(b, MCR);
}

static inline
char mcr_get(void)
{
  return inb(MCR);
}

static inline
void fcr_set(char b)
{
  outb(b, FCR);
}

static inline
void lcr_set(char b)
{
  outb(b, LCR);
}

static inline
char lcr_get(void)
{
  return inb(LCR);
}

static inline
void ier_set(char b)
{
  outb(b, IER);
}

static inline
char ier_get(void)
{
  return inb(IER);
}

static inline
char iir_get(void)
{
  return inb(IIR);
}

static inline
char msr_get(void)
{
  return inb(MSR);
}

static inline
char lsr_get(void)
{
  return inb(LSR);
}

static inline
void trb_set(char b)
{
  outb(b, TRB);
}

static inline
char trb_get(void)
{
  return inb(TRB);
}




static int uart_valid(void)
{
  char scratch, scratch2, scratch3;

  scratch = ier_get();
  ier_set(0x00);

  scratch2 = ier_get();
  ier_set(0x0f);

  scratch3 = ier_get();
  ier_set(scratch);

  return (scratch2 == 0x00 && scratch3 == 0x0f);
}


int uart_startup(l4_addr_t _port, unsigned __irq)
{
  port = _port;
  _irq  = __irq;

  if (!uart_valid())
    return false;

  proc_status o = proc_cli_save();
  ier_set(Base_ier_bits);/* disable all rs-232 interrupts */
  mcr_set(0x0b);         /* out2, rts, and dtr enabled */
  fcr_set(1);            /* enable fifo */
  fcr_set(0x07);         /* clear rcv xmit fifo */
  fcr_set(1);            /* enable fifo */
  lcr_set(0);            /* clear line control register */

  /* clearall interrupts */
  /*read*/ msr_get(); /* IRQID 0*/
  /*read*/ iir_get(); /* IRQID 1*/
  /*read*/ trb_get(); /* IRQID 2*/
  /*read*/ lsr_get(); /* IRQID 3*/

  while(lsr_get() & 1/*DATA READY*/)
    /*read*/ trb_get();
  proc_sti_restore(o);
  return true;
}

void uart_shutdown(void)
{
  proc_status o = proc_cli_save();
  mcr_set(0x06);
  fcr_set(0);
  lcr_set(0);
  ier_set(0);
  proc_sti_restore(o);
}

int uart_change_mode(TransferMode m, BaudRate r)
{
  proc_status o = proc_cli_save();
  char old_lcr = lcr_get();
  if(r != BAUD_NC) {
    l4_uint16_t divisor = Base_rate / r;
    lcr_set(old_lcr | 0x80/*DLAB*/);
    trb_set(divisor & 0x0ff );        /* BRD_LOW  */
    ier_set((divisor >> 8) & 0x0ff ); /* BRD_HIGH */
    lcr_set(old_lcr);
  }
  if (m != MODE_NC)
    lcr_set(m & 0x07f);

  proc_sti_restore(o);
  return true;
}

#if 0
int uart_getmode(void)
{
  return lcr_get() & 0x7f;
}
#endif

int uart_get_mode(enum uart_mode_type type)
{
  switch (type)
    {
      case UART_MODE_TYPE_8N1:
        return MODE_8N1;
      case UART_MODE_TYPE_NONE:
        return 0;
    }
  return 0;
}

int uart_write(char const *s, unsigned count)
{
  /* disable uart irqs */
  char old_ier;
  unsigned i;
  old_ier = ier_get();
  ier_set(old_ier & ~0x0f);

  /* transmission */
  for (i = 0; i < count; i++) {
    while (!(lsr_get() & 0x20 /* THRE */))
      ;
    if (s[i] == '\346')
      trb_set('\265');
    else
      trb_set(s[i]);
    if (s[i]=='\n') {
      while (!(lsr_get() & 0x20 /* THRE */))
	;
      trb_set('\r');
    }
  }

  /* wait till everything is transmitted */
  while (!(lsr_get() & 0x40 /* TSRE */))
    ;

  ier_set(old_ier);
  return 1;
}

int uart_getchar(int blocking)
{
  char old_ier, ch;

  if (!blocking && !(lsr_get() & 1 /* DATA READY */))
    return -1;

  old_ier = ier_get();
  ier_set(old_ier & ~0x0f);
  while(!(lsr_get() & 1 /* DATA READY */))
    ;
  ch = trb_get();
  ier_set(old_ier);
  return ch;
}

int uart_char_avail(void)
{
  if (lsr_get() & 1 /* DATA READY */)
    return 1;

  return 0;
}

l4_addr_t uart_base(void)
{
  return 0x40100000 / 4;
}
