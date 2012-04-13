#ifndef __ASM_L4__GENERIC__LOG_H__
#define __ASM_L4__GENERIC__LOG_H__

void l4x_printf(const char *fmt, ...)
   __attribute__((format(printf, 1, 2)));

#endif /* ! __ASM_L4__GENERIC__LOG_H__ */
