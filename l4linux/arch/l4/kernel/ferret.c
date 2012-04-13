/**
 * Ferret code for L4Linux.
 */

#include <linux/module.h>

#include <l4/log/log_printf.h>

#include <asm/generic/ferret.h>

#ifdef CONFIG_L4_FERRET_USER
ferret_list_local_t *l4x_ferret_user;
size_t l4x_ferret_user_size;
EXPORT_SYMBOL(l4x_ferret_user);
EXPORT_SYMBOL(l4x_ferret_user_size);
#endif

#ifdef CONFIG_L4_FERRET_KERNEL
ferret_list_local_t *l4x_ferret_kernel;
EXPORT_SYMBOL(l4x_ferret_kernel);
#endif

#ifdef CONFIG_L4_FERRET_SYSCALL_COUNTER
ferret_histo_t *l4x_ferret_syscall_ctr;
EXPORT_SYMBOL(l4x_ferret_syscall_ctr);
#endif

/**
 * \return 0 success
 *         1 error
 */
int l4x_ferret_init(void)
{
	int ret, inst;
	extern void *malloc(size_t);

	inst = ferret_create_instance();

#ifdef CONFIG_L4_FERRET_KERNEL
	if ((ret = ferret_create(FERRET_L4LX_MAJOR, FERRET_L4LX_LIST_MINOR,
	                         inst, FERRET_LIST,
	                         0, "64:50000",
	                         l4x_ferret_kernel,
	                         (void *(*)(size_t))(&malloc)))) {
		LOG_printf("Error creating kernel sensor: %d\n", ret);
		return 1;
	}
#endif

#ifdef CONFIG_L4_FERRET_USER
	if ((ret = ferret_create(FERRET_L4LXU_MAJOR, FERRET_L4LXU_MINOR,
	                         inst, FERRET_LIST,
	                         0, CONFIG_L4_FERRET_USER_CONFIG,
	                         l4x_ferret_user,
	                         (void *(*)(size_t))(&malloc)))) {
		LOG_printf("Error creating userland sensor: ret = %d\n", ret);
		return 1;
	}

	l4x_ferret_user_size = ferret_list_size(l4x_ferret_user->glob);
#endif

#ifdef CONFIG_L4_FERRET_SYSCALL_COUNTER
	if ((ret = ferret_create(FERRET_L4LX_MAJOR,
	                         FERRET_L4LX_SYSCALLCOUNT_MINOR,
	                         inst, FERRET_HISTO,
	                         0, "0:310:311",
	                         l4x_ferret_syscall_ctr,
	                         (void *(*)(size_t))(&malloc)))) {
		LOG_printf("Error creating syscall counter: ret = %d\n", ret);
		return 1;
	}
#endif

	return 0;
}
