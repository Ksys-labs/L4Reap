/*
 * L4shm based network driver.
 */

#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>

#include <asm/l4lxapi/misc.h>
#include <asm/l4lxapi/thread.h>

#include <asm/generic/l4lib.h>
#include <asm/generic/setup.h>
#include <asm/generic/do_irq.h>

#include <l4/shmc/shmc.h>
#include <l4/shmc/shmbuf.h>

MODULE_AUTHOR("Adam Lackorzynski <adam@os.inf.tu-dresden.de>");
MODULE_DESCRIPTION("L4shmnet driver");
MODULE_LICENSE("GPL");

enum {
	WAIT_TIMEOUT = 20000,
	NR_OF_DEVS   = 3,
};

static int shmsize = 1 << 20;

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

	struct l4shm_buf           sb;
	unsigned int               num:8;
	unsigned int               remote_attached:1;
	unsigned int               link_up:1;
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

/****************************************************************************
 * Protocol for attaching:
 *
 *   1. Both parties will try to create the shm segment. The first one wins.
 *   2. Both parties create a chunk and sig named after their MAC address
 *      and use it as their rx side. They will set the chunk state to "clear"
 *      which means "link down".
 *   3. Both parties will look for another chunk. If there is one, they will
 *      connect it as their tx side and trigger that signal.
 *   4. The first one won't yet find the other's chunk but he will be alerted
 *      by the second's signal and then try again. He will then connect the
 *      chunk and sig as his tx side, and trigger the signal.
 *   Due to the signal, there is no need to actively poll for the arrival of
 *   the second partner.
 *   If one partner wants to set the interface up, he sets the state of his
 *   rx chunk to "ready" and triggers the tx signal. If both chunks are
 *   "ready" the link state is defined to be "up".
 *   The interface state can be set down again in the same way.
 *   The MAC addresses of both parties must be different.
 ****************************************************************************/

static inline int chunk_size(l4shmc_area_t *s)
{
	return (l4shmc_area_size(s) - l4shmc_area_overhead()) / 2 -
	       l4shmc_chunk_overhead();
}

static int init_dev_self(struct net_device *dev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(dev);
	int ret;
	char myname[15];

	snprintf(myname, sizeof(myname), "%pm", dev->dev_addr);

	if ((ret = l4shmc_add_chunk(&priv->shmcarea, myname,
	                            chunk_size(&priv->shmcarea),
	                            &priv->rx_chunk))) {
		if (ret != -L4_EEXIST) {
			dev_warn(&dev->dev, "l4shmnet: Can't create chunk: %d", ret);
			return ret;
		}
		if ((ret = l4shmc_get_chunk(&priv->shmcarea, myname,
		                            &priv->rx_chunk))) {
			dev_warn(&dev->dev, "l4shmnet: Can't attach to existing chunk '%s': %d",
			         myname, ret);
			return ret;
		}
	}

	/*
	 * The chunk ready / chunk cleared flag is abused as ifup/ifdown flag:
	 * cleared == consumed  -> if down
	 * ready                -> if up
	 */
	l4shmc_chunk_consumed(&priv->rx_chunk);

	if ((ret = l4shmc_add_signal(&priv->shmcarea, myname, &priv->rx_sig)))
		return ret;

	if ((ret = l4shmc_connect_chunk_signal(&priv->rx_chunk, &priv->rx_sig)))
		return ret;

	return 0;
}

static int init_dev_other(struct net_device *dev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(dev);
	char myname[15];
	char othername[15];
	const char *ptr;
	long offs = 0;
	int ret;
	L4XV_V(f);

	othername[0] = '\0';
	snprintf(myname, sizeof(myname), "%pm", dev->dev_addr);
	L4XV_L(f);
	while ((offs = l4shmc_iterate_chunk(&priv->shmcarea, &ptr, offs)) > 0) {
		if (strncmp(ptr, myname, sizeof(myname)) != 0) {
			strncpy(othername, ptr, sizeof(othername));
			if (othername[sizeof(othername)-1] != '\0') {
				/* name too long or invalid */
				L4XV_U(f);
				return -L4_EIO;
			}
		}
	}
	if (offs < 0) {
		L4XV_U(f);
		return offs;
	}
	if (!othername[0]) {
		L4XV_U(f);
		return -L4_EAGAIN;
	}

	if ((ret = l4shmc_get_chunk(&priv->shmcarea, othername,
				    &priv->tx_chunk))) {
		L4XV_U(f);
		return ret;
	}

	if ((ret = l4shmc_get_signal_to(&priv->shmcarea, othername,
					WAIT_TIMEOUT, &priv->tx_sig))) {
		L4XV_U(f);
		return ret;
	}

	l4shm_buf_init(&priv->sb, l4shmc_chunk_ptr(&priv->rx_chunk),
				l4shmc_chunk_capacity(&priv->rx_chunk),
				l4shmc_chunk_ptr(&priv->tx_chunk),
				l4shmc_chunk_capacity(&priv->tx_chunk));

	priv->remote_attached = 1;
	l4shmc_trigger(&priv->tx_sig);
	L4XV_U(f);
	dev_info(&dev->dev, "%s: L4ShmNet established, with %pM, IRQ %d\n",
	         dev->name, dev->dev_addr, dev->irq);
	return 0;
}

