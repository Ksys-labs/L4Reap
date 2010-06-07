#pragma once

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

/** \defgroup DDEKit_util */

/** Panic - print error message and enter the kernel debugger.
 * \ingroup DDEKit_util
 */
void ddekit_panic(char const *fmt, ...) __attribute__((noreturn));

/** Print a debug message.
 * \ingroup DDEKit_util
 */
void ddekit_debug(char const *fmt, ...);

/** Print current backtrace.
 *
 * \ingroup DDEKit_util
 */
void ddekit_backtrace(void);

EXTERN_C_END
