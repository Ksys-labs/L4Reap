#include <linux/pci.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/pci.h>

#include <l4/vbus/vbus.h>
#include <l4/vbus/vbus_pci.h>

#include <asm/l4lxapi/irq.h>
#include <asm/generic/l4lib.h>
#include <asm/generic/cap_alloc.h>
#include <asm/generic/vcpu.h>

#include <l4/re/c/namespace.h>

L4_EXTERNAL_FUNC(l4vbus_pci_irq_enable);
L4_EXTERNAL_FUNC(l4vbus_pci_cfg_read);
L4_EXTERNAL_FUNC(l4vbus_pci_cfg_write);
L4_EXTERNAL_FUNC(l4vbus_get_device_by_hid);

static l4_cap_idx_t vbus;
static l4vbus_device_handle_t root_bridge;

unsigned int pci_probe;
unsigned int pci_early_dump_regs;
int noioapicquirk;
int noioapicreroute = 0;
int pci_routeirq;

/*
 * l4vpci_pci_irq_enable
 * success: return 0
 * failure: return < 0
 */

int l4vpci_irq_enable(struct pci_dev *dev)
{
	unsigned char trigger, polarity;
	int irq;
	u8 pin = 0;
	unsigned flags;
	l4_uint32_t devfn;
	L4XV_V(f);

	if (!dev)
		return -EINVAL;

	pin = dev->pin;
	if (!pin) {
		dev_warn(&dev->dev,
		         "No interrupt pin configured for device %s\n",
		         pci_name(dev));
		return 0;
	}
	pin--;


	if (!dev->bus) {
		dev_err(&dev->dev, "invalid (NULL) 'bus' field\n");
		return -ENODEV;
	}

	L4XV_L(f);

	devfn = (PCI_SLOT(dev->devfn) << 16) | PCI_FUNC(dev->devfn);
	irq = l4vbus_pci_irq_enable(vbus, root_bridge, dev->bus->number, devfn, pin, &trigger, &polarity);
	if (irq < 0) {
		dev_warn(&dev->dev, "PCI INT %c: no GSI", 'A' + pin);
		/* Interrupt Line values above 0xF are forbidden */
		return 0;
	}
	L4XV_U(f);

	switch ((!!trigger) | ((!!polarity) << 1)) {
		case 0: flags = IRQF_TRIGGER_HIGH; break;
		case 1: flags = IRQF_TRIGGER_RISING; break;
		case 2: flags = IRQF_TRIGGER_LOW; break;
		case 3: flags = IRQF_TRIGGER_FALLING; break;
		default: flags = 0; break;
	}

	dev->irq = irq;
	l4lx_irq_set_type(irq_get_irq_data(irq), flags);

	dev_info(&dev->dev, "PCI INT %c -> GSI %u (%s, %s) -> IRQ %d\n",
	         'A' + pin, irq,
	         !trigger ? "level" : "edge",
	         polarity ? "low" : "high", dev->irq);

	return 0;
}

void l4vpci_irq_disable(struct pci_dev *dev)
{
	printk("%s: implement me\n", __func__);
}

/*
 * Functions for accessing PCI base (first 256 bytes) and extended
 * (4096 bytes per PCI function) configuration space with type 1
 * accesses.
 */

static int pci_conf1_read(unsigned int seg, unsigned int bus,
                         unsigned int devfn, int reg, int len, u32 *value)
{
	l4_uint32_t df = (PCI_SLOT(devfn) << 16) | PCI_FUNC(devfn);
	int r;
	L4XV_V(f);
	L4XV_L(f);
	r = l4vbus_pci_cfg_read(vbus, root_bridge, bus, df, reg, value, len * 8);
	L4XV_U(f);
	return r;
}

static int pci_conf1_write(unsigned int seg, unsigned int bus,
                           unsigned int devfn, int reg, int len, u32 value)
{
	l4_uint32_t df = (PCI_SLOT(devfn) << 16) | PCI_FUNC(devfn);
	int r;
	L4XV_V(f);
	L4XV_L(f);
	r = l4vbus_pci_cfg_write(vbus, root_bridge, bus, df, reg, value, len * 8);
	L4XV_U(f);
	return r;
}

