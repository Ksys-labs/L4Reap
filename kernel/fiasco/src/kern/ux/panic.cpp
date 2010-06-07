INTERFACE:

/*
 * The generated headerfile is also exported to C code!
 */
#ifdef __cplusplus
#define EXT_DECL extern "C"
#else
#define EXT_DECL /* empty */
#endif

IMPLEMENTATION:

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

EXT_DECL __attribute__ ((noreturn))
void panic (const char *format, ...) {

  va_list pvar;

  va_start (pvar, format);
  vprintf  (format, pvar);
  va_end   (pvar);

  putchar ('\n');
 
  exit (EXIT_FAILURE);
}
