/*
 * Early_printk implementation, skeleton taken from x86_64 version.
 */
#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>

#include <l4/sys/kdebug.h>

static void early_kdb_write(struct console *con, const char *s, unsigned n)
{
	while (*s && n-- > 0) {
		outchar(*s);
		if (*s == '\n')
			outchar('\r');
		s++;
	}
}

static struct console early_kdb_console = {
	.name =		"earlykdb",
	.write =	early_kdb_write,
	.flags =	CON_PRINTBUFFER,
	.index =	-1,
};

/* Direct interface for emergencies */
static struct console *early_console = &early_kdb_console;
static int early_console_initialized = 0;

asmlinkage void early_printk(const char *fmt, ...)
{
	char buf[512];
	int n;
	va_list ap;

	va_start(ap,fmt);
	n = vscnprintf(buf,512,fmt,ap);
	early_console->write(early_console,buf,n);
	va_end(ap);
}

static int keep_early;

int __init setup_early_printk(char *buf)
{
	if (!buf)
		return 0;

	if (early_console_initialized)
		return 0;
	early_console_initialized = 1;

	if (strstr(buf,"keep"))
		keep_early = 1;

	early_console = &early_kdb_console;
	register_console(early_console);
	return 0;
}

void __init disable_early_printk(void)
{
	if (!early_console_initialized || !early_console)
		return;
	if (!keep_early) {
		printk("disabling early console\n");
		unregister_console(early_console);
		early_console_initialized = 0;
	} else {
		printk("keeping early console\n");
	}
}

__setup("earlyprintk=", setup_early_printk);
