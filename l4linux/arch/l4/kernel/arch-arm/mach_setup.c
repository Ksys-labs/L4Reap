/*
 * Device specific code.
 */

#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/smsc911x.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <asm/generic/devs.h>

#include <asm/l4x/dma.h>


static int dev_init_done;

static LIST_HEAD(platform_callbacks_head);

struct l4x_platform_callback_elem * __init
l4x_register_plat_dev_cb_e(const char *name, l4x_dev_cb_func_type cb)
{
	struct l4x_platform_callback_elem *e;

	if (dev_init_done) {
		printk(KERN_ERR "Registering of platform callback %p is too late!\n",
		                cb);
		return ERR_PTR(-EINVAL);
	}

	e = kzalloc(sizeof(*e), GFP_KERNEL);
	if (!e)
		return ERR_PTR(-ENOMEM);

	e->cb   = cb;
	e->name = name;
	list_add(&e->list, &platform_callbacks_head);

	return e;
}

int __init
l4x_register_platform_device_callback(const char *name, l4x_dev_cb_func_type cb)
{
	struct l4x_platform_callback_elem *e
		= l4x_register_plat_dev_cb_e(name, cb);
	if (IS_ERR(e))
		return PTR_ERR(e);
	return 0;
}

static void __init free_platform_callback_elems(void)
{
	struct l4x_platform_callback_elem *e, *r;
	list_for_each_entry_safe(e, r, &platform_callbacks_head, list) {
		list_del(&e->list);
		kfree(e);
	}
}

static void copy_platform_device(l4io_device_handle_t dh,
                                 l4io_resource_handle_t rh,
                                 l4io_device_t *dev, int id,
                                 void (*platdev_cb)(struct platform_device *))
{
	l4io_resource_t res;
	struct platform_device *platdev;
	struct resource *platres;
	int i;
	L4XV_V(f);

	platdev = platform_device_alloc(dev->name, id);
	if (!platdev)
		return;

	platres = kzalloc(sizeof(*platres) * dev->num_resources, GFP_KERNEL);
	if (!platres)
		return;

	platdev->num_resources     = dev->num_resources;
	platdev->resource          = platres;
	if (platdev_cb)
		platdev_cb(platdev);

	i = 0;
	L4XV_L(f);
	while (!l4io_lookup_resource(dh, L4IO_RESOURCE_ANY,
	                             &rh, &res)) {
		L4XV_U(f);
		BUG_ON(i > platdev->num_resources);

		switch (res.type) {
			case L4IO_RESOURCE_IRQ:
				platres[i].flags = IORESOURCE_IRQ;
				break;
			case L4IO_RESOURCE_MEM:
				platres[i].flags = IORESOURCE_MEM;
				break;
			case L4IO_RESOURCE_PORT:
				platres[i].flags = IORESOURCE_IO;
				break;
			default:
				printk("Platform device: %s: Type %x unknown\n",
				       dev->name, res.type);
				platres[i].flags = IORESOURCE_DISABLED;
				break;
		};

		platres[i].start = res.start;
		platres[i].end   = res.end;
		++i;
		L4XV_L(f);
	}
	L4XV_U(f);

	if (platform_device_add(platdev)) {
		printk("Adding of static platform device '%s' failed.\n",
		       dev->name);
		return;
	}

	printk("Added static device '%s' with %d resources.\n",
	       dev->name, platdev->num_resources);
}

static L4X_DEVICE_CB(default_copy_and_add_platform_device)
{
	int id;
	id = (int)cb_node->privdata;
	cb_node->privdata = (void *)(id + 1);

	copy_platform_device(dh, rh, dev, id, NULL);
}

/* Any additional platform_data information goes here */
static struct pata_platform_info pata_platform_data = {
	.ioport_shift           = 1,
};

static struct smsc911x_platform_config smsc911x_config = {
	.flags          = SMSC911X_USE_32BIT,
	.irq_polarity   = SMSC911X_IRQ_POLARITY_ACTIVE_HIGH,
	.irq_type       = SMSC911X_IRQ_TYPE_PUSH_PULL,
	.phy_interface  = PHY_INTERFACE_MODE_MII,
};

