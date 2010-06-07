/*
 * \brief   Logging facility with printf()-like interface
 * \author  Thomas Friebel <yaron@yaron.de>
 * \date    2006-03-01
 */

#include <l4/dde/ddekit/printf.h>

#include <stdio.h>

/**
 * Log constant string message w/o arguments
 *
 * \param msg  message to be logged
 */
void ddekit_print(const char *msg)
{
	printf("%s", msg);
}

/**
 * Log message with print()-like arguments
 *
 * \param fmt  format string followed by optional arguments
 */
void ddekit_printf(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	ddekit_vprintf(fmt, va);
	va_end(va);
}

/* Log message with vprintf()-like arguments
 *
 * \param fmt  format string
 * \param va   variable argument list
 */
void ddekit_vprintf(const char *fmt, va_list va)
{
	vprintf(fmt, va);
}
