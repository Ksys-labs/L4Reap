/*
 * L4shm based network driver.
 */

#include <linux/module.h>
#include <linux/etherdevice.h>

#include <asm/l4lxapi/misc.h>
#include <asm/l4lxapi/thread.h>

#include <asm/generic/l4lib.h>
#include <asm/generic/setup.h>
#include <asm/generic/do_irq.h>

#include <l4/shmc/shmc.h>

MODULE_AUTHOR("Adam Lackorzynski <adam@os.inf.tu-dresden.de>");
MODULE_DESCRIPTION("L4shmnet driver");
MODULE_LICENSE("GPL");

enum {
	WAIT_TIMEOUT = 20000,
	NR_OF_DEVS   = 3,
};

static int shmsize = 1 << 20;

static char devs_create[NR_OF_DEVS];
static char devs_to_add_name[NR_OF_DEVS][20];
static char devs_to_add_macpart[NR_OF_DEVS];
static int  devs_to_add_pos;

static LIST_HEAD(l4x_l4shmnet_netdevices);

struct l4x_l4shmc_priv {
	struct net_device_stats    net_stats;

	l4shmc_area_t              shmcarea;
	l4shmc_signal_t            tx_sig;
	l4shmc_signal_t            rx_sig;
	l4shmc_chunk_t             tx_chunk;
	l4shmc_chunk_t             rx_chunk;

	char                      *tx_ring_start;
	char                      *rx_ring_start;
	unsigned long              tx_ring_size;
	unsigned long              rx_ring_size;
};

struct l4x_l4shmc_netdev {
	struct list_head  list;
	struct net_device *dev;
};

struct chunk_head {
	unsigned long next_offs_to_write; // end of ring content
	unsigned long next_offs_to_read;  // start of ring content
	unsigned long writer_blocked;     // ring buffer full
};

struct ring_chunk_head {
	unsigned long size; // 0 == not used,  >= 0 valid chunk
};

static inline int chunk_size(l4shmc_area_t *s)
{
	return (l4shmc_area_size(s) / 2) - 52;
}

static int l4x_l4shmc_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	short length = skb->len;
	struct chunk_head *chhead;
	struct ring_chunk_head *rph;
	unsigned long l, offs, nextoffs, r;
	L4XV_V(f);

	if (length == 0)
		return 0;

	// copy chunk into the ring
	chhead = (struct chunk_head *)l4shmc_chunk_ptr(&priv->tx_chunk);

	offs = chhead->next_offs_to_write;

	rph = (struct ring_chunk_head *)(priv->tx_ring_start + offs);

	BUILD_BUG_ON(sizeof(struct ring_chunk_head) & (sizeof(struct ring_chunk_head) - 1));

	nextoffs = (offs + length + sizeof(struct ring_chunk_head) + sizeof(struct ring_chunk_head) - 1)
	           & ~(sizeof(struct ring_chunk_head) - 1);

	r = chhead->next_offs_to_read;
	if (r <= offs)
		r += priv->tx_ring_size;
	if (nextoffs >= r) {
		chhead->writer_blocked = 1;
		netif_stop_queue(netdev);
		return 1;
	}

	nextoffs %= priv->tx_ring_size;

	offs += sizeof(struct ring_chunk_head);
	offs %= priv->tx_ring_size;

	if (offs + length > priv->tx_ring_size)
		l = priv->tx_ring_size - offs;
	else
		l = length;

	memcpy(priv->tx_ring_start + offs, (char *)skb->data, l);
	if (l != length)
		memcpy(priv->tx_ring_start, (char *)skb->data + l, length - l);

	// now write to shm
	rph->size = length;
	rph = (struct ring_chunk_head *)(priv->tx_ring_start + nextoffs);
	rph->size = 0;
	chhead->next_offs_to_write = nextoffs;
	wmb();

	L4XV_L(f);
	l4shmc_trigger(&priv->tx_sig);
	L4XV_U(f);

	netdev->trans_start = jiffies;
	priv->net_stats.tx_packets++;
	priv->net_stats.tx_bytes += skb->len;

	dev_kfree_skb(skb);

	return 0;
}

static struct net_device_stats *l4x_l4shmc_get_stats(struct net_device *netdev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	return &priv->net_stats;
}

/*
 * Interrupt.
 */
