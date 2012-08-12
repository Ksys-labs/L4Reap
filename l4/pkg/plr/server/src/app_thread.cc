/*
 * app_thread.cc --
 *
 *     App_thread functions for creating and preparing a new VCPU
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "app"
#include "app_loading"

#include <pthread-l4.h>

void Romain::App_thread::alloc_vcpu_mem()
{
	/* Alloc vUTCB */
	l4_addr_t kumem;
	L4Re::Util::kumem_alloc(&kumem, 0);
	_check(kumem == 0, "out of memory in kumem_alloc");

	_vcpu_utcb = (l4_utcb_t *)kumem;
	_vcpu = L4vcpu::Vcpu::cast(kumem + L4_UTCB_OFFSET);

	/* store segment registers - stolen from the example */
	//_vcpu->r()->gs = _stored_ds;
	_vcpu->r()->fs = _stored_ds;
	_vcpu->r()->es = _stored_ds;
	_vcpu->r()->ds = _stored_ds;
	_vcpu->r()->ss = _stored_ds;

	/* We want to catch ALL exceptions for this vCPU. */
	_vcpu->saved_state()->set(
	                          L4_VCPU_F_IRQ
	                          | L4_VCPU_F_PAGE_FAULTS
	                          | L4_VCPU_F_EXCEPTIONS
	                          | L4_VCPU_F_DEBUG_EXC
	                          | L4_VCPU_F_USER_MODE
	                          | L4_VCPU_F_FPU_ENABLED
	);

	DEBUG() << "VCPU: utcb = " << (void*)vcpu_utcb()
	        << " vcpu @ " << (void*)vcpu();
}


void Romain::App_thread::touch_stacks()
{
	/* We need to touch at least the handler stack, because upon entry, the vCPU
	 * still has interrupts enabled and we must not raise one by causing a page
	 * fault on the stack area.
	 */
	DEBUG() << "Stack info:";
	DEBUG() << "   handler stack @ " << (void*)_handler_stack
	        << " - " << (void*)(_handler_stack + sizeof(_handler_stack));

	l4_touch_rw(_handler_stack, sizeof(_handler_stack));
}


void Romain::App_thread::alloc_vcpu_cap()
{
#if 0
	_vcpu_cap = chkcap(L4Re::Util::cap_alloc.alloc<L4::Thread>(),
	                   "vCPU cap alloc");
	chksys(L4Re::Env::env()->factory()->create_thread(_vcpu_cap),
	       "create thread");
	l4_debugger_set_object_name(_vcpu_cap.cap(), "vcpu thread");
#endif
}


extern "C" void* pthread_fn(void *);

/*
 * Create a replica thread
 */
void Romain::App_thread::start()
{
	/*
	 * We only set handler IP and SP here, because beforehand our creator
	 * may have modified them.
	 */
	_vcpu->entry_sp(handler_sp());
	_vcpu->entry_ip((l4_umword_t)_handler_fn);

	int err = pthread_create(&_pthread, NULL, pthread_fn, this);
	_check(err != 0, "pthread_create");

	_vcpu_cap = L4::Cap<L4::Thread>(pthread_getl4cap(_pthread));
}


/*
 * Calculate checksum of the replica's state
 *
 * This checksum is used to compare replica states.
 */
unsigned long
Romain::App_thread::csum_state()
{
	// XXX: this should also include the UTCB, which
	//      means the _used_ part of the UTCB
	return _vcpu->r()->ip
	     + _vcpu->r()->sp
	     + _vcpu->r()->ax
	     + _vcpu->r()->bx
	     + _vcpu->r()->cx
	     + _vcpu->r()->dx
	     + _vcpu->r()->bp
#if 0
	     + _vcpu->r()->fs
	     + _vcpu->r()->gs
#endif
	     ;
}
