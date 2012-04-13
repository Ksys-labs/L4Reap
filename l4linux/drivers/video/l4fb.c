/*
 * Framebuffer driver
 *
 * based on vesafb.c
 *
 * Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/screen_info.h>
#include <linux/interrupt.h>
#include <linux/sysdev.h>
#include <asm/uaccess.h>

#include <asm/generic/l4lib.h>
#include <l4/sys/err.h>
#include <l4/sys/task.h>
#include <l4/sys/factory.h>
#include <l4/sys/icu.h>
#include <l4/re/c/util/video/goos_fb.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/namespace.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/util/cap.h>
#include <l4/re/c/event_buffer.h>
#include <l4/re/c/video/view.h>
#include <l4/re/c/video/goos.h>
#include <l4/log/log.h>

#include <asm/l4lxapi/misc.h>
#include <asm/generic/l4fb_ioctl.h>

#include <asm/generic/setup.h>
#include <asm/generic/l4fb.h>
#include <asm/generic/cap_alloc.h>
#include <asm/generic/util.h>
#include <asm/generic/vcpu.h>
#ifdef CONFIG_L4_FB_DRIVER_DBG
#include <asm/generic/stats.h>
#endif

L4_EXTERNAL_FUNC(l4re_video_goos_delete_view);
L4_EXTERNAL_FUNC(l4re_video_goos_create_view);
L4_EXTERNAL_FUNC(l4re_video_goos_delete_buffer);
L4_EXTERNAL_FUNC(l4re_video_goos_get_view);
L4_EXTERNAL_FUNC(l4re_video_view_get_info);
L4_EXTERNAL_FUNC(l4re_video_view_set_info);
L4_EXTERNAL_FUNC(l4re_video_goos_get_static_buffer);
L4_EXTERNAL_FUNC(l4re_event_buffer_consumer_foreach_available_event);
L4_EXTERNAL_FUNC(l4re_event_get);
L4_EXTERNAL_FUNC(l4re_event_buffer_attach);
L4_EXTERNAL_FUNC(l4re_video_goos_info);
L4_EXTERNAL_FUNC(l4re_video_goos_create_buffer);
L4_EXTERNAL_FUNC(l4re_video_goos_refresh);
L4_EXTERNAL_FUNC(l4re_video_view_set_viewport);
L4_EXTERNAL_FUNC(l4re_video_view_stack);

enum {
	MAX_UNMAP_BITS = 32, // enough for the framebuffer
};

struct l4fb_unmap_info {
	unsigned int map[MAX_UNMAP_BITS - 1 - L4_PAGESHIFT];
	unsigned int top;
	unsigned int weight;
	l4_fpage_t *flexpages;
};

struct l4fb_view {
	struct list_head next_view;
	l4re_video_view_t view;
	int index;
};

enum {
	MAX_INPUT_DEV_NAME = 20
};

struct l4fb_screen {
	struct list_head next_screen;

	unsigned long flags;

	l4re_video_goos_t goos;
	l4_cap_idx_t fb_cap;

	l4re_video_goos_info_t ginfo;

	//l4re_video_view_info_t fbi;
	l4_addr_t fb_addr, fb_line_length;
	struct list_head views;

	unsigned int refresh_sleep;
	int refresh_enabled;
	struct timer_list refresh_timer;
	struct l4fb_unmap_info unmap_info;
	u32 pseudo_palette[17];

	/* Input event part */
	l4_cap_idx_t ev_ds;
	l4_cap_idx_t ev_irq;
	long irqnum;
	l4re_event_buffer_consumer_t ev_buf;

	int touchscreen;
	int abs2rel;
	int singledev;

	int last_rel_x, last_rel_y;

	/* Mouse and keyboard are split so that mouse button events are not
	 * treated as keyboard events in the Linux console. */
	struct input_dev *keyb;
	struct input_dev *mouse;

	struct platform_device platform_device;

	char input_keyb_name[MAX_INPUT_DEV_NAME];
	char input_mouse_name[MAX_INPUT_DEV_NAME];
};

static LIST_HEAD(l4fb_screens);
static int disable, verbose, touchscreen, singledev, abs2rel, verbose_wm;
static char *fbs[10] = { "fb", };
static int num_fbs = 1;
static const unsigned int unmaps_per_refresh = 1;
#if 0
static int redraw_pending;
#endif

static inline struct l4fb_screen *l4fb_screen(struct fb_info *info)
{
	return container_of(container_of(info->device,
	                                 struct platform_device, dev),
	                    struct l4fb_screen, platform_device);
}


#ifdef CONFIG_L4_FB_DRIVER_DBG
static struct dentry *debugfs_dir, *debugfs_unmaps, *debugfs_updates;
static unsigned int stats_unmaps, stats_updates;
#endif

/* -- module paramter variables ---------------------------------------- */

static int refreshsleep = -1;

/* -- framebuffer variables/structures --------------------------------- */

static struct fb_var_screeninfo const l4fb_defined = {
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.right_margin	= 32,
	.upper_margin	= 16,
	.lower_margin	= 4,
	.vsync_len	= 4,
	.vmode		= FB_VMODE_NONINTERLACED,
};

static const struct fb_fix_screeninfo l4fb_fix = {
	.id	= "l4fb",
	.type	= FB_TYPE_PACKED_PIXELS,
	.accel	= FB_ACCEL_NONE,
};

/* -- implementations -------------------------------------------------- */


static void l4fb_init_screen(struct l4fb_screen *screen)
{
	screen->goos = L4_INVALID_CAP;

	screen->refresh_sleep = HZ / 10;
	screen->refresh_enabled = 1;

	/* Input event part */
	screen->ev_ds = L4_INVALID_CAP;
	screen->ev_irq = L4_INVALID_CAP;
	screen->irqnum = 0;

	screen->touchscreen = 0;
	screen->abs2rel = 0;
	screen->singledev = 0;

	screen->last_rel_x = 0;
	screen->last_rel_y = 0;

	/* Mouse and keyboard are split so that mouse button events are not
	 * treated as keyboard events in the Linux console. */
	screen->keyb = NULL;
	screen->mouse = NULL;
	INIT_LIST_HEAD(&screen->views);
}

