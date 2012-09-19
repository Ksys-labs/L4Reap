
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/param.h>
#include <linux/smp.h>
#include <linux/timex.h>
#include <linux/clockchips.h>
#include <linux/irq.h>

#include <asm/l4lxapi/thread.h>

#include <asm/generic/cap_alloc.h>
#include <asm/generic/setup.h>
#include <asm/generic/smp.h>
#include <asm/generic/vcpu.h>
#include <asm/generic/timer.h>

#include <l4/sys/factory.h>
#include <l4/sys/irq.h>
#include <l4/log/log.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap.h>

enum { PRIO_TIMER = CONFIG_L4_PRIO_IRQ_BASE + 1 };

enum Protos
{
	L4_PROTO_TIMER = 19L,
};

enum L4_timer_ops
{
	L4_TIMER_OP_START = 0UL,
	L4_TIMER_OP_STOP  = 1UL,
};

enum L4_timer_flags
{
	L4_TIMER_F_UNUSED,
};

typedef unsigned long long l4timer_time_t;
static l4_cap_idx_t timer_srv;

static void L4_CV timer_thread(void *data)
{
	l4_timeout_t to;
	l4_utcb_t *u = l4_utcb();
	l4_cap_idx_t irq_cap = *(l4_cap_idx_t *)data;
	l4_msgtag_t t;
	l4_umword_t l;
	l4_msg_regs_t *v = l4_utcb_mr_u(u);
	l4timer_time_t increment = 0;
	l4_cpu_time_t next_to = 0;

	enum {
		idx_at = 2,
		idx_increment = idx_at + sizeof(l4timer_time_t) / sizeof(v->mr[0]),
	};

	to = L4_IPC_NEVER;
	t = l4_ipc_wait(u, &l, to);
	while (1) {
		int reply = 1;
		int r = 0;

		if (l4_ipc_error(t, u) == L4_IPC_RETIMEOUT) {
			if (l4_error(l4_irq_trigger(irq_cap)) != -1)
				LOG_printf("IRQ timer trigger failed\n");

			if (increment) {
				next_to += increment;
				to = l4_timeout(L4_IPC_TIMEOUT_0,
				                l4_timeout_abs_u(next_to, 1, u));
			} else {
				to = L4_IPC_NEVER;
				increment = 0;
			}
			reply = 0;
		} else if (l4_error(t) == L4_PROTO_TIMER) {
			switch (v->mr[0]) {
				case L4_TIMER_OP_START:
					next_to = *(l4timer_time_t *)&v->mr[idx_at];
					to = l4_timeout(L4_IPC_TIMEOUT_0,
					                l4_timeout_abs_u(next_to, 1, u));
					increment = *(l4timer_time_t *)&v->mr[idx_increment];
					r = 0;
					break;
				case L4_TIMER_OP_STOP:
					to = L4_IPC_NEVER;
					increment = 0;
					r = 0;
					break;
				default:
					LOG_printf("l4timer: invalid opcode\n");
					r = -ENOSYS;
					break;
			};
		} else
			LOG_printf("l4timer: msg r=%ld\n", l4_error(t));

		t = l4_msgtag(r, 0, 0, 0);
		if (reply)
			t = l4_ipc_reply_and_wait(u, t, &l, to);
		else
			t = l4_ipc_wait(u, &l, to);
	}


}

static l4_msgtag_t l4timer_start_u(l4_cap_idx_t timer, unsigned flags,
                                   l4timer_time_t at, l4timer_time_t increment,
                                   l4_utcb_t *utcb)
{
	int idx = 2;
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = L4_TIMER_OP_START;
	v->mr[1] = flags;
	*(l4timer_time_t *)(&v->mr[idx]) = at;
	idx += sizeof(l4timer_time_t) / sizeof(v->mr[0]);
	*(l4timer_time_t *)(&v->mr[idx]) = increment;
	idx += sizeof(l4timer_time_t) / sizeof(v->mr[0]);
	return l4_ipc_call(timer, utcb,
	                   l4_msgtag(L4_PROTO_TIMER, idx, 0, 0),
	                   L4_IPC_NEVER);
}

static l4_msgtag_t l4timer_stop_u(l4_cap_idx_t timer, l4_utcb_t *utcb)
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = L4_TIMER_OP_STOP;
	return l4_ipc_call(timer, utcb, l4_msgtag(L4_PROTO_TIMER, 1, 0, 0),
                           L4_IPC_NEVER);
}

static l4_msgtag_t l4timer_start(l4_cap_idx_t timer, unsigned flags,
                                 l4timer_time_t at, l4timer_time_t increment)
{
	return l4timer_start_u(timer, flags, at, increment, l4_utcb());
}

static l4_msgtag_t l4timer_stop(l4_cap_idx_t timer)
{
	return l4timer_stop_u(timer, l4_utcb());
}