static void realview_device_cb_pata_pd(struct platform_device *d)
{
	d->name              = "pata_platform";
	d->dev.platform_data = &pata_platform_data;
}

static L4X_DEVICE_CB(realview_device_cb_pata)
{
	copy_platform_device(dh, rh, dev, 0, realview_device_cb_pata_pd);
}

static void realview_device_cb_smsc_pd(struct platform_device *d)
{
	d->name              = "smsc911x";
	d->dev.platform_data = &smsc911x_config;
}

static L4X_DEVICE_CB(realview_device_cb_smsc)
{
	copy_platform_device(dh, rh, dev, 0, realview_device_cb_smsc_pd);
}

#include <linux/amba/bus.h>
#include <asm/irq.h>

static struct amba_device aacidev = {
	.dev = {
		.coherent_dma_mask = ~0,
		.init_name = "busid",
		.platform_data = NULL,
	},
	.res = {
		.start = 0x10004000,
		.end   = 0x10004fff,
		.flags  = IORESOURCE_MEM,
	},
	.dma_mask       = ~0,
	.irq            = {32, NO_IRQ },
};

static L4X_DEVICE_CB(aaci_cb)
{
	printk("Adding AACI as AMBA device\n");
	amba_device_register(&aacidev, &iomem_resource);
}

static L4X_DEVICE_CB(dmamem_cb)
{
	l4io_resource_t res;
	unsigned long v, s;
	L4XV_V(f);

	L4XV_L(f);
	if (l4io_lookup_resource(dh, L4IO_RESOURCE_MEM,
	                         &rh, &res)) {
		L4XV_U(f);
		printk(KERN_ERR "No DMA memory?\n");
		return;
	}

	if (l4io_request_iomem(res.start, res.end - res.start,
	                       L4IO_MEM_NONCACHED | L4IO_MEM_EAGER_MAP,
	                       &v)) {
		L4XV_U(f);
		printk(KERN_ERR "Could not get DMA MEM\n");
		return;
	}
	L4XV_U(f);

	s = res.end - res.start + 1;

	printk("DMA mem phys at %08lx - %08lx\n", res.start, res.end);
	printk("DMA mem virt at %08lx - %08lx\n", v, v + s - 1);

	if (l4x_dma_mem_add(v, res.start, s))
		printk("Adding DMA memory to DMA allocator failed!\n");
}

static void register_platform_callbacks(void)
{
	l4x_register_platform_device_callback("compactflash", realview_device_cb_pata);
	l4x_register_platform_device_callback("smsc911x",     realview_device_cb_smsc);
	l4x_register_platform_device_callback("aaci",         aaci_cb);
	l4x_register_platform_device_callback("dmamem",       dmamem_cb);
}

static void
add_and_call_default_cb(l4io_device_t *dev,
                        l4io_device_handle_t dh,
                        l4io_resource_handle_t rh)
{
	struct l4x_platform_callback_elem *e;
	e = l4x_register_plat_dev_cb_e(dev->name,
	                               default_copy_and_add_platform_device);
	if (IS_ERR(e))
		return;

	default_copy_and_add_platform_device(dh, rh, dev, e);
}

void __init l4x_arm_devices_init(void)
{
	l4io_device_handle_t dh;
	l4io_device_t dev;
	l4io_resource_handle_t rh;
	L4XV_V(f);

	register_platform_callbacks();

	L4XV_L(f);
	dh = l4io_get_root_device();
	while (!l4io_iterate_devices(&dh, &dev, &rh)) {
		struct l4x_platform_callback_elem *e;
		L4XV_U(f);

		list_for_each_entry(e, &platform_callbacks_head, list)
			if (!strcmp(e->name, dev.name)) {
				e->cb(dh, rh, &dev, e);
				break;
			}

		if (*dev.name == '\0')
			strlcpy(dev.name, "unnamed", sizeof(dev.name));

		if (&e->list == &platform_callbacks_head) {
			add_and_call_default_cb(&dev, dh, rh);
		}
		L4XV_L(f);
	}
	L4XV_U(f);

	dev_init_done = 1;
	free_platform_callback_elems();
}
