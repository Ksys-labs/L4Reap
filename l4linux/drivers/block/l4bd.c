/*
 * Block driver for generic_blk.
 *
 * With great help from http://lwn.net/Articles/driver-porting/, thanks.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include <asm/api/macros.h>

#include <asm/generic/l4lib.h>
#include <l4/sys/types.h>
#include <l4/sys/syscalls.h>
#include <l4/names/libnames.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/env/errno.h>

#include <l4/generic_blk/blk.h>
#include <l4/generic_blk/blk-client.h>

MODULE_AUTHOR("Adam Lackorzynski <adam@os.inf.tu-dresden.de");
MODULE_DESCRIPTION("Driver for the L4 generic block interface");
MODULE_LICENSE("GPL");

L4_EXTERNAL_FUNC(l4blk_get_driver_thread);
L4_EXTERNAL_FUNC(l4blk_do_request);
L4_EXTERNAL_FUNC(l4dm_revoke);
L4_EXTERNAL_FUNC(l4blk_init);
L4_EXTERNAL_FUNC(l4blk_open_driver);
L4_EXTERNAL_FUNC(l4blk_ctrl_get_num_disks);
L4_EXTERNAL_FUNC(l4blk_ctrl_get_disk_size);
L4_EXTERNAL_FUNC(l4blk_close_driver);

static char l4blk_name[32];
module_param_string(l4blk_name, l4blk_name, sizeof(l4blk_name), 0);
MODULE_PARM_DESC(l4blk_name, "L4 block driver name (mandatory to enable driver)");

static int l4blk_dev = MKDEV(3,0); /* hda */
module_param(l4blk_dev, int, 0);
MODULE_PARM_DESC(l4blk_dev, "device number MKDEV(major,minor) (default is 3.0 for hda)");

static l4blk_driver_t l4bd_driver;

static int l4blk_num_disks;

static int major_num = 0;        /* kernel chooses */
module_param(major_num, int, 0);

#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */
static struct request_queue *queue;

/*
 * The internal representation of our device.
 */
static struct l4bd_device {
	unsigned long size; /* Size in Kbytes */
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
} device;


/*
 * So, here is the story why we should use partial reads and writes:
 * The generic_blk interface offers us a 1024bytes/block interface, Linux
 * uses 512bytes/block. So it happens that we get a odd count of sectors to
 * read or write and we have to cope with that. Linux offers the possibility
 * to adjust the hardsect size of a block device so that we could set this
 * to 1024. The world would be nice and the sun would shine. Unfortunately
 * the partitioning code needs the real hardware sector size to properly
 * handle partitions. Now we could maybe change some generic block handling
 * code in Linux to handle our case but we do not really want that.
 * On the other hand we could just modify the partitioning code to always
 * work with 512bytes/block, set hardsect size to 1024 and be done with it.
 * This method works most of the time but hardsect size is also exported to
 * user land and e.g. used by fdisk (consequently as modifying partitioning
 * needs it). So for maximum satisfaction we cannot set hardsect size to
 * 1024. Consequently we need to code that partial stuff.
 * As a side note: the generic_blk interface is not powerful enough to
 * cope with all device. Devices with a blocksize != 512 and partitions on
 * them won't work as we do not know their real hardware sector size and
 * therefore cannot parse the partition data on the devices.
 */


static void l4bd_submit_request(l4blk_request_t *req,
                                unsigned long size,
                                char *buffer)
{
	l4blk_sg_ds_elem_t ds_buf;
	l4_size_t map_size;
	l4_addr_t map_addr;
	l4_threadid_t dthr;
	int ret;

	req->driver  = l4bd_driver;
	req->sg_list = &ds_buf;
	req->sg_num  = 1;
	req->sg_type = L4BLK_SG_DS;

	ds_buf.size = size;

	ret = l4rm_lookup(buffer, &map_addr, &map_size,
	                  &ds_buf.ds, &ds_buf.offs, &dthr);
	if (unlikely(ret != L4RM_REGION_DATASPACE)) {
		printk("l4bd: lookup request buffer dataspace failed: %s (%d)\n",
		       l4env_errstr(ret), ret);
		return; /* XXX: what to do now? How to propagate the error? */
	}
	if (unlikely(map_size < size)) {
		printk("map_size < nsect * sector_size, should not happen!\n");
		enter_kdebug("*sigh*");
	}

	ret = l4dm_share(&ds_buf.ds,
			 l4blk_get_driver_thread(l4bd_driver), L4DM_RW);
	if (unlikely(ret))
		printk("l4bd: Cannot share DS with driver (ignored): %s (%d)\n",
		       l4env_errstr(ret), ret);

	ret = l4blk_do_request(req);

	if (unlikely(ret < 0))
		printk("l4bd: request failed: %s (%d)\n", l4env_errstr(ret), ret);

	l4dm_revoke(&(*(l4blk_sg_ds_elem_t *)req->sg_list).ds,
	            l4blk_get_driver_thread(l4bd_driver), L4DM_RW);
}