static int pci_read(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *value)
{
	return pci_conf1_read(pci_domain_nr(bus), bus->number,
	                      devfn, where, size, value);
}

static int pci_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 value)
{
	return pci_conf1_write(pci_domain_nr(bus), bus->number,
	                       devfn, where, size, value);
}

static struct pci_ops l4vpci_ops = {
	.read = pci_read,
	.write = pci_write,
};

static int __init l4vpci_init(void)
{
	struct pci_dev *dev = NULL;
#ifdef CONFIG_ARM
	struct pci_sys_data *sd;
#else
	struct pci_sysdata *sd;
#endif
	int err;
	L4XV_V(f);

	vbus = l4re_get_env_cap("vbus");
	if (l4_is_invalid_cap(vbus))
		return -ENOENT;

	L4XV_L(f);

	err = l4vbus_get_device_by_hid(vbus, 0, &root_bridge, "PNP0A03", 0, 0);
	if (err < 0) {
		printk(KERN_INFO "PCI: no root bridge found, no PCI\n");
		L4XV_U(f);
		return err;
	}

	L4XV_U(f);

	printk(KERN_INFO "PCI: L4 root bridge is device %lx\n", root_bridge);

	sd = kzalloc(sizeof(*sd), GFP_KERNEL);
	if (!sd)
		return -ENOMEM;

	pci_scan_bus(0, &l4vpci_ops, sd);

	printk(KERN_INFO "PCI: Using L4-IO for IRQ routing\n");

	for_each_pci_dev(dev)
		l4vpci_irq_enable(dev);

#ifdef CONFIG_X86
	pcibios_resource_survey();
#endif

	return 0;
}

int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	int err;

	if ((err = pci_enable_resources(dev, mask)) < 0)
		return err;

	return l4vpci_irq_enable(dev);
}

#ifdef CONFIG_X86
unsigned int pcibios_assign_all_busses(void)
{
	return 1;
}
#endif

#ifdef CONFIG_ARM
int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
                        enum pci_mmap_state mmap_state, int write_combine)
{
	printk("%s %d\n", __func__, __LINE__);
	return -ENOENT;
}

void
pcibios_resource_to_bus(struct pci_dev *dev, struct pci_bus_region *region,
                        struct resource *res)
{
	printk("%s %d\n", __func__, __LINE__);
}

void __devinit
pcibios_bus_to_resource(struct pci_dev *dev, struct resource *res,
                        struct pci_bus_region *region)
{
	printk("%s %d\n", __func__, __LINE__);
}

resource_size_t pcibios_align_resource(void *data, const struct resource *res,
                                       resource_size_t size, resource_size_t align)
{
	printk("%s %d\n", __func__, __LINE__);
	return 0;
}

void __devinit pcibios_update_irq(struct pci_dev *dev, int irq)
{
	printk("%s %d\n", __func__, __LINE__);
}
#endif

int early_pci_allowed(void)
{
	return 0;
}

void early_dump_pci_devices(void)
{
	printk("%s: unimplemented\n", __func__);
}

u32 read_pci_config(u8 bus, u8 slot, u8 func, u8 offset)
{
	printk("%s: unimplemented\n", __func__);
	return 0;
}

u8 read_pci_config_byte(u8 bus, u8 slot, u8 func, u8 offset)
{
	printk("%s: unimplemented\n", __func__);
	return 0;
}

u16 read_pci_config_16(u8 bus, u8 slot, u8 func, u8 offset)
{
	printk("%s: unimplemented\n", __func__);
	return 0;
}

void write_pci_config(u8 bus, u8 slot, u8 func, u8 offset, u32 val)
{
	printk("%s: unimplemented\n", __func__);
}

char * __devinit  pcibios_setup(char *str)
{
	return str;
}

void pcibios_disable_device (struct pci_dev *dev)
{
	l4vpci_irq_disable(dev);
}

/*
 *  Called after each bus is probed, but before its children
 *  are examined.
 */
void __devinit  pcibios_fixup_bus(struct pci_bus *b)
{
	pci_read_bridge_bases(b);
}

int __init pci_legacy_init(void)
{
	return 0;
}

void __init pcibios_irq_init(void)
{
}

void __init pcibios_fixup_irqs(void)
{
}

subsys_initcall(l4vpci_init);
