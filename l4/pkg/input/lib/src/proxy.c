/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

#include <l4/sys/ipc.h>
#include <l4/sys/utcb.h>
#include <pthread.h>

#include "internal.h"

MODULE_AUTHOR("Alexander Warg");
MODULE_DESCRIPTION("L4 Proxy input injector");
MODULE_LICENSE("GPL");

static char proxy_name[] = "L4 input event injector";
static char proxy_phys[] = "l4/sys";

static struct input_dev l4proxy_dev;

static pthread_t irq_tid;

static void *irq_handler(void *dummy)
{
	//l4thread_started(NULL);
	printk("%s: IRQ handler up\n", proxy_name);
	l4_umword_t input_server;
	l4_umword_t d0 = 0, d1 = 0;

	/* FIXME protocol hard-coded here */
	while (1)
	  {
	    /*l4_msgtag_t res = */l4_ipc_wait(l4_utcb(), &input_server, L4_IPC_NEVER);
	    enter_kdebug("IMPLEMENT ME!");
	    input_event(&l4proxy_dev,
			d0 & (unsigned short)0xffff,
			(d0 >> 16) & (unsigned short)0xffff,
			d1);
	  }
        return NULL;
}

//static int __init proxy_init(int prio)
int l4input_internal_proxy_init(int prio)
{
	init_input_dev(&l4proxy_dev);
	l4proxy_dev.name = proxy_name;
	l4proxy_dev.phys = proxy_phys;
	l4proxy_dev.evbit[0] = BIT(EV_KEY) | BIT(EV_REL) | BIT(EV_ABS) | BIT(EV_MSC);

	int i;
	for (i = 0; i < NBITS(KEY_MAX); i++) l4proxy_dev.keybit[i] = ~0UL;
	for (i = 0; i < NBITS(REL_MAX); i++) l4proxy_dev.relbit[i] = ~0UL;
	for (i = 0; i < NBITS(ABS_MAX); i++) l4proxy_dev.absbit[i] = ~0UL;
	for (i = 0; i < NBITS(MSC_MAX); i++) l4proxy_dev.mscbit[i] = ~0UL;

	/* XXX this is no touchpad */
	clear_bit(BTN_TOOL_FINGER, l4proxy_dev.keybit);

	input_register_device(&l4proxy_dev);

	printk(KERN_INFO "input: %s\n", proxy_name);

	/* FIXME name hard-coded here */
#if 0
	irq_tid = l4thread_create_named((l4thread_fn_t) irq_handler,
	                               "l4i_proxy", 0,
	                               L4THREAD_CREATE_SYNC);

	if (L4THREAD_IS_ERR(irq_tid)) {
#endif
	if (pthread_create(&irq_tid, NULL, irq_handler, NULL)) {
		printf("Error creating IRQ thread!");
		return 1;
	}

	return 0;
}
