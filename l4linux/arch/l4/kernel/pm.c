
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <asm/cacheflush.h>
#include <asm/generic/stack_id.h>
#include <asm/generic/irq.h>
#include <asm/generic/tamed.h>
#include <asm/generic/vmalloc.h>
#include <asm/generic/log.h>

#include <asm/l4lxapi/task.h>
#include <asm/l4lxapi/memory.h>

#include <l4/sys/irq.h>

static LIST_HEAD(wakeup_srcs);

struct wakeup_src {
	struct list_head list;
	int irq;
	l4_cap_idx_t irqcap;
};

static DEFINE_SPINLOCK(list_lock);

static int suspend_all_irq = 1;
static int suspend_possible;

#ifndef CONFIG_L4_VCPU
static void l4x_global_wait_save(void)
{}

static void l4x_global_saved_event_inject(void)
{}
#endif


static l4_cap_idx_t get_int_cap(int irq)
{
	if (irq < NR_IRQS_HW) {
		struct l4x_irq_desc_private *p = irq_get_chip_data(irq);
		return p->irq_cap;
	}

	if (irq < NR_IRQS)
		return l4x_have_irqcap(irq);

	return L4_INVALID_CAP;
}

static int l4x_wakeup_source_register(int irq)
{
	struct wakeup_src *s, *tmp;
	unsigned long flags;
	l4_cap_idx_t cap;

	list_for_each_entry_safe(s, tmp, &wakeup_srcs, list)
		if (irq == s->irq)
			return -EEXIST;

	cap = get_int_cap(irq);
	if (l4_is_invalid_cap(cap))
		return -ENOENT;

	s = kmalloc(sizeof(*s), GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->irq    = irq;
	s->irqcap = cap;

	spin_lock_irqsave(&list_lock, flags);
	list_add(&s->list, &wakeup_srcs);
	spin_unlock_irqrestore(&list_lock, flags);

	return 0;
}

static int l4x_wakeup_source_unregister(int irq)
{
	struct wakeup_src *s, *tmp;
	unsigned long flags;
	int r = -EINVAL;

	list_for_each_entry_safe(s, tmp, &wakeup_srcs, list)
		if (irq == s->irq) {
			spin_lock_irqsave(&list_lock, flags);
			list_del(&s->list);
			spin_unlock_irqrestore(&list_lock, flags);
			r = 0;
			break;
		}

	return r;
}

static int l4x_pm_plat_prepare(void)
{
	return 0;
}

static void attach_to_irq(int irq, l4_cap_idx_t cap, l4_cap_idx_t t)
{
	int ret = L4XV_FN_i(l4_error(l4_irq_attach(cap, irq << 2, t)));
	if (ret)
		printk("Failed to attach wakeup source %d/%lx: %d\n",
				irq, cap, ret);
	else
		printk("Attached to wakeup source %d(%lx)\n", irq, cap);
	L4XV_FN_v(l4_irq_unmask(cap));
}

static void detach_from_irq(int irq, l4_cap_idx_t cap)
{
	int ret = L4XV_FN_i(l4_error(l4_irq_detach(cap)));
	if (ret)
		printk("Failed to detach wakeup source %d: %d\n", irq, ret);
	else
		printk("Detached from wakeup source %d(%lx)\n", irq, cap);
}

static void loop_over_irqs(int doattach)
{
	int i;
	struct wakeup_src *s;
	l4_cap_idx_t t = l4x_cpu_thread_get_cap(smp_processor_id());

	if (suspend_all_irq) {
		for (i = 0; i < NR_IRQS; ++i) {
			struct irq_desc *desc = irq_to_desc(i);
			l4_cap_idx_t cap = get_int_cap(i);
			if (!desc || l4_is_invalid_cap(cap))
				continue;

			if (desc->action
			    && desc->action->flags & __IRQF_TIMER)
				continue;

			if (doattach)
				attach_to_irq(i, cap, t);
			else
				detach_from_irq(i, cap);
		}
	} else {
		list_for_each_entry(s, &wakeup_srcs, list) {
			if (doattach)
				attach_to_irq(s->irq, s->irqcap, t);
			else
				detach_from_irq(s->irq, s->irqcap);
		}
	}
}

static int l4x_pm_plat_enter(suspend_state_t state)
{
	if (state != PM_SUSPEND_MEM)
		return -EINVAL;

	loop_over_irqs(1);

	flush_cache_all();

	l4x_global_wait_save();

	loop_over_irqs(0);

	return 0;
}

static void l4x_pm_plat_finish(void)
{
	l4x_global_saved_event_inject();
}

static void l4x_pm_plat_wake(void)
{
}

static int l4x_pm_plat_valid(suspend_state_t state)
{
#ifdef CONFIG_L4_VCPU
	if (suspend_possible || suspend_all_irq)
		return suspend_valid_only_mem(state);
#endif
	return 0;
}

static struct platform_suspend_ops l4x_pm_plat_ops = {
	.prepare = l4x_pm_plat_prepare,
	.enter   = l4x_pm_plat_enter,
	.finish  = l4x_pm_plat_finish,
	.wake    = l4x_pm_plat_wake,
	.valid   = l4x_pm_plat_valid,
};

struct l4x_virtual_mem_struct {
	struct list_head list;
	unsigned long address, page;
};

static LIST_HEAD(virtual_pages);
static DEFINE_SPINLOCK(virtual_pages_lock);

enum l4x_virtual_mem_type {
	L4X_VIRTUAL_MEM_TYPE_MAP,
	L4X_VIRTUAL_MEM_TYPE_UNMAP,
};

void l4x_virtual_mem_register(unsigned long address, unsigned long page)
{
	struct l4x_virtual_mem_struct *e;
	unsigned long flags;

	if (!(e = kmalloc(sizeof(*e), GFP_KERNEL)))
		BUG();
	e->address = address;
	e->page    = page;
	spin_lock_irqsave(&virtual_pages_lock, flags);
	list_add_tail(&e->list, &virtual_pages);
	spin_unlock_irqrestore(&virtual_pages_lock, flags);
}

void l4x_virtual_mem_unregister(unsigned long address)
{
	struct list_head *p, *tmp;
	unsigned long flags;

	list_for_each_safe(p, tmp, &virtual_pages) {
		struct l4x_virtual_mem_struct *e
		 = list_entry(p, struct l4x_virtual_mem_struct, list);
		if (e->address == address) {
			spin_lock_irqsave(&virtual_pages_lock, flags);
			list_del(p);
			spin_unlock_irqrestore(&virtual_pages_lock, flags);
			kfree(e);
		}
	}
}

static void l4x_virtual_mem_handle_pages(enum l4x_virtual_mem_type t)
{
	struct list_head *p;
	list_for_each(p, &virtual_pages) {
		struct l4x_virtual_mem_struct *e
		 = list_entry(p, struct l4x_virtual_mem_struct, list);

		if (t == L4X_VIRTUAL_MEM_TYPE_MAP) {
			l4x_printf("map virtual %lx -> %lx\n", e->address, e->page);
			l4lx_memory_map_virtual_page(e->address, e->page, 1);
		} else {
			l4x_printf("unmap virtual %lx\n", e->address);
			l4lx_memory_unmap_virtual_page(e->address);
		}
	}
}


static int l4x_pm_suspend(void)
{
#ifndef CONFIG_L4_VCPU
	struct task_struct *p;
	for_each_process(p) {
		if (l4_is_invalid_cap(p->thread.user_thread_id))
			continue;

		// FIXME: destroy better, threads and task
		if (!l4lx_task_delete_task(p->thread.user_thread_id, 1))
			l4x_printf("Error deleting %s(%d)\n", p->comm, p->pid);
		if (l4lx_task_number_free(p->thread.user_thread_id))
			l4x_printf("Error freeing %s(%d)\n", p->comm, p->pid);
		p->thread.user_thread_id = L4_INVALID_CAP;
		l4x_printf("kicked %s(%d)\n", p->comm, p->pid);
	}
#endif

	// Actually only needed for s2d
	l4x_virtual_mem_handle_pages(L4X_VIRTUAL_MEM_TYPE_UNMAP);
	return 0;
}

static void l4x_pm_resume(void)
{
#ifndef CONFIG_L4_VCPU
	struct task_struct *p;
#endif

	// Actually only needed for s2d
	l4x_virtual_mem_handle_pages(L4X_VIRTUAL_MEM_TYPE_MAP);

	// needs fixing for vCPU mode if someone wants that
#ifndef CONFIG_L4_VCPU
	for_each_process(p) {
		l4_msgtag_t tag;
		l4_umword_t src_id;

		if (l4_is_invalid_cap(p->thread.user_thread_id))
			continue;

		if (l4lx_task_get_new_task(L4_INVALID_CAP,
		                           &p->thread.user_thread_id))
			l4x_printf("l4lx_task_get_new_task failed\n");
		if (l4lx_task_create(p->thread.user_thread_id))
			l4x_printf("l4lx_task_create for %s(%d) failed\n",
			           p->comm, p->pid);

		do {
			tag = l4_ipc_wait(l4_utcb(), &src_id, L4_IPC_SEND_TIMEOUT_0);
			if (l4_ipc_error(tag, l4_utcb()))
				l4x_printf("ipc error %lx\n", l4_ipc_error(tag, l4_utcb()));
		} while ( 1 ); //FIXME //!l4_thread_equal(src_id, p->thread.user_thread_id));

		l4x_printf("contacted %s(%d)\n", p->comm, p->pid);
	}
#endif
}

static struct syscore_ops l4x_pm_syscore_ops = {
	.suspend = l4x_pm_suspend,
	.resume  = l4x_pm_resume,
};

static ssize_t wakeup_irq_add_store(struct kobject *kobj,
                                    struct kobj_attribute *attr,
                                    const char *buf, size_t sz)
{
	unsigned irq = simple_strtoul(buf, NULL, 0);
	int r = -EINVAL;

	if (irq < NR_IRQS) {
		if (list_empty(&wakeup_srcs))
			suspend_possible = 1;
		r = l4x_wakeup_source_register(irq);
		if (!r)
			return sz;
	}
	return r;
}

static ssize_t wakeup_irq_del_store(struct kobject *kobj,
                                    struct kobj_attribute *attr,
                                    const char *buf, size_t sz)
{
	unsigned irq = simple_strtoul(buf, NULL, 0);
	int r = -EINVAL;

	if (irq < NR_IRQS) {
		r = l4x_wakeup_source_unregister(irq);
		if (!r)
			r = sz;
		if (list_empty(&wakeup_srcs))
			suspend_possible = 0;
	}
	return r;
}

static ssize_t wakeup_srcs_show(struct kobject *kobj,
                                struct kobj_attribute *attr, char *buf)
{
	struct wakeup_src *s, *tmp;
	ssize_t c = 0;

	list_for_each_entry_safe(s, tmp, &wakeup_srcs, list)
		c += sprintf(buf + c, "%d\n", s->irq);

	return c;
}

static ssize_t wakeup_src_all_store(struct kobject *kobj,
                                    struct kobj_attribute *attr,
                                    const char *buf, size_t sz)
{
	unsigned v = simple_strtoul(buf, NULL, 0);
	if (v & ~1u)
		return -EINVAL;
	suspend_all_irq = v;
	return sz;
}

static ssize_t wakeup_srcs_all_show(struct kobject *kobj,
                                    struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", suspend_all_irq);
}

static struct kobj_attribute wakeup_src_add_attr =
	__ATTR(wakeup_irq_add,    0600, wakeup_srcs_show, wakeup_irq_add_store);
static struct kobj_attribute wakeup_src_del_attr =
	__ATTR(wakeup_irq_remove, 0600, wakeup_srcs_show, wakeup_irq_del_store);
static struct kobj_attribute wakeup_src_all_attr =
	__ATTR(wakeup_irq_all, 0600, wakeup_srcs_all_show, wakeup_src_all_store);


static __init int l4x_pm_init(void)
{
	int r;

	r = sysfs_create_file(power_kobj, &wakeup_src_add_attr.attr);
	if (r)
		printk(KERN_ERR "failed to create sysfs file: %d\n", r);

	r = sysfs_create_file(power_kobj, &wakeup_src_del_attr.attr);
	if (r)
		printk(KERN_ERR "failed to create sysfs file: %d\n", r);

	r = sysfs_create_file(power_kobj, &wakeup_src_all_attr.attr);
	if (r)
		printk(KERN_ERR "failed to create sysfs file: %d\n", r);

	register_syscore_ops(&l4x_pm_syscore_ops);
	suspend_set_ops(&l4x_pm_plat_ops);

	return 0;
}
arch_initcall(l4x_pm_init);