static void l4bd_partial_block_write(l4blk_request_t *req,
                                     unsigned long sector,
                                     char *buffer)
{
	char *tmpbuf;

	req->request.block = sector >> 1;
	req->request.count = 1;

	tmpbuf = kmalloc(L4BLK_BLKSIZE, GFP_KERNEL);
	if (!tmpbuf)
		return;

	/* Get a whole l4blk buffer */
	req->request.cmd = L4BLK_REQUEST_READ;
	l4bd_submit_request(req, L4BLK_BLKSIZE, tmpbuf);

	/* Copy new, smaller content in the buffer */
	memcpy(tmpbuf + (sector % 2) * KERNEL_SECTOR_SIZE, buffer,
	       KERNEL_SECTOR_SIZE);

	/* Finally, write modified buffer back */
	req->request.block = sector >> 1;
	req->request.count = 1;
	req->request.cmd = L4BLK_REQUEST_WRITE;
	l4bd_submit_request(req, L4BLK_BLKSIZE, tmpbuf);

	kfree(tmpbuf);
}

static void l4bd_partial_block_read(l4blk_request_t *req,
                                    unsigned long sector,
                                    char *buffer)
{
	char *tmpbuf;

	req->request.cmd   = L4BLK_REQUEST_READ;
	req->request.block = sector >> 1;
	req->request.count = 1;

	tmpbuf = kmalloc(L4BLK_BLKSIZE, GFP_KERNEL);
	if (!tmpbuf)
		return;

	/* Get whole l4blk sector */
	l4bd_submit_request(req, L4BLK_BLKSIZE, tmpbuf);

	/* Only copy half of the content to the buffer */
	memcpy(buffer, tmpbuf + (sector % 2) * KERNEL_SECTOR_SIZE,
	       KERNEL_SECTOR_SIZE);

	kfree(tmpbuf);
}

/*
 * Handle an I/O request.
 */
static void l4bd_transfer(struct l4bd_device *dev, unsigned long sector_start,
                          unsigned long sector_count, char *buffer, int write)
{
	unsigned long byte_start = sector_start * KERNEL_SECTOR_SIZE;
	unsigned long byte_count = sector_count * KERNEL_SECTOR_SIZE;
	l4blk_request_t req;

	if (((byte_start + byte_count) >> 10) > dev->size) {
		printk(KERN_NOTICE "l4bd: access beyond end of device (%ld %ld)\n",
		       byte_start, byte_count);
		return;
	}

	req.request.device = l4blk_dev; /* hd device */
	req.request.flags  = 0;
	req.request.stream = L4BLK_INVALID_STREAM;

	if (unlikely(sector_start % 2)) {
		/* Odd sector start */
		if (write)
			l4bd_partial_block_write(&req, sector_start, buffer);
		else
			l4bd_partial_block_read(&req, sector_start, buffer);

		sector_start++;
		sector_count--;
		byte_count -= KERNEL_SECTOR_SIZE;
		buffer += KERNEL_SECTOR_SIZE;
	}
	/* Now, sector_start is even */

	/* Looking over the code it seems possible to reuse the req for
	 * multiple l4blk_do_request calls */

	if (likely(sector_count >> 1)) {
		req.request.cmd        = write ? L4BLK_REQUEST_WRITE : L4BLK_REQUEST_READ;
		req.request.block      = sector_start >> 1; /* L4BLK_BLKSIZE == 1024 */
		req.request.count      = sector_count >> 1; /* dito */

		l4bd_submit_request(&req, byte_count, buffer);
	}

	if (unlikely(sector_count % 2)) {
		if (write)
			l4bd_partial_block_write(&req,
			                         sector_start + sector_count - 1,
			                         buffer + (sector_count - 1) * KERNEL_SECTOR_SIZE);
		else
			l4bd_partial_block_read(&req,
			                        sector_start + sector_count - 1,
			                        buffer + (sector_count - 1) * KERNEL_SECTOR_SIZE);
	}
}