static void vesa_setpalette(int regno, unsigned red, unsigned green,
			    unsigned blue)
{
#if 0
	struct { u_char blue, green, red, pad; } entry;
	int shift = 16 - depth;

	if (pmi_setpal) {
		entry.red   = red   >> shift;
		entry.green = green >> shift;
		entry.blue  = blue  >> shift;
		entry.pad   = 0;
	        __asm__ __volatile__(
                "call *(%%esi)"
                : /* no return value */
                : "a" (0x4f09),         /* EAX */
                  "b" (0),              /* EBX */
                  "c" (1),              /* ECX */
                  "d" (regno),          /* EDX */
                  "D" (&entry),         /* EDI */
                  "S" (&pmi_pal));      /* ESI */
	} else {
		/* without protected mode interface, try VGA registers... */
		outb_p(regno,       dac_reg);
		outb_p(red   >> shift, dac_val);
		outb_p(green >> shift, dac_val);
		outb_p(blue  >> shift, dac_val);
	}
#endif
}

static int l4fb_setcolreg(unsigned regno, unsigned red, unsigned green,
                          unsigned blue, unsigned transp,
                          struct fb_info *info)
{
	/*
	 *  Set a single color register. The values supplied are
	 *  already rounded down to the hardware's capabilities
	 *  (according to the entries in the `var' structure). Return
	 *  != 0 for invalid regno.
	 */

	if (regno >= info->cmap.len)
		return 1;

	if (info->var.bits_per_pixel == 8)
		vesa_setpalette(regno,red,green,blue);
	else if (regno < 16) {
		switch (info->var.bits_per_pixel) {
		case 16:
			((u32*) (info->pseudo_palette))[regno] =
				((red   >> (16 -   info->var.red.length)) <<   info->var.red.offset) |
				((green >> (16 - info->var.green.length)) << info->var.green.offset) |
				((blue  >> (16 -  info->var.blue.length)) <<  info->var.blue.offset);
			break;
		case 24:
		case 32:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((u32 *)(info->pseudo_palette))[regno] =
				(red   << info->var.red.offset)   |
				(green << info->var.green.offset) |
				(blue  << info->var.blue.offset);
			break;
		}
	}

	return 0;
}

static int l4fb_pan_display(struct fb_var_screeninfo *var,
                            struct fb_info *info)
{
	if (var->vmode & FB_VMODE_YWRAP) {
		if (var->yoffset < 0
		    || var->yoffset >= info->var.yres_virtual
		    || var->xoffset)
			return -EINVAL;
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
		    var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;
	return 0;
}

static void (*l4fb_update_rect)(struct l4fb_screen *, int, int, int, int);

static l4fb_input_event_hook_function_type l4fb_input_event_hook_function;

void l4fb_input_event_hook_register(l4fb_input_event_hook_function_type f)
{
	l4fb_input_event_hook_function = f;
}
EXPORT_SYMBOL(l4fb_input_event_hook_register);

static inline struct device *scr2dev(struct l4fb_screen *screen)
{ return &screen->platform_device.dev; }

static void l4fb_delete_all_views(struct l4fb_screen *screen)
{
	struct l4fb_view *view, *tmp;
	L4XV_V(f);
	list_for_each_entry_safe(view, tmp, &screen->views, next_view) {
		list_del(&view->next_view);
		if (screen->flags & F_l4re_video_goos_dynamic_views) {
			L4XV_L(f);
			l4re_video_goos_delete_view(screen->goos, &view->view);
			L4XV_U(f);
		}
		kfree(view);
	}
}

static void l4fb_l4re_update_rect(struct l4fb_screen *screen,
                                  int x, int y, int w, int h)
{
	L4XV_V(f);
	if (!screen)
		return;

	L4XV_L(f);
	l4re_video_goos_refresh(screen->goos, x, y, w, h);
	L4XV_U(f);
}


static void l4fb_l4re_update_memarea(struct l4fb_screen *screen,
                                     l4_addr_t base, l4_addr_t size)
{
	int y, h;

	if (size < 0 || base < screen->fb_addr)
		LOG_printf("l4fb: update: WRONG VALS: sz=%ld base=%lx start=%lx\n",
		           size, base, screen->fb_addr);

	y = ((base - screen->fb_addr) / screen->fb_line_length);
	h = (size / screen->fb_line_length) + 1;

#ifdef CONFIG_L4_FB_DRIVER_DBG
	++stats_updates;
#endif

	/* FIXME: assume we have just a single view for now */
	l4fb_l4re_update_rect(screen, 0, y, screen->ginfo.width, h);
}

#if 0
// actually we would need to make a copy of the screen to make is possible
// to restore the before-blank contents on unblank events
static int l4fb_blank(int blank, struct fb_info *info)
{
	// pretent the screen is off
	if (blank != FB_BLANK_UNBLANK)
		memset((void *)info->fix.smem_start, 0, info->fix.smem_len);
	return 0;
}
#endif

static void l4fb_copyarea(struct fb_info *info, const struct fb_copyarea *region)
{
	cfb_copyarea(info, region);
	if (l4fb_update_rect)
		l4fb_update_rect(l4fb_screen(info), region->dx, region->dy, region->width, region->height);
}

static void l4fb_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
	cfb_fillrect(info, rect);
	if (l4fb_update_rect)
		l4fb_update_rect(l4fb_screen(info), rect->dx, rect->dy, rect->width, rect->height);
}

static void l4fb_imageblit(struct fb_info *info, const struct fb_image *image)
{
	cfb_imageblit(info, image);
	if (l4fb_update_rect)
		l4fb_update_rect(l4fb_screen(info), image->dx, image->dy, image->width, image->height);
}

static int l4fb_open(struct fb_info *info, int user)
{
	return 0;
}

static int l4fb_release(struct fb_info *info, int user)
{
	struct l4fb_screen *screen = l4fb_screen(info);
	/* hm, we just throw away all the dynamic view inforamtion here
	 * May be, it is better to only clean up when all clients did a
	 * release?.
	 */

	/* Nothing to cleanup for static goos sessions. */
	if (!(screen->flags & F_l4re_video_goos_dynamic_views))
		return 0;

	if (verbose_wm)
		dev_info(scr2dev(screen), "cleanup goos stuff\n");

	l4fb_delete_all_views(screen);

	return 0;
}

