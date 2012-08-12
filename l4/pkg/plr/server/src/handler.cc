/*
 * Exception handling
 *
 * Here's where the real stuff is going on
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "manager"
#include "log"
#include "exceptions"
#include "emulation"
#include "app_loading"

#include <cassert>

#include <l4/sys/kdebug.h>
#include <l4/util/bitops.h>

#include <pthread-l4.h>
#include <l4/sys/segment.h>

#define MSG() DEBUGf(Romain::Log::Faults)
#define MSGi(inst) MSG() << "[" << (inst)->id() << "] "

EXTERN_C void *pthread_fn(void *data);
EXTERN_C void *pthread_fn(void *data)
{
	Romain::App_thread *t = (Romain::App_thread*)data;

	/* 
	 * Thread creation, copied from the example again.
	 */
	L4::Thread::Attr attr;
	attr.pager(L4::cap_reinterpret_cast<L4::Thread>(L4Re::Env::env()->rm()));
	attr.exc_handler(L4Re::Env::env()->main_thread());
//	attr.bind(t->vcpu_utcb(), L4Re::This_task);

	chksys(t->vcpu_cap()->control(attr), "control");
	chksys(t->vcpu_cap()->vcpu_control((l4_addr_t)t->vcpu()), "enable VCPU");

	l4_sched_param_t sp = l4_sched_param(2);
	sp.affinity = l4_sched_cpu_set(t->cpu(), 0);
	chksys(L4Re::Env::env()->scheduler()->run_thread(t->vcpu_cap(),
	                                                 sp));

#if 0
	MSG() << "!" << std::hex << (void*)t->thread_sp();
	MSG() << "?" << std::hex << (void*)t->handler_sp();
#endif
	asm volatile("mov %0, %%esp\n\t"
				 "jmp *%1\n\t"
				 :
				 : "r" (t->handler_sp()),
				   "r" (t->thread_entry())
				 );

	enter_kdebug("blub?");

	return 0;
}

#if SPLIT_HANDLING

struct SplitInfo {
	Romain::InstanceManager *m;
	Romain::App_instance    *i;
	Romain::App_thread      *t;
	Romain::App_model       *a;
	l4_cap_idx_t             cap;

	SplitInfo()
		: m(0), i(0), t(0), a(0), cap(L4_INVALID_CAP)
	{ }

};

#define SYNC_IPC 1

class SplitHandler
{
	l4_cap_idx_t             _split_handler;
	Romain::InstanceManager *_im;
	Romain::Replicator       _replicator;
	SplitInfo **             _psi;
	unsigned long *          _checksums;

	void wait_for_instances()
	{
		for (unsigned cnt = 0; cnt < _im->instance_count(); ++cnt) {
#if SYNC_IPC
			l4_umword_t label  = 0;
			l4_msgtag_t t      = l4_ipc_wait(l4_utcb(), &label, L4_IPC_NEVER);
			//MSG() << "Split handler notified: " << std::hex << t.label();
			_psi[cnt]          = (SplitInfo*)l4_utcb_mr()->mr[0];
			_checksums[cnt]    = _psi[cnt]->t->csum_state();
#else
			MSG() << (void*)&_psi[cnt] << " " << _psi[cnt];

			while (_psi[cnt] == 0) {
				//MSG() << (void*)_psi[cnt];
				l4_thread_yield();
			}

			MSG() << (void*)&_psi[cnt] << " " << _psi[cnt] << "  Split handler notified";
#endif
		}

	}


	bool validate_instances()
	{
		for (unsigned cnt = 1; cnt < _im->instance_count(); ++cnt) {
			if (_checksums[cnt] != _checksums[cnt-1]) {
				ERROR() << "State mismatch detected!";
				ERROR() << "=== vCPU states ===";
				
				for (unsigned i = 0; i < _im->instance_count(); ++i) {
					ERROR() << "Instance " << _psi[i]->i->id() << " "
					        << "csum " << std::hex << _psi[i]->t->csum_state();
					_psi[i]->t->vcpu()->print_state();
				}

				return false;
			}
		}
		return true;
	}

	void handle_fault()
	{
		L4vcpu::Vcpu *vcpu = _psi[0]->t->vcpu();
		unsigned trap      = _psi[0]->t->vcpu()->r()->trapno;

		while (trap) {
			MSGi(_psi[0]->i) << "\033[33;1mTRAP 0x"
			                 << std::hex << vcpu->r()->trapno
			                 << " @ 0x" << vcpu->r()->ip << "\033[0m";

			Romain::Observer::ObserverReturnVal v 
				= _im->fault_notify(_psi[0]->i, _psi[0]->t, _psi[0]->a);

			switch(v) {
				case Romain::Observer::Finished:
					{
						for (unsigned c = 1; c < _im->instance_count(); ++c) {
							_im->fault_notify(_psi[c]->i, _psi[c]->t, _psi[c]->a);
						}
					}
					break;
				case Romain::Observer::Replicatable:
					{
						_replicator.put(_psi[0]->t);
						for (unsigned c = 1; c < _im->instance_count(); ++c) {
							_replicator.get(_psi[c]->t);
						}
					}
					break;
				default:
					enter_kdebug("notify?");
					break;
			}

			if ((trap = _psi[0]->t->get_pending_trap()) != 0)
				_psi[0]->t->vcpu()->r()->trapno = trap;
		}
	}


