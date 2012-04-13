/*
 *  drivers/char/l4ser.c
 *
 *  L4 pseudo serial driver.
 *
 *  Based on sa1100.c and other drivers from drivers/serial/.
 */
#if defined(CONFIG_L4_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>

#include <l4/sys/vcon.h>
#include <l4/sys/factory.h>
#include <l4/sys/icu.h>
#include <l4/re/c/namespace.h>
#include <l4/re/c/util/cap.h>
#include <asm/generic/setup.h>
#include <asm/generic/cap_alloc.h>
#include <asm/generic/util.h>
#include <asm/generic/vcpu.h>

/* This is the same major as the sa1100 one */
#define SERIAL_L4SER_MAJOR	204
#define MINOR_START		5

#define PORT0_NAME              "log"
#define NR_ADDITIONAL_PORTS	3

static char ports_to_add[NR_ADDITIONAL_PORTS][20];
static int  ports_to_add_pos;

struct l4ser_uart_port {
	struct uart_port port;
	l4_cap_idx_t vcon_cap;
	l4_cap_idx_t vcon_irq_cap;
	int inited;
};

static struct l4ser_uart_port l4ser_port[1 + NR_ADDITIONAL_PORTS];

static void l4ser_stop_tx(struct uart_port *port)
{
}

static void l4ser_stop_rx(struct uart_port *port)
{
}

static void l4ser_enable_ms(struct uart_port *port)
{
}

static int
l4ser_getchar(struct l4ser_uart_port *l4port)
{
	char c;
	int res;
	L4XV_V(f);

	L4XV_L(f);
	if (l4_is_invalid_cap(l4port->vcon_irq_cap)
	    || l4_vcon_read(l4port->vcon_cap, &c, 1) <= 0)
		res = -1;
	else
		res = c;
	L4XV_U(f);
	return res;
}

static void
l4ser_rx_chars(struct uart_port *port)
{
	struct l4ser_uart_port *l4port = (struct l4ser_uart_port *)port;
	struct tty_struct *tty = port->state->port.tty;
	unsigned int flg;
	int ch;
	L4XV_V(f);

	while (1)  {
		L4XV_L(f);
		ch = l4ser_getchar(l4port);
		L4XV_U(f);
		if (ch == -1)
			break;
		port->icount.rx++;

		flg = TTY_NORMAL;

		// ^ can be used as a sysrq starter
		if (0)
			if (ch == '^') {
				if (port->sysrq)
					port->sysrq = 0;
				else {
					port->icount.brk++;
					uart_handle_break(port);
					continue;
				}
			}

		if (uart_handle_sysrq_char(port, ch))
			continue;

		tty_insert_flip_char(tty, ch, flg);
	}
	tty_flip_buffer_push(tty);
	return;
}

static void l4ser_tx_chars(struct uart_port *port)
{
	struct l4ser_uart_port *l4port = (struct l4ser_uart_port *)port;
	struct circ_buf *xmit = &port->state->xmit;
	int c;
	L4XV_V(f);

	if (port->x_char) {
		L4XV_L(f);
		l4_vcon_write(l4port->vcon_cap, &port->x_char, 1);
		L4XV_U(f);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}

	while (!uart_circ_empty(xmit)) {
		c = CIRC_CNT_TO_END(xmit->head, xmit->tail, UART_XMIT_SIZE);
		if (c > L4_VCON_WRITE_SIZE)
			c = L4_VCON_WRITE_SIZE;
		L4XV_L(f);
		l4_vcon_write(l4port->vcon_cap, &xmit->buf[xmit->tail], c);
		L4XV_U(f);
		xmit->tail = (xmit->tail + c) & (UART_XMIT_SIZE - 1);
		port->icount.tx += c;
	}
}

static void l4ser_start_tx(struct uart_port *port)
{
	l4ser_tx_chars(port);
}

static irqreturn_t l4ser_int(int irq, void *dev_id)
{
	struct uart_port *sport = dev_id;

	l4ser_rx_chars(sport);

	return IRQ_HANDLED;
}

static unsigned int l4ser_tx_empty(struct uart_port *port)
{
	return TIOCSER_TEMT;
}

static unsigned int l4ser_get_mctrl(struct uart_port *port)
{
	return 0;
}

static void l4ser_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static void l4ser_break_ctl(struct uart_port *port, int break_state)
{
}