static int l4fb_mmap(struct fb_info *info,
                     struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long pfn;

	printk("mmap FB: offs=%lx size=%lx fb_size=%x\n", offset, size, info->fix.smem_len);
	if (offset + size > info->fix.smem_len)
		return -EINVAL;

	pfn = ((unsigned long)info->fix.smem_start + offset) >> PAGE_SHIFT;
	while (size > 0) {
		if (remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED)) {
			return -EAGAIN;
		}
		start += PAGE_SIZE;
		pfn++;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}
	l4_touch_rw((char *)info->fix.smem_start + offset,
	            vma->vm_end - vma->vm_start);

	return 0;
}

static struct l4fb_view *l4fb_alloc_view(void)
{
	return kzalloc(sizeof(struct l4fb_view), GFP_KERNEL);
}

static struct l4fb_view *l4fb_find_view(struct l4fb_screen *screen, int view)
{
	struct l4fb_view *v;

	list_for_each_entry(v, &screen->views, next_view)
		if (v->index == view)
			return v;
	return NULL;
}


static int l4fb_create_view(struct l4fb_screen *screen, int view)
{
	int ret;
	struct l4fb_view *v;
	l4re_video_view_info_t vi;
	L4XV_V(f);

	if (verbose_wm)
		dev_info(scr2dev(screen), "create goos view[%d]\n", view);

	if (!(screen->flags & F_l4re_video_goos_dynamic_views))
		return -EINVAL;

	if (l4fb_find_view(screen, view))
		return -EEXIST;

	if (!((v = l4fb_alloc_view())))
		return -ENOMEM;

	v->index = view;
	L4XV_L(f);
	ret = l4re_video_goos_create_view(screen->goos, &v->view);
	L4XV_U(f);

	if (ret < 0) {
		kfree(v);
		return ret;
	}

	/* we use the preferred color mode of the screen */
	vi.pixel_info = screen->ginfo.pixel_info;
	/* we use always a single buffer as backing store */
	vi.buffer_index = 0;
	/* we prepare for classical overlay wm, so all views use
	 * a single scan-line length */
	vi.bytes_per_line = screen->fb_line_length;

	vi.flags =   F_l4re_video_view_set_buffer
	           | F_l4re_video_view_set_bytes_per_line
	           | F_l4re_video_view_set_pixel;


	L4XV_L(f);
	ret = l4re_video_view_set_info(&v->view, &vi);
	L4XV_U(f);

	if (ret)
		dev_err(scr2dev(screen), "error setting the view properties (%d)\n", ret);

	list_add(&v->next_view, &screen->views);
	return 0;
}


static int l4fb_delete_view(struct l4fb_screen *screen, int view)
{
	struct l4fb_view *v = l4fb_find_view(screen, view);
	int ret;
	L4XV_V(f);

	if (verbose_wm)
		dev_info(scr2dev(screen), "delete goos view[%d]\n", view);

	if (!v)
		return -ENOENT;

	list_del(&v->next_view);
	L4XV_L(f);
	ret = l4re_video_goos_delete_view(screen->goos, &v->view);
	L4XV_U(f);
	return ret;
}

static int l4fb_background_view(struct l4fb_screen *screen, int view)
{
	struct l4fb_view *v = l4fb_find_view(screen, view);
	int ret;
	l4re_video_view_info_t vi;
	L4XV_V(f);

	if (verbose_wm)
		dev_info(scr2dev(screen), "background goos view[%d]\n", view);

	if (!v)
		return -ENOENT;

	vi.flags = F_l4re_video_view_set_background;
	L4XV_L(f);
	ret = l4re_video_view_set_info(&v->view, &vi);
	L4XV_U(f);
	return ret;
}

static int l4fb_place_view(struct l4fb_screen *screen, int view, int x, int y, int w, int h)
{
	struct l4fb_view *v = l4fb_find_view(screen, view);
	int ret;
	unsigned long buffer_offset;
	L4XV_V(f);

	if (0)
		dev_info(scr2dev(screen), "place goos view[%d] -> %d,%d - %d,%d\n",
		         view, x, y, x + w, y + h);

	if (!v)
		return -ENOENT;

	if (x < 0) {
		w += x;
		x = 0;
	}

	if (w < 0)
		w = 0;

	if (y < 0) {
		h += y;
		y = 0;
	}

	if (h < 0)
		h = 0;

	buffer_offset = y * screen->fb_line_length
	                + x * screen->ginfo.pixel_info.bytes_per_pixel;
	if (0)
		dev_info(scr2dev(screen), "place goos view[%d] -> %d,%d - %d,%d %lu\n", view, x, y,
		         x + w, y + h, buffer_offset);
	L4XV_L(f);
	ret = l4re_video_view_set_viewport(&v->view, x, y, w, h, buffer_offset);
	L4XV_U(f);
	return ret;
}

static int l4fb_stack_view(struct l4fb_screen *screen, int view, int pivot, int behind)
{
	struct l4fb_view *v, *p = NULL;
	int ret;
	L4XV_V(f);

	if (0)
		dev_info(scr2dev(screen), "stack goos view[%d] %s(%d) view: %d\n", view,
		         behind ? "behind" : "before", behind, pivot);

	v = l4fb_find_view(screen, view);
	if (!v)
		return -ENOENT;

	if (pivot >= 0)
		p = l4fb_find_view(screen, pivot);

	L4XV_L(f);
	ret = l4re_video_view_stack(&v->view, p ? &p->view : NULL, behind);
	L4XV_U(f);
	return ret;
}

static int l4fb_view_set_flags(struct l4fb_screen *screen, int view, unsigned long flags)
{
	struct l4fb_view *v;
	int ret;
	l4re_video_view_info_t vi;
	L4XV_V(f);

	v = l4fb_find_view(screen, view);
	if (!v)
		return -ENOENT;

	vi.flags = F_l4re_video_view_set_flags
	           | (flags & F_l4re_video_view_flags_mask);

	L4XV_L(f);
	ret = l4re_video_view_set_info(&v->view, &vi);
	L4XV_U(f);
	return ret;
}


