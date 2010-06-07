#pragma once

#include <l4/sys/compiler.h>
#include <stdarg.h>

EXTERN_C_BEGIN

/** Print message.
 * \ingroup DDEKit_util
 */
void ddekit_print(const char *);

/** Print message with format.
 * \ingroup DDEKit_util
 */
void ddekit_printf(const char *fmt, ...);

/** Print message with format list.
 * \ingroup DDEKit_util
 */
void ddekit_vprintf(const char *fmt, va_list va);

/** Log function and message.
 * \ingroup DDEKit_util
 */
#define ddekit_log(doit, msg...) \
	do {                                       \
		if (doit) {                            \
			ddekit_printf("%s(): ", __func__); \
			ddekit_printf(msg);                \
			ddekit_printf("\n");               \
		}                                      \
	} while(0);

EXTERN_C_END
