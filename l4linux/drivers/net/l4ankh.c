/*
 * Interface to the Ankh network multiplexer.
 *
 * (c) 2010 Matthias Lange <mlange@sec.t-labs.tu-berlin.de>
 */

#include <linux/etherdevice.h>
#include <linux/kthread.h>

#include <asm/l4lxapi/misc.h>
#include <asm/l4lxapi/thread.h>

#include <asm/generic/l4lib.h>
#include <asm/generic/setup.h>
#include <asm/generic/do_irq.h>
#include <asm/generic/cap_alloc.h>

#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/util/cap.h>
#include <l4/re/consts.h>
#include <l4/ankh/client-c.h>
#include <l4/ankh/session.h>
#include <l4/log/log.h>

MODULE_AUTHOR("Matthias Lange <mlange@sec.t-labs.tu-berlin.de>, Adam Lackorzynski <adam@os.inf.tu-dresden.de>");
MODULE_DESCRIPTION("L4ankh stub driver");
MODULE_LICENSE("GPL");

static char *_send_buf;
static char *_recv_buf;

static char ankh_shm[64];
static int bufsize = 16384;
static int  l4x_net_numdevs = 1;
#define MAX_NETINST 6
static char *l4x_net_instances[MAX_NETINST] = { "Ankh:eth0", 0, 0, 0, 0, 0 };
static LIST_HEAD(l4x_net_netdevices);

struct task_struct *ts_ankh_receiver;

L4_EXTERNAL_FUNC(l4ankh_init);
L4_EXTERNAL_FUNC(l4ankh_send);
L4_EXTERNAL_FUNC(l4ankh_open);
L4_EXTERNAL_FUNC(l4ankh_get_info);
L4_EXTERNAL_FUNC(l4ankh_prepare_send);
L4_EXTERNAL_FUNC(l4ankh_recv_blocking);
L4_EXTERNAL_FUNC(l4ankh_prepare_recv);

struct l4x_net_priv {
	struct net_device_stats    net_stats;
	l4_cap_idx_t               rx_sig;
	unsigned char              *pkt_buffer;
	unsigned long              pkt_size;
	l4lx_thread_t              irq_thread;
};

struct l4x_net_netdev {
	struct list_head  list;
	struct net_device *dev;
};

static int l4x_net_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct l4x_net_priv *priv = netdev_priv(netdev);

//	printk(KERN_INFO "[L4Ankh] %s, %dbytes\n", __func__, skb->len);

	l4ankh_send(skb->data, skb->len, 1);

	dev_kfree_skb(skb);

	netdev->trans_start = jiffies;
	priv->net_stats.tx_packets++;
	priv->net_stats.tx_bytes += skb->len;

	return 0;
}

static struct net_device_stats *l4x_net_get_stats(struct net_device *netdev)
{
	struct l4x_net_priv *priv = netdev_priv(netdev);
	return &priv->net_stats;
}

static void l4x_net_tx_timeout(struct net_device *netdev)
{
	printk("L4net: %s\n", __func__);
}

/*
 * Interrupt.
 */
static irqreturn_t l4x_net_interrupt(int irq, void *dev_id)
{
	struct net_device *netdev = dev_id;
	struct l4x_net_priv *priv = netdev_priv(netdev);
	struct sk_buff *skb;

	skb = dev_alloc_skb(priv->pkt_size);
	if (likely(skb)) {
		skb->dev = netdev;
		memcpy(skb_put(skb, priv->pkt_size), _recv_buf,
		       priv->pkt_size);
		memset(_recv_buf, 0, priv->pkt_size);
		priv->pkt_size = 0;
		skb->protocol = eth_type_trans(skb, netdev);
		netif_rx(skb);

		netdev->last_rx = jiffies;
		priv->net_stats.rx_bytes += skb->len;
		priv->net_stats.rx_packets++;
	} else {
		printk(KERN_WARNING "%s: dropping packet.\n", netdev->name);
		priv->net_stats.rx_dropped++;
	}

	return IRQ_HANDLED;
}

static unsigned int l4x_net_irq_startup(struct irq_data *data)
{
	printk("L4net: %s\n", __func__);
	return 1;
}

static void l4x_net_irq_dummy_void(struct irq_data *data)
{
}