static int l4fb_ioctl(struct fb_info *info, unsigned int cmd,
                          unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct l4fb_screen *scr = l4fb_screen(info);
	int ret = 0;

	switch (cmd) {
	case L4FB_IOCTL_CREATE_VIEW:
		ret = l4fb_create_view(scr, (int)arg);
		break;
	case L4FB_IOCTL_DESTROY_VIEW:
		ret = l4fb_delete_view(scr, (int)arg);
		break;
	case L4FB_IOCTL_VIEW_SET_FLAGS:
		{
			struct l4fb_view_flags v;
			if (copy_from_user(&v, argp, sizeof(v)))
				return -EFAULT;
			ret = l4fb_view_set_flags(scr, v.view, v.flags);
			break;
		}
		break;
	case L4FB_IOCTL_BACK_VIEW:
		ret = l4fb_background_view(scr, (int)arg);
		break;
	case L4FB_IOCTL_PLACE_VIEW:
		{
			struct l4fb_set_viewport v;
			if (copy_from_user(&v, argp, sizeof(v)))
				return -EFAULT;
			ret = l4fb_place_view(scr, v.view, v.r.x, v.r.y, v.r.w, v.r.h);
			break;
		}
	case L4FB_IOCTL_STACK_VIEW:
		{
			struct l4fb_stack_view v;
			if (copy_from_user(&v, argp, sizeof(v)))
				return -EFAULT;
			ret = l4fb_stack_view(scr, v.view, v.pivot, v.behind);
			break;
		}
	case L4FB_IOCTL_REFRESH:
		{
			struct l4fb_region v;
			if (copy_from_user(&v, argp, sizeof(v)))
				return -EFAULT;
			l4fb_l4re_update_rect(scr, v.x, v.y, v.w, v.h);
			break;
		}
	default:
		dev_info(scr2dev(scr), "unknown ioctl: %x\n", cmd);
		ret = -EINVAL;
		break;
	}
	if (ret)
		dev_info(info->device, "ioctl ret=%d\n", ret);
	return ret;
}


#if 0
/* some try to figure aout VT switching from and to X-Server */
static int l4fb_check_var(struct fb_var_screeninfo *var,
                          struct fb_info *info)
{
	*var = info->var;
	return 0;
}

static int l4fb_set_par(struct fb_info *info)
{
	dev_info(info->device, "set par ... %lx %lx %lx\n",
	         info->flags, info->var.vmode, info->var.activate);
	//init your hardware here
	return 0;
}
#endif

static struct fb_ops l4fb_ops = {
	.owner		= THIS_MODULE,
	.fb_open        = l4fb_open,
	.fb_release     = l4fb_release,
	.fb_setcolreg	= l4fb_setcolreg,
	.fb_pan_display	= l4fb_pan_display,
	.fb_fillrect	= l4fb_fillrect,
	.fb_copyarea	= l4fb_copyarea,
	.fb_imageblit	= l4fb_imageblit,
	.fb_mmap	= l4fb_mmap,
	.fb_ioctl	= l4fb_ioctl,
	//.fb_check_var	= l4fb_check_var,
	//.fb_set_par	= l4fb_set_par,
	//.fb_blank	= l4fb_blank,
};

/* ============================================ */


L4_CV static void input_event_put(l4re_event_t *event, void *data)
{
	struct l4fb_screen *screen = (struct l4fb_screen *)data;
	struct input_event *e = (struct input_event *)event;
	enum {
		EV_CON = 0x10, EV_CON_REDRAW = 1,
	};

	/* Prevent input events before system is up, see comment in
	 * DOpE input function for more. */
	if (system_state != SYSTEM_RUNNING) {
#if 0
		/* Serve pending redraw requests later */
		if (e->type == EV_CON && e->code == EV_CON_REDRAW)
			redraw_pending = 1;
#endif
		return;
	}

	if (l4fb_input_event_hook_function)
		if (l4fb_input_event_hook_function(e->type, e->code))
			return;
#if 0
	/* console sent redraw event -- update whole screen */
	if (e->type == EV_CON && e->code == EV_CON_REDRAW) {
		l4fb_l4re_update_rect(0, 0, fbi.width, fbi.height);
		return;
	}
#endif

	if (screen->abs2rel && e->type == EV_ABS) {
		unsigned tmp;
		// x and y are enough?
		if (e->code == ABS_X) {
			e->type = EV_REL;
			e->code = REL_X;
			tmp = e->value;
			e->value = e->value - screen->last_rel_x;
			screen->last_rel_x = tmp;
		} else if (e->code == ABS_Y) {
			e->type = EV_REL;
			e->code = REL_Y;
			tmp = e->value;
			e->value = e->value - screen->last_rel_y;
			screen->last_rel_y = tmp;
		}
	}

	if (screen->touchscreen && e->type == EV_KEY && e->code == BTN_LEFT)
		e->code = BTN_TOUCH;

	/* The l4input library is based on Linux-2.6, so we're lucky here */
	if (e->type == EV_KEY && e->code < BTN_MISC) {
		input_event(screen->keyb, e->type, e->code, e->value);
		input_sync(screen->keyb);
	} else {
		input_event(screen->mouse, e->type, e->code, e->value);
		input_sync(screen->mouse);
	}
}


static irqreturn_t event_interrupt(int irq, void *data)
{
	struct l4fb_screen *screen = (struct l4fb_screen *)data;
	l4re_event_buffer_consumer_foreach_available_event(&screen->ev_buf, data, input_event_put);
	return IRQ_HANDLED;
}