static void update_carrier(struct net_device *dev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(dev);
	int link_up = 0;

	if (priv->remote_attached &&
	    l4shmc_is_chunk_ready(&priv->tx_chunk) &&
	    l4shmc_is_chunk_ready(&priv->rx_chunk))
		link_up = 1;
	if (priv->link_up == link_up)
		return;
	priv->link_up = link_up;
	if (link_up) {
		netif_carrier_on(dev);
		netif_wake_queue(dev);
	}
	else {
		netif_carrier_off(dev);
		netif_stop_queue(dev);
	}
}

static int l4x_l4shmc_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	int ret;
	L4XV_V(f);

	ret = l4shm_buf_tx(&priv->sb, skb->data, skb->len);
	if (ret == -L4_EAGAIN) {
		netif_stop_queue(netdev);
		return 1;
	}
	/* XXX: handle other l4shm_buf_tx errors */

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
	unsigned long len;
	int ret;

	if (!priv->remote_attached)
		init_dev_other(netdev);

	update_carrier(netdev);
	/* XXX: return if link down? */

	chhead = (struct chunk_head *)l4shmc_chunk_ptr(&priv->tx_chunk);
	if (chhead->writer_blocked) {
		chhead->writer_blocked = 0;
		netif_wake_queue(netdev);
	}

	chhead = (struct chunk_head *)l4shmc_chunk_ptr(&priv->rx_chunk);

	while ((len = l4shm_buf_rx_len(&priv->sb)) > 0) {
		char *p;

		/* NET_IP_ALIGN is non-zero on some architectures */
		skb = dev_alloc_skb(len + NET_IP_ALIGN);

		if (unlikely(!skb)) {
			dev_warn(&netdev->dev, "%s: dropping packet (%ld).\n",
			         netdev->name, len);
			priv->net_stats.rx_dropped++;
			break;
		}

		skb->dev = netdev;

		skb_reserve(skb, NET_IP_ALIGN);
		p = skb_put(skb, len);
		ret = l4shm_buf_rx(&priv->sb, p, len);
		/* XXX: check errors */
		skb->protocol = eth_type_trans(skb, netdev);
		netif_rx(skb);

		netdev->last_rx = jiffies;
		priv->net_stats.rx_bytes += skb->len;
		priv->net_stats.rx_packets++;
	}

	if (len == -L4_EAGAIN) {
		L4XV_V(f);
		L4XV_L(f);
		l4shmc_trigger(&priv->tx_sig);
		L4XV_U(f);
	}
	/* XXX: handle other errors */

	return IRQ_HANDLED;
}

static int l4x_l4shmc_open(struct net_device *netdev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	int err;
	L4XV_V(f);

	netif_carrier_off(netdev);

	if ((err = request_irq(netdev->irq, l4x_l4shmc_interrupt,
	                       IRQF_SAMPLE_RANDOM | IRQF_SHARED,
	                       netdev->name, netdev))) {
		dev_err(&netdev->dev, "%s: request_irq(%d, ...) failed: %d\n",
		        netdev->name, netdev->irq, err);
		return err;
	}

	l4shmc_chunk_ready(&priv->rx_chunk, 0);
	L4XV_L(f);
	l4shmc_trigger(&priv->tx_sig);
	L4XV_U(f);
	if (!priv->remote_attached)
		init_dev_other(netdev);
	update_carrier(netdev);

	return 0;
}

static int l4x_l4shmc_close(struct net_device *netdev)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	L4XV_V(f);

	free_irq(netdev->irq, netdev);
	l4shmc_chunk_consumed(&priv->rx_chunk);
	L4XV_L(f);
	l4shmc_trigger(&priv->tx_sig);
	L4XV_U(f);
	update_carrier(netdev);

	return 0;
}