	void resume_instances()
	{
		for (unsigned c = 0; c < _im->instance_count(); ++c) {
			MSGi(_psi[c]->i) << "Resuming instance @ " << std::hex << _psi[c]->t->vcpu()->r()->ip;
			_psi[c]->t->commit_client_gdt();
#if SYNC_IPC
			l4_ipc_send(_psi[c]->cap, l4_utcb(), l4_msgtag(0,0,0,0), L4_IPC_NEVER);
#else
			_psi[c]    = 0;
#endif
		}
		MSG() << "... resumed.";
	}

	public:

		static SplitHandler*     _handlers[5];

		static SplitHandler* get(unsigned idx)
		{
			assert(idx == 0); // for now
			return _handlers[idx];
		}

		void notify(Romain::App_instance* i,
		            Romain::App_thread* t,
		            Romain::App_model* a)
		{
#if SYNC_IPC
			SplitInfo si;

			//MSGi(i) << "split handler is " << std::hex << SplitHandler::split_handler_cap();

			//si.m   = _im;
			si.i   = i;
			si.t   = t;
			si.a   = a;
			si.cap = t->vcpu_cap().cap();

			l4_utcb_mr()->mr[0] = (l4_umword_t)&si;
			l4_msgtag_t tag = l4_msgtag(0xF00, 1, 0, 0);
			l4_msgtag_t res = l4_ipc_call(SplitHandler::split_handler_cap(),
			                              l4_utcb(), tag, L4_IPC_NEVER);
#else
			SplitInfo si;
			si.i = i;
			si.t = t;
			si.a = a;
			psi(i->id(), &si);
			MSG() << (void*)&_psi[i->id()] << " " << (void*)&si;

			while (_psi[i->id()] != 0) {
				l4_thread_yield();
			}
#endif
			MSGi(i) << "handled.";
		}

		SplitHandler(Romain::InstanceManager* im)
			: _split_handler(pthread_getl4cap(pthread_self())),
		      _im(im), _replicator()
		{
			_psi       = new SplitInfo*[_im->instance_count()];
			_checksums = new unsigned long [_im->instance_count()];
			memset(_psi, 0, sizeof(SplitInfo*) * _im->instance_count());
		}


		~SplitHandler()
		{
			delete [] _psi;
			delete [] _checksums;
		}


		void run()
		{
			MSG() << "Instance mgr: " << (void*)_im;
			MSG() << "Instances: " << _im->instance_count();

			while (true) {
				wait_for_instances();

				DEBUG() << "received faults from " << _im->instance_count()
						<< " instances...";

				if (!validate_instances())
					enter_kdebug("recover");

				handle_fault();

				resume_instances();
			}
		}

		void psi(unsigned idx, SplitInfo* si)
		{ _psi[idx] = si; }

		l4_cap_idx_t split_handler_cap()
		{ return _split_handler; }
};


SplitHandler* SplitHandler::_handlers[5] = {0};


EXTERN_C void *split_handler_fn(void* data)
{
	//SplitHandler::split_handler_cap(pthread_getl4cap(pthread_self()));

	SplitHandler::_handlers[0] = new SplitHandler(reinterpret_cast<Romain::InstanceManager*>(data));
	SplitHandler::get(0)->run();

	enter_kdebug("split_handler terminated!");
	return 0;
}

#endif // SPLIT_HANDLING

void __attribute__((noreturn)) Romain::InstanceManager::VCPU_startup(Romain::InstanceManager *,
                                                                     Romain::App_instance *i,
                                                                     Romain::App_thread *t,
                                                                     Romain::App_model*am)
{
	L4vcpu::Vcpu *vcpu = t->vcpu();
	vcpu->task(i->vcpu_task());
	vcpu->r()->sp = (l4_umword_t)am->stack()->relocate(am->stack()->ptr());
	vcpu->print_state();

	struct timeval tv;
	gettimeofday(&tv, 0);
	if (Romain::Log::withtime)
		INFO() << "\033[33;1mStarting @ " << tv.tv_sec << "." << tv.tv_usec << "\033[0m";
	else
		INFO() << "\033[33;1mStarting\033[0m";

	MSGi(i) << "Resuming instance ...";

	L4::Cap<L4::Thread> cap = t->vcpu_cap();

	t->commit_client_gdt();

	cap->vcpu_resume_commit(cap->vcpu_resume_start());

	enter_kdebug("after resume");
	l4_sleep_forever();
}