static irqreturn_t l4x_l4shmc_interrupt(int irq, void *dev_id)
{
	struct net_device *netdev = dev_id;
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	struct sk_buff *skb;
	struct chunk_head *chhead;
	struct ring_chunk_head *rph;
	unsigned long offs;

	chhead = (struct chunk_head *)l4shmc_chunk_ptr(&priv->tx_chunk);
	if (chhead->writer_blocked) {
		chhead->writer_blocked = 0;
		netif_wake_queue(netdev);
	}

	chhead = (struct chunk_head *)l4shmc_chunk_ptr(&priv->rx_chunk);
	offs = chhead->next_offs_to_read;
	while (1) {
		unsigned long l;
		char *p;

		rph = (struct ring_chunk_head *)(priv->rx_ring_start + offs);

		if (!rph->size)
			break;

		skb = dev_alloc_skb(rph->size);

		if (unlikely(!skb)) {
			printk(KERN_WARNING "%s: dropping packet (%ld).\n",
			       netdev->name, rph->size);
			priv->net_stats.rx_dropped++;
			break;
		}

		skb->dev = netdev;

		offs += sizeof(struct ring_chunk_head);
		offs %= priv->rx_ring_size;

		if (offs + rph->size > priv->rx_ring_size)
			l = priv->rx_ring_size - offs;
		else
			l = rph->size;

		skb_reserve(skb, NET_IP_ALIGN);
		p = skb_put(skb, rph->size);
		memcpy(p, priv->rx_ring_start + offs, l);
		if (l != rph->size)
			memcpy(p + l, priv->rx_ring_start, rph->size - l);

	        offs = (offs + rph->size + sizeof(struct ring_chunk_head) - 1)
	               & ~(sizeof(struct ring_chunk_head) - 1);
		offs %= priv->rx_ring_size;
		chhead->next_offs_to_read = offs;
		rph->size = 0;

		skb->protocol = eth_type_trans(skb, netdev);
		netif_rx(skb);

		netdev->last_rx = jiffies;
		priv->net_stats.rx_bytes += skb->len;
		priv->net_stats.rx_packets++;
	}

	if (chhead->writer_blocked) {
		L4XV_V(f);
		L4XV_L(f);
		l4shmc_trigger(&priv->tx_sig);
		L4XV_U(f);
	}

	return IRQ_HANDLED;
}

static int l4x_l4shmc_open(struct net_device *netdev)
{
	int err;

	netif_carrier_off(netdev);

	if ((err = request_irq(netdev->irq, l4x_l4shmc_interrupt,
	                       IRQF_SAMPLE_RANDOM | IRQF_SHARED,
	                       netdev->name, netdev))) {
		printk("%s: request_irq(%d, ...) failed: %d\n",
		       netdev->name, netdev->irq, err);
		return err;
	}


	netif_carrier_on(netdev);
	netif_wake_queue(netdev);

	return 0;
}

static int l4x_l4shmc_close(struct net_device *netdev)
{
	free_irq(netdev->irq, netdev);
	netif_stop_queue(netdev);
	netif_carrier_off(netdev);

	return 0;
}

static int l4x_l4shmc_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);

	if (new_mtu > chunk_size(&priv->shmcarea) - 100)
		return -EINVAL;

	netdev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops l4shmnet_netdev_ops = {
	.ndo_open       = l4x_l4shmc_open,
	.ndo_start_xmit = l4x_l4shmc_xmit_frame,
	.ndo_stop       = l4x_l4shmc_close,
	.ndo_change_mtu = l4x_l4shmc_change_mtu,
	.ndo_get_stats  = l4x_l4shmc_get_stats,

};