static int l4ser_startup(struct uart_port *port)
{
	int retval;

	if (port->irq) {
		retval = request_irq(port->irq, l4ser_int, 0, "L4-uart", port);
		if (retval)
			return retval;

		l4ser_rx_chars(port);
	}

	return 0;
}

static void l4ser_shutdown(struct uart_port *port)
{
	if (port->irq)
		free_irq(port->irq, port);
}

static void l4ser_set_termios(struct uart_port *port, struct ktermios *termios,
                              struct ktermios *old)
{
}

static const char *l4ser_type(struct uart_port *port)
{
	return port->type == PORT_SA1100 ? "L4" : NULL;
}


static int l4ser_request_port(struct uart_port *port)
{
	return 0;
}

static void l4ser_release_port(struct uart_port *port)
{
}

static void l4ser_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_SA1100;
}

static int
l4ser_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return 0;
}

static struct uart_ops l4ser_pops = {
	.tx_empty	= l4ser_tx_empty,
	.set_mctrl	= l4ser_set_mctrl,
	.get_mctrl	= l4ser_get_mctrl,
	.stop_tx	= l4ser_stop_tx,
	.start_tx	= l4ser_start_tx,
	.stop_rx	= l4ser_stop_rx,
	.enable_ms	= l4ser_enable_ms,
	.break_ctl	= l4ser_break_ctl,
	.startup	= l4ser_startup,
	.shutdown	= l4ser_shutdown,
	.set_termios	= l4ser_set_termios,
	.type		= l4ser_type,
	.release_port	= l4ser_release_port,
	.request_port	= l4ser_request_port,
	.config_port	= l4ser_config_port,
	.verify_port	= l4ser_verify_port,
};

static int __init l4ser_init_port(int num, const char *name)
{
	int irq, r;
	l4_vcon_attr_t vcon_attr;
	L4XV_V(f);

	if (l4ser_port[num].inited)
		return 0;
	l4ser_port[num].inited = 1;

	if ((r = l4x_re_resolve_name(name, &l4ser_port[num].vcon_cap))) {
		if (num == 0)
			l4ser_port[num].vcon_cap = l4re_env()->log;
		else
			return r;
	}

	L4XV_L(f);
	l4ser_port[num].vcon_irq_cap = l4x_cap_alloc();
	if (l4_is_invalid_cap(l4ser_port[num].vcon_irq_cap)) {
		l4x_cap_free(l4ser_port[num].vcon_cap);
		L4XV_U(f);
		l4ser_port[num].vcon_cap = L4_INVALID_CAP;
		return -ENOMEM;
	}


	if (l4_error(l4_factory_create_irq(l4re_env()->factory,
	                                   l4ser_port[num].vcon_irq_cap))) {
		l4re_util_cap_release(l4ser_port[num].vcon_cap);
		L4XV_U(f);
		l4x_cap_free(l4ser_port[num].vcon_irq_cap);
		l4x_cap_free(l4ser_port[num].vcon_cap);
		return -ENOMEM;
	}

	if ((l4_error(l4_icu_bind(l4ser_port[num].vcon_cap, 0,
	                          l4ser_port[num].vcon_irq_cap)))) {
		l4re_util_cap_release(l4ser_port[num].vcon_irq_cap);
		l4x_cap_free(l4ser_port[num].vcon_irq_cap);

		// No interrupt, just output
		l4ser_port[num].vcon_irq_cap = L4_INVALID_CAP;
		irq = 0;
	} else if ((irq = l4x_register_irq(l4ser_port[num].vcon_irq_cap)) < 0) {
		l4x_cap_free(l4ser_port[num].vcon_irq_cap);
		l4x_cap_free(l4ser_port[num].vcon_cap);
		L4XV_U(f);
		return -EIO;
	}

	vcon_attr.i_flags = 0;
	vcon_attr.o_flags = 0;
	vcon_attr.l_flags = 0;
	l4_vcon_set_attr(l4ser_port[num].vcon_cap, &vcon_attr);
	L4XV_U(f);

	l4ser_port[num].port.uartclk   = 3686400;
	l4ser_port[num].port.ops       = &l4ser_pops;
	l4ser_port[num].port.fifosize  = 8;
	l4ser_port[num].port.line      = num;
	l4ser_port[num].port.iotype    = UPIO_MEM;
	l4ser_port[num].port.membase   = (void *)1;
	l4ser_port[num].port.mapbase   = 1;
	l4ser_port[num].port.flags     = UPF_BOOT_AUTOCONF;
	l4ser_port[num].port.irq       = irq;

	return 0;
}