static l4_cap_idx_t timer_irq_cap;


static void timer_set_mode(enum clock_event_mode mode,
                           struct clock_event_device *clk)
{
	const l4timer_time_t increment = 1000000 / HZ;
	int r;

	switch (mode) {
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_ONESHOT:
		r = L4XV_FN_i(l4_error(l4timer_stop(timer_srv)));
		if (r)
			printk(KERN_WARNING "l4timer: stop failed (%d)\n", r);
		while (L4XV_FN_i(l4_ipc_error(l4_irq_receive(timer_irq_cap, L4_IPC_BOTH_TIMEOUT_0), l4_utcb())) != L4_IPC_RETIMEOUT)
			;
		break;
	case CLOCK_EVT_MODE_PERIODIC:
	case CLOCK_EVT_MODE_RESUME:
		r = L4XV_FN_i(l4_error(l4timer_start(timer_srv, 0,
		                       l4lx_kinfo->clock, increment)));
		if (r)
			printk(KERN_WARNING "l4timer: start failed (%d)\n", r);
		break;
	case CLOCK_EVT_MODE_UNUSED:
		break;
	default:
		printk("l4timer_set_mode: Unknown mode %d\n", mode);
		break;
	}
}

static int timer_set_next_event(unsigned long evt,
                                struct clock_event_device *unused)
{
	printk("timer_set_next_event\n");
	return 0;
}

static struct clock_event_device l4timer_clockevent = {
	.name           = "timer",
	.shift          = 10,
	.features       = CLOCK_EVT_FEAT_PERIODIC,
	.set_mode       = timer_set_mode,
	.set_next_event = timer_set_next_event,
	.rating         = 300,
};

static irqreturn_t timer_interrupt_handler(int irq, void *dev_id)
{
	l4timer_clockevent.event_handler(&l4timer_clockevent);
	l4x_smp_broadcast_timer();
	return IRQ_HANDLED;
}

static struct irqaction l4timer_irq = {
	.name           = "L4-timer",
	.flags          = IRQF_TIMER | IRQF_IRQPOLL | IRQF_NOBALANCING,
	.handler        = timer_interrupt_handler,
	.dev_id		= &l4timer_clockevent,
};

static int __init l4x_timer_init_ret(void)
{
	int r;
	l4lx_thread_t thread;
	int irq;
	L4XV_V(f);

	timer_irq_cap = l4x_cap_alloc();
	if (l4_is_invalid_cap(timer_irq_cap)) {
		printk(KERN_ERR "l4timer: Failed to alloc\n");
		return -ENOMEM;
	}

	r = L4XV_FN_i(l4_error(l4_factory_create_irq(l4re_env()->factory,
	                                             timer_irq_cap)));
	if (r) {
		printk(KERN_ERR "l4timer: Failed to create irq: %d\n", r);
		goto out1;
	}

	if ((irq = l4x_register_irq(timer_irq_cap)) < 0) {
		r = -ENOMEM;
		goto out2;
	}

	printk("l4timer: Using IRQ%d\n", irq);

	setup_irq(irq, &l4timer_irq);

	L4XV_L(f);
	thread = l4lx_thread_create
                  (timer_thread,                /* thread function */
                   smp_processor_id(),          /* cpu */
                   NULL,                        /* stack */
                   &timer_irq_cap, sizeof(timer_irq_cap), /* data */
                   l4x_cap_alloc(),             /* cap */
                   PRIO_TIMER,                  /* prio */
                   0,                           /* vcpup */
                   "timer",                     /* name */
		   NULL);
	L4XV_U(f);

	timer_srv = l4lx_thread_get_cap(thread);

	if (!l4lx_thread_is_valid(thread)) {
		printk(KERN_ERR "l4timer: Failed to create thread\n");
		r = -ENOMEM;
		goto out3;
	}


	l4timer_clockevent.irq = irq;
	l4timer_clockevent.mult =
		div_sc(1000000, NSEC_PER_SEC, l4timer_clockevent.shift);
	l4timer_clockevent.max_delta_ns =
		clockevent_delta2ns(0xffffffff, &l4timer_clockevent);
	l4timer_clockevent.min_delta_ns =
		clockevent_delta2ns(0xf, &l4timer_clockevent);
	l4timer_clockevent.cpumask = cpumask_of(0);
	clockevents_register_device(&l4timer_clockevent);

	return 0;

out3:
	l4x_unregister_irq(irq);
out2:
	L4XV_FN_v(l4re_util_cap_release(timer_irq_cap));
out1:
	l4x_cap_free(timer_irq_cap);
	return r;
}

void __init l4x_timer_init(void)
{
	if (l4x_timer_init_ret())
		printk(KERN_ERR "l4timer: Failed to initialize!\n");
}