struct irq_chip l4x_net_irq_type = {
	.name		= "L4net IRQ",
	.irq_startup	= l4x_net_irq_startup,
	.irq_shutdown	= l4x_net_irq_dummy_void,
	.irq_enable	= l4x_net_irq_dummy_void,
	.irq_disable	= l4x_net_irq_dummy_void,
	.irq_mask	= l4x_net_irq_dummy_void,
	.irq_unmask	= l4x_net_irq_dummy_void,
	.irq_ack	= l4x_net_irq_dummy_void,
};

static L4_CV void l4x_ankh_irq_thread(void *data)
{
	struct net_device *netdev = *(struct net_device **)data;
	struct thread_info *ctx = current_thread_info();
	struct l4x_net_priv *priv = netdev_priv(netdev);
	int size;
	int res;

	// LOG_printf("[L4Ankh] l4x_ankh_irq_thread, irq=%d\n", netdev->irq);

	l4x_prepare_irq_thread(ctx, 0);

	res = l4ankh_prepare_recv(L4RE_THIS_TASK_CAP);
	if (res)
		printk("l4ankh_prepare_recv failed with %d\n", res);

	while (1) {
		size = 16384;
		l4ankh_recv_blocking(_recv_buf, &size);
		//LOG_printf("[L4Ankh] Received %d bytes from ankh\n", size);

		priv->pkt_size = size;

		l4x_do_IRQ(netdev->irq, ctx);
	}
}

static int l4x_net_open(struct net_device *netdev)
{
	struct l4x_net_priv *priv = netdev_priv(netdev);
	int err = -ENODEV;

	netif_carrier_off(netdev);

	if (netdev->irq > NR_IRQS) {
		printk(KERN_WARNING "[L4Ankh] %s: irq(%d) > NR_IRQS(%d), failing\n",
		       netdev->name, netdev->irq, NR_IRQS);
		goto err_out_close;
	}

	priv->pkt_buffer = kmalloc(ETH_HLEN + netdev->mtu, GFP_KERNEL);

	if (!priv->pkt_buffer) {
		printk(KERN_WARNING "[L4Ankh] %s kmalloc error\n",
		       netdev->name);
		goto err_out_close;
	}

	if ((err = request_irq(netdev->irq, l4x_net_interrupt,
	                       IRQF_SAMPLE_RANDOM | IRQF_SHARED,
	                       netdev->name, netdev))) {
		printk("[L4Ankh] %s: request_irq(%d, ...) failed: %d\n",
		       netdev->name, netdev->irq, err);
		goto err_out_kfree;
	}

	priv->irq_thread = l4lx_thread_create(l4x_ankh_irq_thread,
	                                      0, NULL, &netdev, sizeof(netdev),
	                                      CONFIG_L4_PRIO_L4ANKH,
	                                      0, "L4AnkhRcv");

	if (!l4lx_thread_is_valid(priv->irq_thread))
		goto err_out_free_irq;

	netif_carrier_on(netdev);
	netif_wake_queue(netdev);

	printk("[L4Ankh] %s interface up.\n", netdev->name);

	return 0;

err_out_free_irq:
	err = -ENODEV;
	free_irq(netdev->irq, netdev);

err_out_kfree:
	kfree(priv->pkt_buffer);

err_out_close:
	return err;
}

static int l4x_net_close(struct net_device *netdev)
{
	struct l4x_net_priv *priv = netdev_priv(netdev);
	free_irq(netdev->irq, netdev);
	netif_stop_queue(netdev);
	netif_carrier_off(netdev);
	kfree(priv->pkt_buffer);
	l4lx_thread_shutdown(priv->irq_thread, 0);
	return 0;
}

/*
 * Split 'inst:foo' into separate strings 'inst' and 'foo'
 */
static int l4x_net_parse_instance(int id, char *inst, int instsize,
                                  char *dev, int devsize)
{
	char *string = l4x_net_instances[id];
	char *s2;

	s2 = strsep(&string, ":");
	if (!s2 || !string)
		return -1;
	strncpy(inst, s2, instsize);
	strncpy(dev,  string, devsize);
	inst[instsize - 1] = 0;
	dev[devsize - 1] = 0;

	return 0;
}

static const struct net_device_ops l4xnet_netdev_ops = {
	.ndo_open       = l4x_net_open,
	.ndo_stop       = l4x_net_close,
	.ndo_start_xmit = l4x_net_xmit_frame,
	.ndo_get_stats  = l4x_net_get_stats,
	.ndo_tx_timeout = l4x_net_tx_timeout,
};