static int l4fb_input_setup(struct fb_info *info, struct l4fb_screen *screen)
{
	unsigned int i;
	int err;

	if ((screen->irqnum = l4x_register_irq(screen->ev_irq)) < 0)
		return -ENOMEM;

	if ((err = request_irq(screen->irqnum, event_interrupt,
	                       IRQF_SAMPLE_RANDOM, "L4fbev", screen))) {
		dev_err(scr2dev(screen), "%s: request_irq failed: %d\n", __func__, err);
		return err;
	}

	screen->keyb   = input_allocate_device();
	if (screen->singledev)
		screen->mouse = screen->keyb;
	else
		screen->mouse = input_allocate_device();

	if (!screen->keyb || !screen->mouse)
		return -ENOMEM;

	/* Keyboard */
	screen->keyb->phys = "L4Re::Event";
	snprintf(screen->input_keyb_name, sizeof(screen->input_keyb_name), screen->singledev ? "L4input '%d'" : "L4keyb '%d'", info->node);
	screen->keyb->uniq = screen->input_keyb_name;
	screen->keyb->name = screen->input_keyb_name;
	screen->keyb->id.bustype = 0;
	screen->keyb->id.vendor  = 0x50fb;
	screen->keyb->id.product = info->node;
	screen->keyb->id.version = 0;

	/* We generate key events */
	set_bit(EV_KEY, screen->keyb->evbit);
	set_bit(EV_REP, screen->keyb->evbit);
	/* We can generate every key, do not use KEY_MAX as apps compiled
	 * against older linux/input.h might have lower values and segfault.
	 * Fun. */
	for (i = 0; i < 0x1ff; i++)
		set_bit(i, screen->keyb->keybit);

	if (!screen->singledev) {
		i = input_register_device(screen->keyb);
		if (i)
			return i;
	}

	/* Mouse */
	if (!screen->singledev) {
		screen->mouse->phys = "L4Re::Event";
		snprintf(screen->input_mouse_name, sizeof(screen->input_mouse_name),
		         "L4mouse '%d'", info->node);
		screen->mouse->name = screen->input_mouse_name;
		screen->mouse->uniq = screen->input_mouse_name;
		screen->mouse->id.bustype = 0;
		screen->mouse->id.vendor  = 0x50fb;
		screen->mouse->id.product = info->node;
		screen->mouse->id.version = 0;
	}

	/* We generate key and relative mouse events */
	set_bit(EV_KEY, screen->mouse->evbit);
	if (!screen->touchscreen)
		set_bit(EV_REL, screen->mouse->evbit);
	set_bit(EV_ABS, screen->mouse->evbit);
	set_bit(EV_SYN, screen->mouse->evbit);

	/* Buttons */
	if (screen->touchscreen)
		set_bit(BTN_TOUCH,  screen->mouse->keybit);
	else {
		set_bit(BTN_LEFT,   screen->mouse->keybit);
		set_bit(BTN_RIGHT,  screen->mouse->keybit);
		set_bit(BTN_MIDDLE, screen->mouse->keybit);
		set_bit(BTN_0,      screen->mouse->keybit);
		set_bit(BTN_1,      screen->mouse->keybit);
		set_bit(BTN_2,      screen->mouse->keybit);
		set_bit(BTN_3,      screen->mouse->keybit);
		set_bit(BTN_4,      screen->mouse->keybit);
		set_bit(BTN_TOOL_PEN,      screen->mouse->keybit);
	}

	/* Movements */
#if 0
	bitmap_fill(screen->mouse->relbit, REL_MAX);
	bitmap_fill(screen->mouse->absbit, ABS_MAX);
#else
	if (!screen->touchscreen) {
		set_bit(REL_X,      screen->mouse->relbit);
		set_bit(REL_Y,      screen->mouse->relbit);
	}
	set_bit(ABS_X,      screen->mouse->absbit);
	set_bit(ABS_Y,      screen->mouse->absbit);

	set_bit(ABS_PRESSURE,   screen->mouse->absbit);
	set_bit(ABS_TOOL_WIDTH, screen->mouse->absbit);
#endif
	input_set_abs_params(screen->mouse, ABS_PRESSURE, 0, 1, 0, 0);
	if (0)
		clear_bit(ABS_PRESSURE,   screen->mouse->absbit);

	/* Coordinates are 1:1 pixel in frame buffer */
	input_set_abs_params(screen->mouse, ABS_X, 0, screen->ginfo.width, 0, 0);
	input_set_abs_params(screen->mouse, ABS_Y, 0, screen->ginfo.height, 0, 0);

	i = input_register_device(screen->mouse);
	if (i)
		return i;

	return 0;
}

static void l4fb_update_dirty_unmap(struct l4fb_screen *screen)
{
	unsigned int i;
	l4_msg_regs_t *v = l4_utcb_mr_u(l4_utcb());

	i = 0;
	while (i < screen->unmap_info.weight) {
		l4_msgtag_t tag;
		l4_addr_t bulkstart, bulksize;
		unsigned int j, num_flexpages;

		num_flexpages = screen->unmap_info.weight - i >= L4_UTCB_GENERIC_DATA_SIZE - 2
		                ? L4_UTCB_GENERIC_DATA_SIZE - 2
		                : screen->unmap_info.weight - i;

		tag = l4_task_unmap_batch(L4RE_THIS_TASK_CAP, screen->unmap_info.flexpages + i,
		                          num_flexpages, L4_FP_ALL_SPACES);

		if (l4_error(tag))
			LOG_printf("l4fb: error with l4_task_unmap_batch\n");

		if (0)
			LOG_printf("l4fb: unmapped %d-%d/%d pages\n", i, i + num_flexpages - 1, screen->unmap_info.weight);

#ifdef CONFIG_L4_FB_DRIVER_DBG
		++stats_unmaps;
#endif

		/* redraw dirty pages bulkwise */
		bulksize = 0;
		bulkstart = L4_INVALID_ADDR;
		for (j = 0; j < num_flexpages; j++) {
			l4_fpage_t ret_fp;
			ret_fp.raw = v->mr[2 + j];
			if ((l4_fpage_rights(ret_fp) & L4_FPAGE_W)) {
				if (0)
					LOG_printf("   %d: addr=%lx size=%x was dirty\n",
					           j, l4_fpage_page(ret_fp) << L4_PAGESHIFT,
					           1 << l4_fpage_size(ret_fp));
				if (!bulksize)
					bulkstart = l4_fpage_page(ret_fp) << L4_PAGESHIFT;
				bulksize += 1 << l4_fpage_size(ret_fp);

				continue;
			}
			// we need to flush
			if (bulkstart != L4_INVALID_ADDR)
				l4fb_l4re_update_memarea(screen, bulkstart, bulksize);
			bulksize = 0;
			bulkstart = L4_INVALID_ADDR;
		}
		if (bulkstart != L4_INVALID_ADDR)
			l4fb_l4re_update_memarea(screen, bulkstart, bulksize);

		i += num_flexpages;
	}
}

/* init flexpage array according to map */
static void l4fb_update_dirty_init_flexpages(struct l4fb_screen *screen,
                                             l4_addr_t addr)
{
	unsigned int log2size, i, j;
	unsigned int size;

	size = screen->unmap_info.weight * sizeof(l4_fpage_t);
	if (verbose)
		LOG_printf("l4fb: going to kmalloc(%d) to store flexpages\n", size);
	screen->unmap_info.flexpages = kmalloc(size, GFP_KERNEL);
	log2size = screen->unmap_info.top;
	for (i = 0; i < screen->unmap_info.weight;) {
		for (j = 0; j < screen->unmap_info.map[log2size-L4_PAGESHIFT]; ++j) {
			screen->unmap_info.flexpages[i] = l4_fpage(addr, log2size, 0);
			if (verbose)
				LOG_printf("%d %lx - %lx \n", i, addr, addr + (1 << log2size));
			addr += 1 << log2size;
			++i;
		}
		--log2size;
	}
}