#ifdef CONFIG_L4_SERIAL_CONSOLE

static int __init
l4ser_console_setup(struct console *co, char *options)
{
	struct uart_port *up;

	if (co->index >= 1 + NR_ADDITIONAL_PORTS)
		co->index = 0;
	up = &l4ser_port[co->index].port;
	if (!up)
		return -ENODEV;

	return uart_set_options(up, co, 115200, 'n', 8, 'n');
}

/*
 * Interrupts are disabled on entering
 */
static void
l4ser_console_write(struct console *co, const char *s, unsigned int count)
{
	do {
		unsigned c = count;
		L4XV_V(f);
		if (c > L4_VCON_WRITE_SIZE)
			c = L4_VCON_WRITE_SIZE;
		L4XV_L(f);
		l4_vcon_write(l4ser_port[co->index].vcon_cap, s, c);
		L4XV_U(f);
		count -= c;
	} while (count);
}


static struct uart_driver l4ser_reg;
static struct console l4ser_console = {
	.name		= "ttyLv",
	.write		= l4ser_console_write,
	.device		= uart_console_device,
	.setup		= l4ser_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &l4ser_reg,
};

static int __init l4ser_rs_console_init(void)
{
	if (l4ser_init_port(0, PORT0_NAME))
		return -ENODEV;
	register_console(&l4ser_console);
	return 0;
}
console_initcall(l4ser_rs_console_init);

#define L4SER_CONSOLE	&l4ser_console
#else
#define L4SER_CONSOLE	NULL
#endif

static struct uart_driver l4ser_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "ttyLv",
	.dev_name		= "ttyLv",
	.major			= SERIAL_L4SER_MAJOR,
	.minor			= MINOR_START,
	.nr			= 1 + NR_ADDITIONAL_PORTS,
	.cons			= L4SER_CONSOLE,
};

static int __init l4ser_serial_init(void)
{
	int ret, i;

	printk(KERN_INFO "L4 serial driver\n");

	if (l4ser_init_port(0, PORT0_NAME))
		return -ENODEV;

	ret = uart_register_driver(&l4ser_reg);
	if (!ret)
		uart_add_one_port(&l4ser_reg, &l4ser_port[0].port);

	for (i = 0; i < NR_ADDITIONAL_PORTS; ++i) {
		if (*ports_to_add[i]
		    && l4ser_init_port(i + 1, ports_to_add[i])) {
			printk(KERN_WARNING "l4ser: Failed to initialize additional port '%s'.\n", ports_to_add[i]);
			continue;
		}
		uart_add_one_port(&l4ser_reg, &l4ser_port[i + 1].port);
	}
	return ret;
}

static void __exit l4ser_serial_exit(void)
{
	int i;
	uart_remove_one_port(&l4ser_reg, &l4ser_port[0].port);
	for (i = 0; i < NR_ADDITIONAL_PORTS; ++i)
		uart_remove_one_port(&l4ser_reg, &l4ser_port[i + 1].port);
	uart_unregister_driver(&l4ser_reg);
}

module_init(l4ser_serial_init);
module_exit(l4ser_serial_exit);

/* This function is called much earlier than module_init, esp. there's
 * no kmalloc available */
static int l4ser_setup(const char *val, struct kernel_param *kp)
{
	if (ports_to_add_pos >= NR_ADDITIONAL_PORTS) {
		printk("l4ser: Too many additional ports specified\n");
		return 1;
	}
	strlcpy(ports_to_add[ports_to_add_pos], val,
	        sizeof(ports_to_add[ports_to_add_pos]));
	ports_to_add_pos++;
	return 0;
}

module_param_call(add, l4ser_setup, NULL, NULL, 0200);
MODULE_PARM_DESC(add, "Use l4ser.add=name to add an another port, name queried in cap environment");

MODULE_AUTHOR("Adam Lackorzynski <adam@os.inf.tu-dresden.de");
MODULE_DESCRIPTION("L4 serial driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV_MAJOR(SERIAL_L4SER_MAJOR);
