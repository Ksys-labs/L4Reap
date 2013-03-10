#pragma once

/*
 * lock_observer.h --
 *
 *     Deterministic lock acquisition
 *
 * (c) 2012-2013 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "observers.h"
#include "../instruction_length.h"

#define EXTERNAL_DETERMINISM 0
#define INTERNAL_DETERMINISM 1

EXTERN_C_BEGIN
#include <l4/plr/pthread_rep.h>
EXTERN_C_END

namespace Romain
{
	/*
	 * Wrapper for a pthread mutex.
	 *
	 * We delegate the actual locking (management, sleep handling) work to a
	 * pthread mutex.
	 *
	 * Additionally, we add a wrapper that allows replicated threads to acquire
	 * a recursive mutex even if the thread calling lock() the 2nd time is !=
	 * the first lock acquirer.
	 */
	class PThreadMutex
	{
		pthread_mutex_t mtx;          // the mutex
		sem_t           sem;
		bool recursive;               // this is a recursive mtx?
		Romain::Thread_group* owner;  // owning thread group
		unsigned counter;             // recursive lock counter
		
		public:

		PThreadMutex(bool rec)
			: recursive(rec), owner(0), counter(0)
		{
			/*
			 * Even if the mutex is recursive, we only create a
			 * simple one here, because we deal with lock recursion
			 * ourselves.
			 * 
			 * (Actually, we cannot use pthread recursion, because it
			 * checks that the recursive call comes from the same thread,
			 * which may not be the case due to replication).
			 */ 
			sem_init(&sem, 0, 1);
			pthread_mutex_init(&mtx, 0);
		}


		/*
		 * Acquire mutex
		 */
		int lock(Romain::Thread_group* tg)
		{
			pthread_mutex_lock(&mtx);
			/*
			 * If this is recursive, only increment the counter.
			 */
			if (recursive and counter) {
				if (tg != owner) {
					pthread_mutex_unlock(&mtx);
					return 1; // EPERM
				}
				counter++;
				pthread_mutex_unlock(&mtx);
				return 0;
			}

			owner = tg;
			counter++;
			pthread_mutex_unlock(&mtx);

			return sem_wait(&sem);
			//return pthread_mutex_lock(&mtx);
		}


		/*
		 * Release mutex
		 */
		int unlock()
		{
			pthread_mutex_lock(&mtx);
			counter--;

			if (recursive and (counter == 0)) {
				owner = 0;
			}
			pthread_mutex_unlock(&mtx);

			return sem_post(&sem);
			//return pthread_mutex_unlock(&mtx);
		}
	};

	
	struct LockFunction
	{
		l4_addr_t   orig_address;
		Breakpoint  *bp;
		unsigned    function_id;
		l4_addr_t   wrapper_address;


		LockFunction()
			: orig_address(0), bp(0), function_id(~0UL), wrapper_address(0)
		{ }


		void configure(char const *configString, char const *alternativeString,
		               unsigned id)
		{
			DEBUG() << configString;
			orig_address    = ConfigIntValue(configString);
#if INTERNAL_DETERMINISM
			wrapper_address = ConfigIntValue(alternativeString);
			function_id     = id;
#else
			bp           = new Breakpoint(orig_address);
#endif
			DEBUG() << std::hex << orig_address;
		}


		void activate(Romain::App_instance *inst, Romain::App_model* am)
		{
#if INTERNAL_DETERMINISM
			if (((function_id == pt_lock_id) or
			     (function_id == pt_unlock_id)) and 
				(orig_address != ~0UL)) {
				/* Assumption: __pthread_(un)lock are called seldomly, so
				 * we simply use the bp functionality here */
				bp = new Breakpoint(orig_address);
				bp->activate(inst, am);
			} else {
				do_patch(am);
			}
#else
			DEBUG() << lockID_to_str(function_id);
			if ((orig_address == 0) or (orig_address == ~0))
				return;
			bp = new Breakpoint(orig_address);
			bp->activate(inst, am);
			DEBUG() << "BP set.";
#endif
		}


		void do_patch(Romain::App_model *am)
		{
#if INTERNAL_DETERMINISM
			DEBUG() << "=============== Patching " << PURPLE << lockID_to_str(function_id)
			        << NOCOLOR << " ===============";

			lock_info* lockinfo = reinterpret_cast<lock_info*>(am->lockinfo_local());
			if (wrapper_address == ~0) {
				DEBUG() << BLUE << "   no known address" << NOCOLOR;
				return;
			}

			unsigned char *instructionBuffer = lockinfo->trampolines + function_id * 32;
			memset(instructionBuffer, 0xff, 32);
			instructionBuffer[0] = 0xCC; // INT3
			instructionBuffer[1] = 0xC3; // RET

			return;

			/* XXX: Dead code below ... at least for now.  -> This is how we
			 *      would patch the binary if we did not have access to the
			 *      pthread implementation.
			 */
			
			DEBUG() << "Patching function at " << std::hex << orig_address;
			
			l4_addr_t func_local = am->rm()->remote_to_local(orig_address, 0);
			DEBUG() << "function @ " << std::hex << orig_address
			        << " = " << func_local;
			DEBUG() << "Wrapper @ " << std::hex << wrapper_address;

			DEBUG() << "Original code:";
			Romain::dump_mem((void*)func_local, 20);

			/*
			 * Patch the target code. We overwrite the first bytes of the
			 * function with a CALL into our handler function. Later, we make
			 * the handler return so that it executes the original code.
			 * 
			 * To achieve this, we copy the function's first N instructions
			 * into a separate buffer. The buffer is then extended with a
			 * jump back to the original (non-overwritten) code. For the jump
			 * back, we use a jmp through EAX, hence we need 2 additional
			 * instructions to save and restore EAX' original value.
			 * 
			 * Hence, 6 bytes of code to be replaced in the original code
			 * (5 for the CALL, 1 for PUSHing EAX).
			 */

			unsigned bytes = 0;
			l4_addr_t ip   = func_local;
			while (bytes < 6) {
				Romain::InstructionPrinter(ip, 0);
				unsigned len = mlde32((void*)ip);
				bytes += len;
				ip += len;
			}

			DEBUG() << "Need to store away " << bytes << " bytes. Then return to 0x"
			        << std::hex << orig_address + bytes;

#if 0
			/*
			 * -----------------------------------------------------
			 *  Create return code in the trampoline page 
			 * -----------------------------------------------------
			 */
			// POP EAX -> this reg was used by wrapper fn to determine
			// where to jump to
			instructionBuffer[0] = 0x58;
			// copy the instructions into buffer
			memcpy(instructionBuffer + 1, (void*)func_local, bytes);
			// + 1 byte:  PUSH eax
			instructionBuffer[bytes + 1] = 0x50;
			// + 5 bytes: MOV eax, $retaddr
			instructionBuffer[bytes + 2] = 0xB8;
			/* Returning to the instruction _after_ our patched JMP */
			*(l4_addr_t*)&instructionBuffer[bytes+3] = orig_address + 5;
			// + 2 bytes: JMP [eax]
			instructionBuffer[bytes + 7] = 0xFF; // 2 bytes: jmp *%eax
			instructionBuffer[bytes + 8] = 0xE0;
			DEBUG() << "Return buffer: ";
			Romain::dump_mem(instructionBuffer, 32, 24);
#endif

			/*
			 * -----------------------------------------------------
			 *  Patch the target function to jump to our wrapper
			 * -----------------------------------------------------
			 */
			DEBUG() << "Patching original code... (ibuf @ " << std::hex
			        << (void*)instructionBuffer << ")";
			memset((void*)func_local, 0x90, bytes);
			// 5 bytes: JMP [wrapper_func]
			*((char*)func_local   ) = 0xE9;

			int offset = wrapper_address - orig_address - 5;
			DEBUG() << "Offset: " << std::hex << wrapper_address
			        << " - " << orig_address << " = " << offset;
			*(l4_addr_t*)(func_local + 1) = offset;
#if 0
			// 1 byte: POP eax
			*((char*)func_local + 5) = 0x58;
#endif
			DEBUG() << "Patched function entry: ";
			Romain::dump_mem((char*)func_local, 32, 24);


#endif
		}
	};


	/*
	 * Observer responsible for making multithreaded lock acquisition
	 * deterministic. We achieve this by placing breakpoints on the
	 * following well-known pthread functions:
	 *
	 * __pthread_lock    (internal pthread use)
	 * __pthread_unlock  (internal pthread use)
	 * pthread_mutex_init
	 * pthread_mutex_lock
	 * pthread_mutex_unlock
	 *
	 * Unsupported so far:
	 *
	 * pthread_mutex_destroy
	 * pthread_mutex_trylock
	 * pthread_mutex_timedlock
	 *
	 * Then we intercept all calls to these functions and emulate them
	 * ourselves. As the observer is only called after all replicas
	 * agreed to grab a lock, we are sure that we can immediately try
	 * to grab the lock and cannot see interfering replicas from other threads.
	 *
	 * However, we still have a race when two or more thread groups
	 * successfully decide to acquire the lock. We leave resolution of these
	 * issues to the underlying pthread implementation. The whole system's
	 * behavior nevertheless becomes deterministic, because we only need to
	 * make sure that for this single run we see the same ordering of lock
	 * acquisitions (and this is ensured by using the pthread lock).
	 */
	class PThreadLock_priv : public Romain::PThreadLockObserver
	{
		LockFunction _functions[pt_max_wrappers];
		std::map<l4_umword_t, PThreadMutex*> _locks;
		Romain::App_model::Dataspace         _lip_ds;
		l4_addr_t                            _lip_local;
		pthread_mutex_t                      _tablemtx;

		unsigned /* Internal determinism counters */
			     det_lock_count,   // counter: det_lock
		         det_unlock_count, // counter: det_unlock
		         /* External determinism counters: */
		         mtx_lock_count,   // counter: pthread_mutex_lock
		         mtx_unlock_count, // counter: pthread_mutex_unlock
		         pt_lock_count,    // counter: __pthread_lock
		         pt_unlock_count,  // counter: __pthread_unlock
		         /* Global counter */
		         ignore_count,     // counter: call ignored
		         total_count;      // counter: total invocations

		PThreadMutex* lookup_or_fail(unsigned addr)
		{
			pthread_mutex_lock(&_tablemtx);
			PThreadMutex* r = _locks[addr];
			if (!r) {
				ERROR() << "Called with uninitialized mutex?";
				enter_kdebug("op on uninitialized mutex");
			}
			pthread_mutex_unlock(&_tablemtx);
			return r;
		}


		PThreadMutex* lookup_or_create(unsigned addr, bool init_locked = false,
		                               Romain::Thread_group* tg = 0)
		{
			pthread_mutex_lock(&_tablemtx);
			PThreadMutex* mtx = _locks[addr];
			if (!mtx) {
				mtx           = new PThreadMutex(false);
				_locks[addr]  = mtx;

				if (init_locked) {
					mtx->lock(tg);
				}
			}
			pthread_mutex_unlock(&_tablemtx);
			return mtx;
		}


		void det_lock(Romain::App_instance* inst,
		              Romain::App_thread*   t,
		              Romain::Thread_group* tg,
		              Romain::App_model*    am)
		{
			l4_addr_t stackaddr = am->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
			l4_addr_t lock      = *(l4_addr_t*)(stackaddr + 4);
			DEBUG() << "\033[35mLOCK @ \033[0m" << std::hex << lock;
			lookup_or_create(lock, true, tg)->lock(tg);
			//enter_kdebug("det_lock");
		}


		void det_unlock(Romain::App_instance* inst,
		                Romain::App_thread*   t,
		                Romain::Thread_group* tg,
		                Romain::App_model*    am)
		{
			l4_addr_t stackaddr = am->rm()->remote_to_local(t->vcpu()->r()->sp, inst->id());
			l4_addr_t lock      = *(l4_addr_t*)(stackaddr + 4);
			DEBUG() << "\033[35mUNLOCK @ \033[0m" << std::hex << lock;

			pthread_mutex_lock(&_tablemtx);
			PThreadMutex* m = _locks[lock];
			if (!m) {
				/* This may actually happen! The unlocker is simply faster sending the
				 * notification than the locker is in sending his wakeup. Hence,
				 * we need to potentially create the respective lock here.
				 */
				l4_umword_t mtx_kind_ptr = am->rm()->remote_to_local(lock + 12, inst->id());
				m            = new PThreadMutex(*(l4_umword_t*)mtx_kind_ptr == PTHREAD_MUTEX_RECURSIVE_NP);
				_locks[lock] = m;
			}
			pthread_mutex_unlock(&_tablemtx);
			m->unlock();
			//enter_kdebug("det_unlock");
		}


		void attach_lock_info_page(Romain::App_model *am);

		
		public:
		PThreadLock_priv()
			: det_lock_count(0), det_unlock_count(0),
			  mtx_lock_count(0), mtx_unlock_count(0),
			  pt_lock_count(0), pt_unlock_count(0),
			  ignore_count(0), total_count(0)
		{
			pthread_mutex_init(&_tablemtx, 0);
#if 0
			_functions[mutex_init_id].configure("threads:mutex_init",
			                                    "threads:mutex_init_rep",
			                                    mutex_init_id);
#endif
			_functions[mutex_lock_id].configure("threads:mutex_lock",
			                                    "threads:mutex_lock_rep",
			                                    mutex_lock_id);
			_functions[mutex_unlock_id].configure("threads:mutex_unlock",
			                                      "threads:mutex_unlock_rep",
			                                      mutex_unlock_id);
#if 1
			_functions[pt_lock_id].configure("threads:lock",
			                                 "threads:lock_rep",
			                                 pt_lock_id);
			_functions[pt_unlock_id].configure("threads:unlock",
			                                   "threads:unlock_rep",
			                                   pt_unlock_id);
#endif
		}

		DECLARE_OBSERVER("pthread_lock");

		void lock(Romain::App_instance* inst, Romain::App_thread* thread,
		          Romain::Thread_group* group, Romain::App_model* model);
		void unlock(Romain::App_instance* inst, Romain::App_thread* thread,
		            Romain::Thread_group* group, Romain::App_model* model);

		void mutex_init(Romain::App_instance* inst, Romain::App_thread* thread,
		                Romain::Thread_group* group, Romain::App_model* model);
		void mutex_lock(Romain::App_instance* inst, Romain::App_thread* thread,
		                Romain::Thread_group* group, Romain::App_model* model);
		void mutex_unlock(Romain::App_instance* inst, Romain::App_thread* thread,
		                  Romain::Thread_group* group, Romain::App_model* model);
	};
}
