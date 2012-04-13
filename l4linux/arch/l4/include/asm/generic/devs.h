#ifndef __ASM_L4__GENERIC__DEVS_H__
#define __ASM_L4__GENERIC__DEVS_H__

#include <asm/generic/io.h>

struct l4x_platform_callback_elem;

typedef void (*l4x_dev_cb_func_type)
      (l4io_device_handle_t dh,
       l4io_resource_handle_t rh,
       l4io_device_t *dev,
       struct l4x_platform_callback_elem *cb_elem);

struct l4x_platform_callback_elem {
	struct list_head      list;
	const char           *name;
	l4x_dev_cb_func_type  cb;
	void                 *privdata;
};


#define L4X_DEVICE_CB(name) \
        void name(l4io_device_handle_t dh, \
                  l4io_resource_handle_t rh, \
                  l4io_device_t *dev, \
                  struct l4x_platform_callback_elem *cb_node)


void l4x_arm_devices_init(void);

int
l4x_register_platform_device_callback(const char *name,
                                      l4x_dev_cb_func_type cb);


#endif /* ! __ASM_L4__GENERIC__DEVS_H__ */
