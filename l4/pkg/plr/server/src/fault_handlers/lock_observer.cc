#include "lock_observer.h"

#define DEBUGt(t) DEBUG() <<  "[" << t->vcpu() << "] "

Romain::PThreadLockObserver*
Romain::PThreadLockObserver::Create()
{ return new PThreadLock_priv(); }


void Romain::PThreadLock_priv::status() const { }


Romain::Observer::ObserverReturnVal
Romain::PThreadLock_priv::notify(Romain::App_instance* inst,
                                 Romain::App_thread*   thread,
                                 Romain::Thread_group* group,
                                 Romain::App_model*    model)
{
	if (!entry_reason_is_int3(thread->vcpu(), inst, model)) {
		return Romain::Observer::Ignored;
	}

	/*
	 * HACK: intercept notifications coming from the replication-
	 *       aware pthread library.
	 */
	if (thread->vcpu()->r()->ip == 0xA021) {
		det_lock(inst, thread, group, model);
		//thread->vcpu()->r()->flags |= TrapFlag;
		return Romain::Observer::Replicatable;
	} else if (thread->vcpu()->r()->ip == 0xA041) {
		det_unlock(inst, thread, group, model);
		return Romain::Observer::Replicatable;
	}

#define HANDLE_BP(var, handler) do { \
		if(_functions[(var)].orig_address == thread->vcpu()->r()->ip - 1) { \
			(handler)(inst, thread, group, model); \
			return Romain::Observer::Replicatable; \
		} \
	} while (0)

	HANDLE_BP(mutex_lock_id, mutex_lock);
	HANDLE_BP(mutex_unlock_id, mutex_unlock);
	HANDLE_BP(mutex_init_id, mutex_init);
	HANDLE_BP(pt_lock_id, lock);
	HANDLE_BP(pt_unlock_id, unlock);

#undef HANDLE_BP

	return Romain::Observer::Ignored;
}

void Romain::PThreadLock_priv::startup_notify(Romain::App_instance *inst,
                                              Romain::App_thread *,
                                              Romain::Thread_group *,
                                              Romain::App_model *am)
{
	static unsigned callCount = 0;
	lock_info *lip            = reinterpret_cast<lock_info*>(_lip_local);

	if (!callCount) {
#if INTERNAL_DETERMINISM
		_lip_ds           = am->alloc_ds(L4_PAGESIZE);
		_lip_local        = am->local_attach_ds(_lip_ds, L4_PAGESIZE, 0);
		INFO() << "Local LIP address: " << std::hex << _lip_local;
		lip               = reinterpret_cast<lock_info*>(_lip_local);
		void* remote__lip = (void*)am->prog_attach_ds(Romain::LOCK_INFO_PAGE,
		                                              L4_PAGESIZE, _lip_ds, 0, 0,
		                                              "lock info page",
		                                              _lip_local, true);

		am->lockinfo_local(_lip_local);
		am->lockinfo_remote(Romain::LOCK_INFO_PAGE);

		memset((void*)_lip_local, 0, L4_PAGESIZE);

		lip->locks[0].lockdesc = 0xFFFFFFFF;
		lip->locks[0].owner    = 0xDEADBEEF;

#endif
		//DEBUG() << "Replica LIP address: " << std::hex << remote_lock_info;

		for (unsigned idx = 0; idx < pt_max_wrappers; ++idx) {
			DEBUG() << idx;
			_functions[idx].activate(inst, am);
		}
	}
	callCount++;
	lip->replica_count += 1;
	//enter_kdebug();
}


/*
 * pthread_mutex_init()
 *   - mutex address    @ ESP + 4
 *   - mutex attrib ptr @ ESP + 8
 *   - returns int in EAX
 */
void Romain::PThreadLock_priv::mutex_init(Romain::App_instance *inst,
                                          Romain::App_thread *t,
                                          Romain::Thread_group *tg,
                                          Romain::App_model *am)
{
	l4_addr_t stack  = am->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
	l4_umword_t ret  = *(l4_umword_t*)stack;
	l4_umword_t lock = *(l4_umword_t*)(stack + 1*sizeof(l4_umword_t));
	l4_umword_t attr = *(l4_umword_t*)(stack + 2*sizeof(l4_umword_t));

	DEBUG() << "INIT: lock @ " << std::hex << lock << ", attr " << attr;
	if (attr != 0) {
		enter_kdebug("init with attributes");
	}

	_locks[lock] = new PThreadMutex(false /*XXX*/);

	t->vcpu()->r()->ax  = 0;
	t->vcpu()->r()->sp += sizeof(l4_umword_t);
	t->vcpu()->r()->ip  = ret;
}