/* try to reduce the number of flexpages needed (weight) to num_flexpages */
static void l4fb_update_dirty_init_optimize(struct l4fb_screen *screen,
                                            unsigned int num_flexpages)
{
	struct l4fb_unmap_info *info = &screen->unmap_info;
	while ((num_flexpages > info->weight
	        || info->top > L4_SUPERPAGESHIFT)
	       && info->top >= L4_PAGESHIFT) {
		info->map[info->top - L4_PAGESHIFT] -= 1;
		info->map[info->top - L4_PAGESHIFT - 1] += 2;
		info->weight += 1;
		if (!info->map[info->top - L4_PAGESHIFT])
			info->top -= 1;
	}
	if (verbose)
		LOG_printf("l4fb: optimized on using %d flexpages, %d where requested \n",
		           info->weight, num_flexpages);
}

static void l4fb_update_dirty_init(struct l4fb_screen *screen,
                                   l4_addr_t addr, l4_addr_t size)
{
	struct l4fb_unmap_info *info = &screen->unmap_info;
	//unsigned int num_flexpages = (L4_UTCB_GENERIC_DATA_SIZE - 2) * unmaps_per_refresh;
	unsigned int num_flexpages = (3 - 2) * unmaps_per_refresh;
	unsigned int log2size = MAX_UNMAP_BITS - 1;

	memset(info, 0, sizeof(struct l4fb_unmap_info));
	size = l4_round_page(size);

	/* init map with bitlevel number */
	while (log2size >= L4_PAGESHIFT) {
		if (size & (1 << log2size)) {
			info->map[log2size-L4_PAGESHIFT] = 1;
			if (!info->top)
				info->top = log2size;
			info->weight += 1;
		}
		--log2size;
	}
	l4fb_update_dirty_init_optimize(screen, num_flexpages);
	l4fb_update_dirty_init_flexpages(screen, addr);
}


static void l4fb_refresh_func(unsigned long data)
{
	struct l4fb_screen *screen = (struct l4fb_screen *)data;
	if (screen->refresh_enabled && l4fb_update_rect) {
		if (1)
			l4fb_update_rect(screen, 0, 0, screen->ginfo.width, screen->ginfo.height);
		else
			l4fb_update_dirty_unmap(screen);
	}

	mod_timer(&screen->refresh_timer, jiffies + screen->refresh_sleep);
}

/* ============================================ */
static int l4fb_fb_init(struct fb_info *fb, struct l4fb_screen *screen)
{
	int ret, input_avail = 0;
	L4XV_V(f);

	L4XV_L(f);

	if (verbose)
		LOG_printf("Starting L4FB\n");

	ret = -ENOMEM;

	if (l4_is_invalid_cap(screen->ev_ds = l4x_cap_alloc()))
		goto out_unlock;

	if (l4_is_invalid_cap(screen->ev_irq = l4x_cap_alloc())) {
		l4x_cap_free(screen->ev_ds);
		goto out_unlock;
	}

	if (l4re_event_get(screen->goos, screen->ev_ds)) {
		LOG_printf("l4fb: INFO: No input available\n");

		l4x_cap_free(screen->ev_ds);
		l4x_cap_free(screen->ev_irq);
	} else {
		input_avail = 1;
		ret = -ENOENT;
		if (l4re_event_buffer_attach(&screen->ev_buf, screen->ev_ds, l4re_env()->rm))
			goto out_unlock;

		if (l4_error(l4_factory_create_irq(l4re_env()->factory, screen->ev_irq))) {
			l4x_cap_free(screen->ev_ds);
			l4x_cap_free(screen->ev_irq);
			goto out_unlock;
		}

		if (l4_error(l4_icu_bind(screen->goos, 0, screen->ev_irq))) {
			l4re_util_cap_release(screen->ev_irq);
			l4x_cap_free(screen->ev_ds);
			l4x_cap_free(screen->ev_irq);
			goto out_unlock;
		}
	}

	L4XV_U(f);

	if (input_avail)
		return l4fb_input_setup(fb, screen);
	return 0;

out_unlock:
	L4XV_U(f);
	return ret;
}

/* ============================================ */

static void l4fb_shutdown(struct l4fb_screen *screen)
{
	L4XV_V(f);
	del_timer_sync(&screen->refresh_timer);

	/* Also do not update anything anymore */
	l4fb_update_rect = NULL;

	free_irq(screen->irqnum, NULL);
	l4x_unregister_irq(screen->irqnum);

	L4XV_L(f);
	l4re_rm_detach((void *)screen->fb_addr);

	if (l4_is_valid_cap(screen->ev_irq)) {
		l4re_util_cap_release(screen->ev_irq);
		l4x_cap_free(screen->ev_irq);
	}
	if (l4_is_valid_cap(screen->ev_ds)) {
		l4re_util_cap_release(screen->ev_ds);
		l4x_cap_free(screen->ev_ds);
	}

	L4XV_U(f);

	if (screen->unmap_info.flexpages)
		kfree(screen->unmap_info.flexpages);

	l4fb_delete_all_views(screen);

	if (screen->flags & F_l4re_video_goos_dynamic_buffers) {
		L4XV_L(f);
		l4re_video_goos_delete_buffer(screen->goos, 0);
		L4XV_U(f);
	}

	L4XV_L(f);
	if (l4_is_valid_cap(screen->goos)) {
		l4re_util_cap_release(screen->goos);
		l4x_cap_free(screen->goos);
	}
	L4XV_U(f);

	screen->flags = 0;
	l4fb_init_screen(screen);
}