static void local_vCPU_handling(Romain::InstanceManager *m,
                                Romain::App_instance *i,
                                Romain::App_thread *t,
                                Romain::App_model *a)
{
	// XXX: At this point we might want to reset the GDT to the view that is
	//      expected by the master task, which might differ from the client.

	L4vcpu::Vcpu *vcpu = t->vcpu();
	unsigned trap = t->vcpu()->r()->trapno;

	/*
	 * We potentially handle multiple traps here: As we are emulating a bunch
	 * of instructions (e.g., write PF emulation), some real instructions are never
	 * actually executed and may thus not raise certain exceptions such as the INT1 single
	 * step exception. Observers emulating behavior need to be aware of that and inject
	 * a new exception in such circumstances.
	 */
	while (trap) {
		MSGi(i) << "\033[33;1mTRAP 0x" << std::hex << vcpu->r()->trapno
		        << " @ 0x" << vcpu->r()->ip << "\033[0m";

		Romain::Observer::ObserverReturnVal v         = Romain::Observer::Invalid;

		/*
		 * Enter redundancy mode. May cause vCPU to block until leader vCPU executed
		 * its handlers.
		 */
		Romain::RedundancyCallback::EnterReturnVal rv = m->redundancy()->enter(i,t,a);
		//MSGi(i) << "red::enter: " << rv;

		/*
		 * Case 1: we are the first to exec this system call.
		 */
		if (rv == Romain::RedundancyCallback::First_syscall) {
			v = m->fault_notify(i,t,a);
			MSGi(i) << "fault_notify: " << v;
			switch(v) {
				case Romain::Observer::Finished:
				case Romain::Observer::Finished_wait:
				case Romain::Observer::Finished_step:
				case Romain::Observer::Finished_wakeup:
					//MSGi(i) << "leader_repeat()";
					m->redundancy()->leader_repeat(i,t,a);
					break;
				case Romain::Observer::Replicatable:
					m->redundancy()->leader_replicate(i,t,a);
					break;
				default:
					//enter_kdebug("fault was not finished/replicatable");
					break;
			}
		}
		/*
		 * Case 2: leader told us to do the syscall ourselves
		 */
		else if (rv == Romain::RedundancyCallback::Repeat_syscall) {
			v = m->fault_notify(i,t,a);
			MSGi(i) << "fault_notify: " << v;
		}

		/*
		 * Special handling for the Finished_{wakeup,wait,step} cases
		 * used by the fault injector.
		 */
		switch(v) {
			case Romain::Observer::Finished_wait:
				// go to sleep until woken up
				m->redundancy()->wait(i,t,a);
				break;
			case Romain::Observer::Finished_wakeup:
				// wakeup -> first needs to ensure that all other
				// vCPUs are actually waiting
				m->redundancy()->silence(i,t,a);
				m->redundancy()->wakeup(i,t,a);
				break;
			case Romain::Observer::Finished_step:
				// let other vCPUs step until they go to sleep
				m->redundancy()->silence(i,t,a);
				break;
			default:
				break;
		}

		if ((trap = t->get_pending_trap()) != 0)
			t->vcpu()->r()->trapno = trap;

		m->redundancy()->resume(i, t, a);
	}

	//vcpu->print_state();
    MSGi(i) << "Resuming instance @ " << std::hex << t->vcpu()->r()->ip;
	t->commit_client_gdt();
}


#if SPLIT_HANDLING
static void split_vCPU_handling(Romain::InstanceManager *m,
                                Romain::App_instance *i,
                                Romain::App_thread *t,
                                Romain::App_model *a)
{
	MSG() << "here";
	SplitHandler::get(0)->notify(i,t,a);
	t->commit_client_gdt();
}
#endif // SPLIT_HANDLING


#if MIGRATE_VCPU
static void migrated_vCPU_handling(Romain::InstanceManager *m,
                                   Romain::App_instance *i,
                                   Romain::App_thread *t,
                                   Romain::App_model *a)
{
	l4_sched_param_t sp = l4_sched_param(2);
	sp.affinity = l4_sched_cpu_set(0, 0);
	chksys(L4Re::Env::env()->scheduler()->run_thread(t->vcpu_cap(),
													 sp));

	local_vCPU_handling(m, i, t, a);

	sp = l4_sched_param(2);
	sp.affinity = l4_sched_cpu_set(t->cpu(), 0);
	chksys(L4Re::Env::env()->scheduler()->run_thread(t->vcpu_cap(),
													 sp));
}
#endif // MIGRATE_VCPU


/*
 * VCPU fault entry point.
 *
 * Calls the registered observers and keeps track of any further faults that might get
 * injected during handler execution.
 */
void __attribute__((noreturn)) Romain::InstanceManager::VCPU_handler(Romain::InstanceManager *m,
                                                                     Romain::App_instance *i,
                                                                     Romain::App_thread *t,
                                                                     Romain::App_model *a)
{
	L4vcpu::Vcpu *vcpu = t->vcpu();
	vcpu->state()->clear(L4_VCPU_F_EXCEPTIONS | L4_VCPU_F_DEBUG_EXC);
	handler_prolog(t);

#if MIGRATE_VCPU
	migrated_vCPU_handling(m, i, t, a);
#elif SPLIT_HANDLING
	split_vCPU_handling(m, i, t, a);
#elif LOCAL_HANDLING
	local_vCPU_handling(m, i, t, a);
#else
#error No vCPU handling method selected!
#endif
	L4::Cap<L4::Thread> self;
	self->vcpu_resume_commit(self->vcpu_resume_start());

	enter_kdebug("after resume");
	l4_sleep_forever();
}