/*
 * __pthread_lock():
 * 		- called with lock pointer in EAX
 * 		- returns void
 */
void Romain::PThreadLock_priv::lock(Romain::App_instance *inst,
                                    Romain::App_thread *t,
                                    Romain::Thread_group *tg,
                                    Romain::App_model *am)
{
	l4_addr_t stack  = am->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
	l4_umword_t ret  = *(l4_umword_t*)stack;
	l4_umword_t lock = t->vcpu()->r()->ax;

	DEBUG() << "Stack ptr " << std::hex << t->vcpu()->r()->sp << " => "
         	<< stack;
	DEBUG() << "Lock @ " << std::hex << lock;
	DEBUG() << "Return addr " << std::hex << ret;

	//enter_kdebug("lock");

	lookup_or_create(lock)->lock(tg);

	t->vcpu()->r()->sp += sizeof(l4_umword_t); // RET: inc. ESP
	t->vcpu()->r()->ip  = ret;                 // RET: return addr
}


/*
 * __pthread_unlock():
 * 		- called with lock pointer as single stack argument
 * 		- returns int
 */
void Romain::PThreadLock_priv::unlock(Romain::App_instance *inst,
                                      Romain::App_thread *t,
                                      Romain::Thread_group *tg,
                                      Romain::App_model *am)
{
	l4_addr_t stack     = am->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
	l4_umword_t retaddr = *(l4_umword_t*)stack;
	l4_umword_t lock    = *(l4_umword_t*)(stack + 1*sizeof(l4_umword_t));

	DEBUG() << "Return addr " << std::hex << retaddr;
	DEBUG() << "Lock @ " << std::hex << lock;

	//enter_kdebug("unlock");

	int ret = lookup_or_fail(lock)->unlock();

	t->vcpu()->r()->ax  = ret;
	t->vcpu()->r()->sp += sizeof(l4_umword_t); // RET: inc. ESP
	t->vcpu()->r()->ip  = retaddr;             // RET: return addr
}

/*
 * pthread_mutex_lock()
 * 		- mutex   @ ESP + 4
 * 		- returns int
 */
void Romain::PThreadLock_priv::mutex_lock(Romain::App_instance* inst, Romain::App_thread* t,
                                          Romain::Thread_group* group, Romain::App_model* model)
{
	l4_addr_t stack     = model->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
	l4_umword_t retaddr = *(l4_umword_t*)stack;
	l4_umword_t lock    = *(l4_umword_t*)(stack + 1*sizeof(l4_umword_t));

	//DEBUG() << "lock @ " << std::hex << lock << " ESP.local = " << stack;
	PThreadMutex* mtx = _locks[lock];
	if (!mtx) {
		/*
		 * Can't use the shortcut here. We found a yet unknown lock. Now we need to 
		 * figure out whether it should be recursive. The respective member in the
		 * pthread data structure is at offset 12 (dec) from the mutex pointer
		 * we received as the argument.
		 */
		l4_umword_t mtx_kind_ptr = model->rm()->remote_to_local(lock + 12, inst->id());
#if 0
		DEBUG() << "log kind: " << *(l4_umword_t*)mtx_kind_ptr
		        << " RECURSIVE: " << PTHREAD_MUTEX_RECURSIVE_NP;
#endif

		mtx              = new PThreadMutex(*(l4_umword_t*)mtx_kind_ptr == PTHREAD_MUTEX_RECURSIVE_NP);
		_locks[lock]     = mtx;
	}

	int ret = mtx->lock(group);

	t->vcpu()->r()->ax  = ret;
	t->vcpu()->r()->sp += sizeof(l4_umword_t); // RET: inc. ESP
	t->vcpu()->r()->ip  = retaddr;             // RET: return addr
}


void Romain::PThreadLock_priv::mutex_unlock(Romain::App_instance* inst, Romain::App_thread* t,
                                            Romain::Thread_group* group, Romain::App_model* model)
{
	l4_addr_t stack     = model->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
	l4_umword_t retaddr = *(l4_umword_t*)stack;
	l4_umword_t lock    = *(l4_umword_t*)(stack + 1*sizeof(l4_umword_t));

	int ret = lookup_or_fail(lock)->unlock();
	DEBUG() << "unlock @ " << std::hex << lock << " = " << ret;

	t->vcpu()->r()->ax  = ret;
	t->vcpu()->r()->sp += sizeof(l4_umword_t); // RET: inc. ESP
	t->vcpu()->r()->ip  = retaddr;             // RET: return addr
}