static int l4fb_init_session(struct fb_info *fb, struct l4fb_screen *screen)
{
	int ret;
	struct fb_var_screeninfo *const var = &fb->var;
	struct fb_fix_screeninfo *const fix = &fb->fix;
	l4re_video_goos_info_t *ginfo = &screen->ginfo;
	l4re_video_view_info_t vinfo;
	void *fb_addr = 0;

	L4XV_V(f);
	L4XV_L(f);
	ret = l4re_video_goos_info(screen->goos, ginfo);
	L4XV_U(f);
	if (ret) {
		dev_err(scr2dev(screen), "cannot get goos info (%d)\n", ret);
		return ret;
	}

	ret = -ENOMEM;

	/* check for strange combinations */
	if ((ginfo->flags & F_l4re_video_goos_dynamic_buffers) &&
	    !(ginfo->flags & F_l4re_video_goos_dynamic_views)) {
		dev_err(scr2dev(screen), "dynamic buffer + static view is not supported\n");
		return -EINVAL;
	}

	if (!(ginfo->flags & F_l4re_video_goos_dynamic_views)) {
		struct l4fb_view *view = l4fb_alloc_view();
		if (!view)
			return ret;

		L4XV_L(f);
		ret = l4re_video_goos_get_view(screen->goos, 0, &view->view);
		L4XV_U(f);
		if (ret) {
			dev_err(scr2dev(screen), "cannot get static view\n");
			kfree(view);
			return ret;
		}

		L4XV_L(f);
		ret = l4re_video_view_get_info(&view->view, &vinfo);
		L4XV_U(f);
		if (ret) {
			dev_err(scr2dev(screen), "cannot get view info\n");
			kfree(view);
			return ret;
		}

		list_add(&view->next_view, &screen->views);

		/* use the view's resolution as reference */
		var->xres = vinfo.width;
		var->yres = vinfo.height;
		fix->line_length = vinfo.bytes_per_line;

		var->red.offset = vinfo.pixel_info.r.shift;
		var->red.length = vinfo.pixel_info.r.size;
		var->green.offset = vinfo.pixel_info.g.shift;
		var->green.length = vinfo.pixel_info.g.size;
		var->blue.offset = vinfo.pixel_info.b.shift;
		var->blue.length = vinfo.pixel_info.b.size;
		var->bits_per_pixel = l4re_video_bits_per_pixel(&vinfo.pixel_info);
	} else {
		/* in this case the user must allocate views via the ioctls */
		/* use the goos info for the reference size */
		var->xres = ginfo->width;
		var->yres = ginfo->height;
		fix->line_length = ginfo->width * ginfo->pixel_info.bytes_per_pixel;

		var->red.offset = ginfo->pixel_info.r.shift;
		var->red.length = ginfo->pixel_info.r.size;
		var->green.offset = ginfo->pixel_info.g.shift;
		var->green.length = ginfo->pixel_info.g.size;
		var->blue.offset = ginfo->pixel_info.b.shift;
		var->blue.length = ginfo->pixel_info.b.size;
		var->bits_per_pixel = l4re_video_bits_per_pixel(&ginfo->pixel_info);
	}

	/* We cannot really set (smaller would work) screen paramenters
	 * when using con */
	if (var->bits_per_pixel == 15)
		var->bits_per_pixel = 16;

	if (l4_is_invalid_cap(screen->fb_cap = l4x_cap_alloc()))
		return ret;

	screen->flags |= ginfo->flags;

	/* allocate the fb memory if not static */
	if (ginfo->flags & F_l4re_video_goos_dynamic_buffers) {
		unsigned long size = fix->line_length * ginfo->height;
		size = l4_round_page(size);

		L4XV_L(f);
		ret = l4re_video_goos_create_buffer(screen->goos, size,
		                                    screen->fb_cap);
		L4XV_U(f);
		if (ret) {
			dev_err(scr2dev(screen), "cannot allocate fb (%d)\n", ret);
			return ret;
		}
		fix->smem_len = size;
	} else {
		long ret;
		L4XV_L(f);
		ret = l4re_video_goos_get_static_buffer(screen->goos, 0,
		                                        screen->fb_cap);
		if (ret >= 0)
			ret = l4re_ds_size(screen->fb_cap);
		L4XV_U(f);

		if (ret < 0) {
			dev_err(scr2dev(screen), "cannot get static fb (%ld)\n",
			        ret);
			return ret;
		}

		fix->smem_len = l4_round_page(ret);
	}

	L4XV_L(f);
	ret = l4re_rm_attach(&fb_addr, fix->smem_len, L4RE_RM_SEARCH_ADDR,
	                     screen->fb_cap, 0, 20);
	L4XV_U(f);

	if (ret < 0) {
		dev_err(scr2dev(screen), "cannot map fb memory (%d)\n", ret);
		return ret;
	}

	screen->fb_addr = (unsigned long)fb_addr;
	screen->fb_line_length = fix->line_length;
	fix->smem_start = (unsigned long)fb_addr;

	/* currently the virtual fb is equal to the screen */
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;

	fix->visual = FB_VISUAL_TRUECOLOR;

	dev_info(scr2dev(screen), "%dx%d@%d %dbypp, size: %d @ %lx\n",
	         var->xres, var->yres, var->bits_per_pixel,
	         screen->ginfo.pixel_info.bytes_per_pixel, fix->smem_len,
	         fix->smem_start);
	dev_info(scr2dev(screen), "%d:%d:%d %d:%d:%d linelen=%d visual=%d\n",
	         var->red.length, var->green.length, var->blue.length,
	         var->red.offset, var->green.offset, var->blue.offset,
	         fix->line_length, fix->visual);

	l4fb_update_dirty_init(screen, fix->smem_start, fix->smem_len);

	if (screen->refresh_sleep) {
		setup_timer(&screen->refresh_timer, l4fb_refresh_func, (unsigned long)screen);
		screen->refresh_timer.expires  = jiffies + screen->refresh_sleep;
		add_timer(&screen->refresh_timer);
	}

	return ret;
}

