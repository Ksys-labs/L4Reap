#include <l4/dde/ddekit/panic.h>
#include <l4/dde/ddekit/printf.h>

#include <l4/sys/kdebug.h>
#include <l4/util/backtrace.h>
#include <stdarg.h>

void ddekit_panic(const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	ddekit_vprintf(fmt, va);
	va_end(va);
	ddekit_printf("\n");

	ddekit_backtrace();

	while (1)
		enter_kdebug("ddekit_panic()");
}

void ddekit_debug(const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	ddekit_vprintf(fmt, va);
	va_end(va);
	ddekit_printf("\n");

	ddekit_backtrace();
	enter_kdebug("ddekit_debug()");
}

void ddekit_backtrace()
{
	int len = 16;
	void *array[len];
	unsigned i, ret = l4util_backtrace(&array[0], len);

	ddekit_printf("backtrace:\n");
	for (i = 0; i < ret; ++i)
		ddekit_printf("\t%p\n", array[i]);
}