static int l4x_l4shmc_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct l4x_l4shmc_priv *priv = netdev_priv(netdev);
	int max_mtu = min(l4shmc_chunk_capacity(&priv->rx_chunk),
			  l4shmc_chunk_capacity(&priv->tx_chunk)) - 100;

	if (new_mtu > max_mtu)
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

static int __init l4x_l4shmnet_init_dev(int num)
{
	struct l4x_l4shmc_priv *priv;
	struct net_device *dev = NULL;
	struct l4x_l4shmc_netdev *nd = NULL;
	const char *name = devs_to_add_name[num];
	int err, ret;
	L4XV_V(f);

	if (shmsize < PAGE_SIZE)
		shmsize = PAGE_SIZE;

	if (!(dev = alloc_etherdev(sizeof(struct l4x_l4shmc_priv))))
		return -ENOMEM;

	dev->netdev_ops = &l4shmnet_netdev_ops,
	dev->dev_addr[0] = 0x52;
	dev->dev_addr[1] = 0x54;
	dev->dev_addr[2] = 0x00;
	dev->dev_addr[3] = 0xb0;
	dev->dev_addr[4] = 0xcf;
	dev->dev_addr[5] = devs_to_add_macpart[num];

	priv = netdev_priv(dev);
	priv->num = num;
	priv->link_up = 0;
	priv->remote_attached = 0;

	pr_info("%s: Requesting, Shmsize %d Kbytes\n", name, shmsize >> 10);

	L4XV_L(f);
	err = -ENOMEM;
	if ((ret = l4shmc_create(name, shmsize)) < 0) {
		if (ret != -L4_EEXIST)
			goto err_out_free_dev_unlock;
	}

	// we block very long here, don't do that
	if (l4shmc_attach_to(name, WAIT_TIMEOUT, &priv->shmcarea))
		goto err_out_free_dev_unlock;

	ret = init_dev_self(dev);
	if (ret < 0) {
		/* XXX: convert error code */
		goto err_out_free_dev_unlock;
	}

	L4XV_U(f);

	if ((dev->irq = l4x_register_irq(l4shmc_signal_cap(&priv->rx_sig))) < 0) {
		pr_err("l4shmnet: Failed to get virq\n");
		goto err_out_free_dev;
	}

	dev->mtu = 1500;

	err = -ENODEV;
	if ((err = register_netdev(dev))) {
		pr_err("l4shmnet: Cannot register net device, aborting.\n");
		goto err_out_free_dev;
	}

	nd = kmalloc(sizeof(struct l4x_l4shmc_netdev), GFP_KERNEL);
	if (!nd) {
		pr_err("l4shmnet: Out of memory.\n");
		goto err_out_free_dev;
	}
	nd->dev = dev;
	list_add(&nd->list, &l4x_l4shmnet_netdevices);

	ret = init_dev_other(dev);
	if (ret < 0 && ret != -L4_EAGAIN) {
		/* XXX: convert error code */
		goto err_out_free_dev;
	}

	return 0;

err_out_free_dev_unlock:
	L4XV_U(f);
err_out_free_dev:
	pr_info("%s: Failed to establish communication\n", name);
	free_netdev(dev);

	return err;
}

static int __init l4x_l4shmnet_init(void)
{
	int i, ret = -ENODEV;

	for (i = 0; i < devs_to_add_pos; ++i)
		if (*devs_to_add_name[i]
		    && !l4x_l4shmnet_init_dev(i))
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
		pr_err("l4shmnet: Too many devices specified, max %d\n",
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
			c++;
			if (!strncmp(c, "macpart=", 8)) {
				devs_to_add_macpart[devs_to_add_pos]
				  = simple_strtoul(c + 8, NULL, 0);
			}
			else {
				char *end = strchr(c, ',');
				pr_err("l4shmnet: unknown argument: %*s",
					end ? end - c : strlen(c), c);
			}
		} while ((c = strchr(c, ',')));
	}
	strlcpy(devs_to_add_name[devs_to_add_pos], val, l);
	devs_to_add_pos++;
	return 0;
}

module_param_call(add, l4x_l4shmnet_setup, NULL, NULL, 0200);
MODULE_PARM_DESC(add, "Use l4shmnet.add=name,macpart=xx to add a device, name queried in namespace");

module_param(shmsize, int, 0);
MODULE_PARM_DESC(shmsize, "Size of the shared memory area");