static int __init l4fb_probe(struct platform_device *dev)
{
	struct fb_info *info;
	struct l4fb_screen *screen = container_of(dev, struct l4fb_screen, platform_device);
	int video_cmap_len;
	int ret = -ENOMEM;

	if (disable)
		return -ENODEV;

	/* Process module parameters */
	if (refreshsleep >= 0) {
		u64 t = HZ * refreshsleep;
		do_div(t, 1000);
		screen->refresh_sleep = t;
	}

	info = framebuffer_alloc(0, &dev->dev);
	if (!info)
		goto failed_after_screen_alloc;

	info->fbops = &l4fb_ops;
	info->var   = l4fb_defined;
	info->fix   = l4fb_fix;
	info->pseudo_palette = screen->pseudo_palette;
	info->flags = FBINFO_FLAG_DEFAULT;

	ret = l4fb_init_session(info, screen);
	if (ret) {
		if (verbose)
			LOG_printf("init error %d\n", ret);
		goto failed_after_framebuffer_alloc;
	}

	info->screen_base = (void *)info->fix.smem_start;
	if (!info->screen_base) {
		dev_err(&dev->dev, "abort, graphic system could not be initialized.\n");
		ret = -EIO;
		goto failed_after_framebuffer_alloc;
	}

	/* some dummy values for timing to make fbset happy */
	info->var.pixclock     = 10000000 / info->var.xres * 1000 / info->var.yres;
	info->var.left_margin  = (info->var.xres / 8) & 0xf8;
	info->var.hsync_len    = (info->var.xres / 8) & 0xf8;

	info->var.transp.length = 0;
	info->var.transp.offset = 0;

	video_cmap_len = 16;

	info->fix.ypanstep  = 0;
	info->fix.ywrapstep = 0;

	ret = fb_alloc_cmap(&info->cmap, video_cmap_len, 0);
	if (ret < 0)
		goto failed_after_framebuffer_alloc;

	if (register_framebuffer(info) < 0) {
		ret = -EINVAL;
		goto failed_after_fb_alloc_cmap;
	}
	dev_set_drvdata(&dev->dev, info);

	dev_info(&dev->dev, "%s L4 frame buffer device (refresh: %ujiffies)\n",
	         info->fix.id, screen->refresh_sleep);

	l4fb_fb_init(info, screen);

	list_add(&screen->next_screen, &l4fb_screens);

	return 0;

failed_after_fb_alloc_cmap:
	fb_dealloc_cmap(&info->cmap);

failed_after_framebuffer_alloc:
	framebuffer_release(info);

failed_after_screen_alloc:
	l4fb_shutdown(screen);
	kfree(screen);

	return ret;
}

static int l4fb_alloc_screen(int id, char const *cap)
{
	struct l4fb_screen *screen;
	int ret;
	L4XV_V(f);

	printk(KERN_INFO "l4fb l4fb.%d: look for capability '%s' as goos session\n", id, cap);

	screen = kzalloc(sizeof(struct l4fb_screen), GFP_KERNEL);

	if (!screen)
		return -ENOMEM;

	l4fb_init_screen(screen);

	L4XV_L(f);
	ret = l4x_re_resolve_name(cap, &screen->goos);
	L4XV_U(f);

	if (ret) {
		printk("l4fb l4fb.%d: init failed err=%d\n", id, ret);
		kfree(screen);
		return ret;
	}

	screen->platform_device.id = id;
	screen->platform_device.name = "l4fb";
	ret = platform_device_register(&screen->platform_device);
	if (ret < 0) {
		dev_err(scr2dev(screen), "cannot register l4fb device (%d)\n", ret);
		kfree(screen);
		return ret;
	}
	return 0;
}

static int l4fb_remove(struct platform_device *device)
{
	struct fb_info *info = platform_get_drvdata(device);

	if (info) {
		struct l4fb_screen *screen = l4fb_screen(info);
		list_del(&screen->next_screen);
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);

		if (screen->keyb)
			input_unregister_device(screen->keyb);
		if (screen->mouse && screen->keyb != screen->mouse)
			input_unregister_device(screen->mouse);

		l4fb_shutdown(screen);
		kfree(screen);
	}
	return 0;
}

static struct platform_driver l4fb_driver = {
	.probe   = l4fb_probe,
	.remove  = l4fb_remove,
	.driver  = {
		.name = "l4fb",
	},
};

static int __init l4fb_init(void)
{
	int ret;
	int i;

	l4fb_update_rect = l4fb_l4re_update_rect;

	ret = platform_driver_register(&l4fb_driver);
	if (ret)
		return ret;

	for (i = 0; i < num_fbs; ++i) {
		ret = l4fb_alloc_screen(i, fbs[i]);
		if (ret)
			return ret;
	}
#ifdef CONFIG_L4_FB_DRIVER_DBG
	if (!IS_ERR(l4x_debugfs_dir))
		debugfs_dir = debugfs_create_dir("l4fb", NULL);
	if (!IS_ERR(debugfs_dir)) {
		debugfs_unmaps  = debugfs_create_u32("unmaps", S_IRUGO,
		                                     debugfs_dir, &stats_unmaps);
		debugfs_updates = debugfs_create_u32("updates", S_IRUGO,
		                                     debugfs_dir, &stats_updates);
	}
#endif
	return ret;
}
module_init(l4fb_init);

static void __exit l4fb_exit(void)
{
	struct l4fb_screen *screen, *tmp;
#ifdef CONFIG_L4_FB_DRIVER_DBG
	debugfs_remove(debugfs_unmaps);
	debugfs_remove(debugfs_updates);
	debugfs_remove(debugfs_dir);
#endif

	list_for_each_entry_safe(screen, tmp, &l4fb_screens, next_screen) {
		platform_device_unregister(&screen->platform_device);
	}
	platform_driver_unregister(&l4fb_driver);
}
module_exit(l4fb_exit);

MODULE_AUTHOR("Adam Lackorzynski <adam@os.inf.tu-dresden.de>");
MODULE_DESCRIPTION("Frame buffer driver for L4Re::Console's");
MODULE_LICENSE("GPL");


module_param(refreshsleep, int, 0444);
MODULE_PARM_DESC(refreshsleep, "Sleep between frame buffer refreshs in ms");
module_param(disable, bool, 0);
MODULE_PARM_DESC(disable, "Disable driver");
module_param(touchscreen, bool, 0);
MODULE_PARM_DESC(touchscreen, "Be a touchscreen");
module_param(singledev, bool, 0);
MODULE_PARM_DESC(singledev, "Expose only one input device");
module_param(abs2rel, bool, 0);
MODULE_PARM_DESC(abs2rel, "Convert absolute events to relative ones");
module_param(verbose, bool, 0);
MODULE_PARM_DESC(verbose, "Tell more");
module_param_array(fbs, charp, &num_fbs, 0200);
MODULE_PARM_DESC(fbs, "List of goos caps");