static int __init l4x_l4shmnet_init_dev(int num, const char *name)
{
	struct l4x_l4shmc_priv *priv;
	struct net_device *dev = NULL;
	struct l4x_l4shmc_netdev *nd = NULL;
	struct chunk_head *ch;
	int err;
	L4XV_V(f);

	if (shmsize < PAGE_SIZE)
		shmsize = PAGE_SIZE;

	if (!(dev = alloc_etherdev(sizeof(struct l4x_l4shmc_priv))))
		return -ENOMEM;

	dev->netdev_ops = &l4shmnet_netdev_ops,
	priv = netdev_priv(dev);

	printk("%s: Requesting, role %s, Shmsize %d Kbytes\n",
	       name,
               devs_create[num] ? "Creator" : "User", shmsize >> 10);

	L4XV_L(f);
	err = -ENOMEM;
	if (devs_create[num]) {
		if (l4shmc_create(name, shmsize))
			goto err_out_free_dev_unlock;
	}

	// we block very long here, don't do that
	if (l4shmc_attach_to(name, WAIT_TIMEOUT, &priv->shmcarea))
		goto err_out_free_dev_unlock;

	if (l4shmc_add_chunk(&priv->shmcarea, devs_create[num] ? "joe" : "bob",
	                     chunk_size(&priv->shmcarea), &priv->tx_chunk))
		goto err_out_free_dev_unlock;

	if (l4shmc_add_signal(&priv->shmcarea, devs_create[num] ? "joe" : "bob",
	                      &priv->tx_sig))
		goto err_out_free_dev_unlock;

	if (l4shmc_connect_chunk_signal(&priv->tx_chunk, &priv->tx_sig))
		goto err_out_free_dev_unlock;

	/* Now get the receiving side */
	if (l4shmc_get_chunk_to(&priv->shmcarea, devs_create[num] ? "bob" : "joe",
	                        WAIT_TIMEOUT, &priv->rx_chunk)) {
		printk("%s: Did not find other side\n", name);
		goto err_out_free_dev_unlock;
	}

	if (l4shmc_get_signal_to(&priv->shmcarea, devs_create[num] ? "bob" : "joe",
	                         WAIT_TIMEOUT, &priv->rx_sig)) {
		printk("%s: Could not get signal\n", name);
		goto err_out_free_dev_unlock;
	}
	if (l4shmc_connect_chunk_signal(&priv->rx_chunk, &priv->rx_sig))
		goto err_out_free_dev_unlock;
	L4XV_U(f);

	ch = (struct chunk_head *)l4shmc_chunk_ptr(&priv->tx_chunk);
	ch->next_offs_to_write = 0;
	ch->next_offs_to_read  = 0;
	ch->writer_blocked     = 0;

	priv->tx_ring_size = l4shmc_chunk_capacity(&priv->tx_chunk)
	                       - sizeof(struct chunk_head);
	priv->rx_ring_size = l4shmc_chunk_capacity(&priv->rx_chunk)
	                       - sizeof(struct chunk_head);

	priv->tx_ring_start = (char *)l4shmc_chunk_ptr(&priv->tx_chunk)
	                       + sizeof(struct chunk_head);
	priv->rx_ring_start = (char *)l4shmc_chunk_ptr(&priv->rx_chunk)
	                       + sizeof(struct chunk_head);

	dev->dev_addr[0] = 0x52;
	dev->dev_addr[1] = 0x54;
	dev->dev_addr[2] = 0x00;
	dev->dev_addr[3] = 0xb0;
	dev->dev_addr[4] = 0xcf;
	dev->dev_addr[5] = devs_to_add_macpart[num];

	if ((dev->irq = l4x_register_irq(l4shmc_signal_cap(&priv->rx_sig))) < 0) {
		printk("Failed to get virq\n");
		goto err_out_free_dev;
	}

	dev->mtu = 1500;

	err = -ENODEV;
	if ((err = register_netdev(dev))) {
		printk("l4ore: Cannot register net device, aborting.\n");
		goto err_out_free_dev;
	}

	nd = kmalloc(sizeof(struct l4x_l4shmc_netdev), GFP_KERNEL);
	if (!nd) {
		printk("Out of memory.\n");
		goto err_out_free_dev;
	}
	nd->dev = dev;
	list_add(&nd->list, &l4x_l4shmnet_netdevices);

	printk(KERN_INFO "%s: L4ShmNet established, with %pM, IRQ %d\n",
	                 dev->name, dev->dev_addr, dev->irq);

	return 0;

err_out_free_dev_unlock:
	L4XV_U(f);
err_out_free_dev:
	printk(KERN_INFO "%s: Failed to establish communication\n", name);
	free_netdev(dev);

	return err;
}

static int __init l4x_l4shmnet_init(void)
{
	int i, ret = -ENODEV;

	for (i = 0; i < devs_to_add_pos; ++i)
		if (*devs_to_add_name[i]
		    && !l4x_l4shmnet_init_dev(i, devs_to_add_name[i]))
			ret = 0;
	return ret;
}

static void __exit l4x_l4shmnet_exit(void)
{
	struct list_head *p, *n;
	list_for_each_safe(p, n, &l4x_l4shmnet_netdevices) {
		struct l4x_l4shmc_netdev *nd
		  = list_entry(p, struct l4x_l4shmc_netdev, list);
		struct net_device *dev = nd->dev;

		unregister_netdev(dev);
		free_netdev(dev);
		l4x_unregister_irq(dev->irq);
		list_del(p);
		kfree(nd);
	}
}

module_init(l4x_l4shmnet_init);
module_exit(l4x_l4shmnet_exit);

/* This function is called much earlier than module_init, esp. there's
 * no kmalloc available */
static int l4x_l4shmnet_setup(const char *val, struct kernel_param *kp)
{
	int l;
	char *c;
	if (devs_to_add_pos >= NR_OF_DEVS) {
		printk("l4shmnet: Too many devices specified, max %d\n",
		       NR_OF_DEVS);
		return 1;
	}
	l = strlen(val) + 1;
	if (l > sizeof(devs_to_add_name[devs_to_add_pos]))
		l = sizeof(devs_to_add_name[devs_to_add_pos]);
	c = strchr(val, ',');
	if (c) {
		l = c - val + 1;
		do {
			if (!strncmp(c + 1, "create", 6))
				devs_create[devs_to_add_pos] = 1;
			else if (!strncmp(c + 1, "macpart=", 8)) {
				devs_to_add_macpart[devs_to_add_pos]
				  = simple_strtoul(c + 9, NULL, 0);
			}
		} while ((c = strchr(c + 1, ',')));
	}
	strlcpy(devs_to_add_name[devs_to_add_pos], val, l);
	devs_to_add_pos++;
	return 0;
}

module_param_call(add, l4x_l4shmnet_setup, NULL, NULL, 0200);
MODULE_PARM_DESC(add, "Use l4shmnet.add=name,macpart[,create] to add a device, name queried in namespace");

module_param(shmsize, int, 0);
MODULE_PARM_DESC(shmsize, "Size of the shared memory area");
