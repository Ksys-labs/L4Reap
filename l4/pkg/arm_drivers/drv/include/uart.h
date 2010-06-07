#ifndef __BOOTSTRAP__DRV__ARM__UART_H__
#define __BOOTSTRAP__DRV__ARM__UART_H__

#include <l4/sys/types.h>

enum {
  false = 0,
  true
};

enum uart_mode_type {
  UART_MODE_TYPE_NONE,
  UART_MODE_TYPE_8N1
};

typedef unsigned TransferMode;
typedef unsigned BaudRate;

void uart_init(void);
int uart_startup(l4_addr_t _address, unsigned irq);
int uart_write(const char *s, unsigned count);
void uart_shutdown(void);
int uart_change_mode(TransferMode m, BaudRate baud);
int uart_getchar(int blocking);
int uart_char_avail(void);
l4_addr_t uart_base(void);
int uart_get_mode(enum uart_mode_type);

#include "proc.h"

#endif /* ! __BOOTSTRAP__DRV__ARM__UART_H__ */