/* Initialize one virtual interface. */
static int __init l4x_net_init_device(char *oreinst, char *devname)
{
	struct l4x_net_priv *priv;
	struct net_device *dev = NULL;
	struct l4x_net_netdev *nd = NULL;
	int err;
	struct AnkhSessionDescriptor *session;

	printk(KERN_INFO "[L4Ankh] %s, ankh_shm=%s\n", __func__, ankh_shm);

	if (l4ankh_init())
		return -ENODEV;

	err = l4ankh_open(ankh_shm, bufsize);
	if (err < 0)
		return -ENODEV;

	session = l4ankh_get_info();

	printk(KERN_INFO "[L4Ankh] after l4ankh_get_info()\n");

	err = l4ankh_prepare_send(L4RE_THIS_TASK_CAP);
	if (err < 0)
		return -ENODEV;

	if (!(dev = alloc_etherdev(sizeof(struct l4x_net_priv)))) {
		printk("[L4Ankh] alloc_etherdev failed\n");
		return -ENOMEM;
	}

	memcpy((void*)dev->dev_addr, &session->mac, sizeof(session->mac));

	_send_buf = kmalloc(16384, GFP_KERNEL);
	_recv_buf = kmalloc(16384, GFP_KERNEL);

	dev->netdev_ops = &l4xnet_netdev_ops;
	priv = netdev_priv(dev);

	priv->rx_sig = L4_INVALID_CAP;

	if (l4_is_invalid_cap(priv->rx_sig = l4x_cap_alloc()))
		return -1;

	if ((dev->irq = l4x_register_irq(priv->rx_sig)) < 0) {
		printk(KERN_INFO "[L4Ankh] Failed to get signal irq\n");
		goto err_out_free_dev;
	}

	if ((err = register_netdev(dev))) {
		printk("[L4Ankh] Cannot register net device, aborting.\n");
		goto err_out_free_dev;
	}

	nd = kmalloc(sizeof(struct l4x_net_netdev), GFP_KERNEL);
	if (!nd) {
		printk("Out of memory.\n");
		err = -ENOMEM;
		goto err_out_free_dev;
	}
	nd->dev = dev;
	list_add(&nd->list, &l4x_net_netdevices);

	printk(KERN_INFO "[L4Ankh] %s Ankh card found  IRQ %d\n",
	                 dev->name, //print_mac(macstring, dev->dev_addr),
	                 dev->irq);

	return 0;

err_out_free_dev:
	free_netdev(dev);
	kfree(_send_buf);
	kfree(_recv_buf);

	return err;
}

static int __init l4x_net_init(void)
{
	int i = 0;
	int num_init = 0;

	if (*ankh_shm == 0)
		return -ENODEV;

	printk("[L4Ankh] Initializing, creating %d Ankh device(s).\n", l4x_net_numdevs);

	for (i = 0; i < l4x_net_numdevs; i++) {
		char instbuf[16], devbuf[16];
		int ret = l4x_net_parse_instance(i, instbuf, sizeof(instbuf),
		                                    devbuf, sizeof(devbuf));
		if (!ret) {
			printk("[L4Ankh] Opening device %s at Ankh instance %s\n",
			       devbuf, instbuf);
			ret = l4x_net_init_device(instbuf, devbuf);
			if (!ret)
				num_init++;
		}
		else
			printk("L4net: Invalid device string: %s\n",
			       l4x_net_instances[i]);
	}

	return num_init > 0 ? 0 : -ENODEV;
}

static void __exit l4x_net_exit(void)
{
	struct list_head *p, *n;
	list_for_each_safe(p, n, &l4x_net_netdevices) {
		struct l4x_net_netdev *nd
		  = list_entry(p, struct l4x_net_netdev, list);
		struct net_device *dev = nd->dev;
		unregister_netdev(dev);
		free_netdev(dev);
		list_del(p);
		kfree(nd);
	}
}

module_init(l4x_net_init);
module_exit(l4x_net_exit);

static int l4x_net_setup(const char *val, struct kernel_param *kp)
{
	strlcpy(ankh_shm, val, sizeof(ankh_shm));
	return 0;
}

module_param_array_named(instances, l4x_net_instances, charp, &l4x_net_numdevs, 0);
MODULE_PARM_DESC(netinstances, "Ankh instances to connect to");
module_param_call(ankh_shm, l4x_net_setup, NULL, NULL, 0200);
MODULE_PARM_DESC(ankh_shm, "Name of shared memory for ankh");
