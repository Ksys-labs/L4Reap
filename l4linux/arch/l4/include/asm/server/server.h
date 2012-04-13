#ifndef __ASM_L4__GENERIC__SERVER_H__
#define __ASM_L4__GENERIC__SERVER_H__

#include <l4/sys/utcb.h>

struct l4x_srv_object
{
	L4_CV long (*dispatch)(struct l4x_srv_object *, l4_umword_t obj,
	                       l4_utcb_t *msg, l4_msgtag_t *tag);
};

#ifdef __cplusplus

#define C_FUNC extern "C" L4_CV

#include <l4/sys/capability>
#include <l4/cxx/ipc_stream>
#include <l4/log/log.h>

L4::Cap<L4::Kobject>
l4x_srv_rcv_cap();

L4::Cap<void>
l4x_srv_register(l4x_srv_object *o, L4::Cap<L4::Thread> thread);

L4::Cap<void>
l4x_srv_register_name(l4x_srv_object *o,
                      L4::Cap<L4::Thread> thread,
                      const char *service);

template< typename T>
L4_CV
long l4x_srv_generic_dispatch(l4x_srv_object *_this, l4_umword_t obj,
                              l4_utcb_t *msg, l4_msgtag_t *tag)
{
	L4::Ipc_iostream ios(msg);
	ios.reset();
	ios.Istream::tag() = *tag;

	long r = static_cast<T*>(_this)->dispatch(obj, ios);

	if (r != -L4_ENOREPLY) {
		*tag = ios.reply(L4_IPC_BOTH_TIMEOUT_0, r);
		if ((*tag).has_error())
			LOG_printf("IPC failed (r=%ld, e=%ld)\n",
			           r, l4_error(*tag));
	}

	return r;
}


#else
#define C_FUNC L4_CV
#include <l4/sys/types.h>
#endif

C_FUNC l4_cap_idx_t
l4x_srv_register_c(struct l4x_srv_object *obj, l4_cap_idx_t thread);

C_FUNC l4_cap_idx_t
l4x_srv_register_name_c(struct l4x_srv_object *obj,
                        l4_cap_idx_t thread, const char *service);

C_FUNC void l4x_srv_init(void);
C_FUNC void l4x_srv_setup_recv(l4_utcb_t *u);

#endif /* __ASM_L4__GENERIC__SERVER_H__ */