static void l4bd_request(struct request_queue *q)
{
	struct request *req;

	while ((req = blk_peek_request(q)) != NULL) {
		blk_start_request(req);
		if (!blk_fs_request(req)) {
			printk (KERN_NOTICE "Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}
		printk("This code needs to be fixed, look at libbdds.c for handling this... \n");
		l4bd_transfer(&device, blk_rq_pos(req),
		              blk_rq_cur_sectors(req),
		              req->buffer, rq_data_dir(req) == WRITE);
		__blk_end_request_all(req, 0);
	}
}


static int l4bd_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->cylinders = (device.size << (10 - 6))
                            * (L4BLK_BLKSIZE / KERNEL_SECTOR_SIZE);
	geo->heads     = 4;
	geo->sectors   = 32;
	return 0;
}


/*
 * The device operations structure.
 */
static struct block_device_operations l4bd_ops = {
	.owner = THIS_MODULE,
	.getgeo = l4bd_getgeo,
};

static int __init l4bd_init(void)
{
	int ret;

	if (!l4blk_name[0]) {
		printk("l4bd: No l4blk_name given, not starting.\n");
		return 0;
	}

	if (L4BLK_BLKSIZE != 1024) {
		printk("l4bd: L4BLK_BLKSIZE != 1024; needs adaption.\n");
		return -ENODEV;
	}
	if (KERNEL_SECTOR_SIZE != 512) {
		printk("l4bd: KERNEL_SECTOR_SIZE != 512; needs adaption.\n");
		return -ENODEV;
	}

	l4blk_init();

	ret = l4blk_open_driver(l4blk_name, &l4bd_driver, NULL);
	if (ret) {
		printk("l4bd: driver open failed: %s (%d)\n",
		       l4env_errstr(ret), ret);
		return -ENODEV;
	}

	printk("l4bd: Found L4 block driver \'%s\' as handle %d.\n",
	       l4blk_name, l4bd_driver);

	/* get number of disks */
	ret = -ENODEV;
	l4blk_num_disks = l4blk_ctrl_get_num_disks(l4bd_driver);
	if (l4blk_num_disks < 0) {
		printk("l4bd: get number of disks failed: %d!\n", l4blk_num_disks);
		goto out1;
	}

	if (!l4blk_num_disks) {
		printk("l4bd: no disks found, exiting...\n");
		goto out1;
	}

	printk("l4bd: Found %d disks (limiting to 1 for now).\n", l4blk_num_disks);

	/* Setup device. */
	ret = l4blk_ctrl_get_disk_size(l4bd_driver, l4blk_dev);
	if (ret < 0) {
		printk("l4bd: cannot get size for disk: %d(%s)\n",
		       ret, l4env_errstr(ret));
		goto out1;
	}

	device.size = ret; /* device.size is unsigned and in KBytes */

	printk("l4bd: Disk %d.%d size = %lu KB (%lu MB)\n",
	       MAJOR(l4blk_dev), MINOR(l4blk_dev), device.size, device.size >> 10);

	spin_lock_init(&device.lock);
	device.data = NULL;

	/* Get a request queue. */
	queue = blk_init_queue(l4bd_request, &device.lock);
	if (queue == NULL)
		goto out1;

	/* Register device */
	major_num = register_blkdev(major_num, "l4bd");
	if (major_num <= 0) {
		printk(KERN_WARNING "l4bd: unable to get major number\n");
		goto out2;
	}

	/* gendisk structure. */
	device.gd = alloc_disk(16);
	if (!device.gd)
		goto out3;
	device.gd->major        = major_num;
	device.gd->first_minor  = 0;
	device.gd->fops         = &l4bd_ops;
	device.gd->private_data = &device;
	strcpy(device.gd->disk_name, "l4bd0");
	set_capacity(device.gd, device.size * 2 /* 2 * kb = 512b-sectors */);
	device.gd->queue = queue;
	add_disk(device.gd);

	return 0;

out3:
	unregister_blkdev(major_num, "l4bd");
out2:
	blk_cleanup_queue(queue);
out1:
	/* close L4 block driver instance */
	l4blk_close_driver(l4bd_driver);

	return ret;
}

static void __exit l4bd_exit(void)
{
	del_gendisk(device.gd);
	put_disk(device.gd);
	unregister_blkdev(major_num, "l4bd");
	blk_cleanup_queue(queue);
	l4blk_close_driver(l4bd_driver);
}

module_init(l4bd_init);
module_exit(l4bd_exit);
